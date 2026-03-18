#!/usr/bin/env python3
"""
font2c.py - Converte formato de texto simples para C array de fonte bitmap.

Uso:
    python3 font2c.py <arquivo_fonte.txt> <arquivo_saida.h>
    python3 font2c.py <arquivo_fonte.txt> <arquivo_saida.h> [--validate] [--stats]

Formato do arquivo fonte:
    # Comentários começam com #
    nome: MinhaFonte
    tamanho: 8x8
    padrao: ?

    [A]
    .######.
    #......#
    #......#
    ########
    #......#
    #......#
    #......#
    ........
"""

import sys
import os
import argparse
from pathlib import Path
from dataclasses import dataclass, field
from typing import Optional

# ─── Tipos de pixel válidos ────────────────────────────────────────────────────
PIXEL_ON  = {'#', '@', '1', 'X'}
PIXEL_OFF = {'.', '0', ' ', '_'}

# ─── Estruturas de dados ───────────────────────────────────────────────────────

@dataclass
class FontMetadata:
    name:    str = "MinhaFonte"
    width:   int = 8
    height:  int = 8
    default: str = "?"

    @property
    def bytes_per_row(self) -> int:
        """Quantos bytes são necessários por linha (para larguras > 8)."""
        return (self.width + 7) // 8

    @property
    def c_row_type(self) -> str:
        """Tipo C adequado para uma linha da fonte."""
        if self.width <= 8:
            return "uint8_t"
        elif self.width <= 16:
            return "uint16_t"
        elif self.width <= 32:
            return "uint32_t"
        else:
            return "uint64_t"

    @property
    def c_row_fmt(self) -> str:
        """Formato hex adequado para o tipo da linha."""
        # uint8_t → 8 bits, uint16_t → 16, etc.
        nbits = next(n for n in (8, 16, 32, 64) if self.width <= n)
        return f"0x{{:0{nbits // 4}X}}"


@dataclass
class ParseResult:
    metadata: FontMetadata
    glyphs:   dict[str, list[int]]
    warnings: list[str] = field(default_factory=list)


# ─── Parser ────────────────────────────────────────────────────────────────────

class FontParser:
    def __init__(self, verbose: bool = False):
        self.verbose = verbose

    def parse(self, path: str) -> ParseResult:
        file_path = Path(path)
        if not file_path.exists():
            raise FileNotFoundError(f"Arquivo não encontrado: {path}")
        if not file_path.is_file():
            raise ValueError(f"Não é um arquivo: {path}")

        raw = file_path.read_text(encoding="utf-8")
        return self._parse_text(raw)

    def _parse_text(self, text: str) -> ParseResult:
        metadata = FontMetadata()
        glyphs: dict[str, list[int]] = {}
        warnings: list[str] = []

        # Remove comentários e linhas vazias, mantendo numeração original
        lines_raw = text.splitlines()
        lines: list[tuple[int, str]] = []  # (linha_original, conteúdo)
        for lineno, line in enumerate(lines_raw, 1):
            stripped = line.rstrip()
            if not stripped:
                continue
            ls = stripped.lstrip()
            # Comentário: começa com "## " ou "# " (hash + espaço/hash).
            # Linhas de bitmap como "#......#" NÃO são comentários.
            is_comment = ls.startswith("## ") or ls.startswith("# ") or ls == "##" or ls == "#"
            if not is_comment:
                lines.append((lineno, stripped))

        i = 0
        while i < len(lines):
            lineno, line = lines[i]
            stripped = line.strip()

            # ── Metadados ──────────────────────────────────────────────────────
            if ":" in stripped and not stripped.startswith("["):
                key, _, value = stripped.partition(":")
                key   = key.strip().lower()
                value = value.strip()

                if key == "nome":
                    metadata.name = value
                elif key in ("tamanho", "size"):
                    try:
                        parts = value.lower().replace(" ", "").split("x")
                        metadata.width, metadata.height = int(parts[0]), int(parts[1])
                        if metadata.width < 1 or metadata.height < 1:
                            raise ValueError("Dimensões devem ser positivas")
                        if metadata.width > 64 or metadata.height > 64:
                            warnings.append(f"Linha {lineno}: fonte muito grande ({metadata.width}x{metadata.height}), pode ser lenta")
                    except (ValueError, IndexError) as e:
                        raise ValueError(f"Linha {lineno}: 'tamanho' inválido: '{value}' — esperado ex: 8x8") from e
                elif key in ("padrao", "default"):
                    metadata.default = value[0] if value else "?"

                i += 1
                continue

            # ── Glifo ──────────────────────────────────────────────────────────
            if stripped.startswith("[") and stripped.endswith("]"):
                inner = stripped[1:-1]
                char  = self._parse_char_spec(inner, lineno)

                bitmap: list[int] = []
                i += 1
                rows_read = 0

                for j in range(metadata.height):
                    if i >= len(lines):
                        break
                    row_lineno, row_line = lines[i]
                    row_stripped = row_line.strip()
                    # Para se encontrar o início de outro glifo ou metadado
                    if (row_stripped.startswith("[") and row_stripped.endswith("]")) or \
                       (":" in row_stripped and not row_stripped.startswith("[")):
                        break
                    row_val = self._parse_row(row_line, metadata.width, row_lineno, char, warnings)
                    bitmap.append(row_val)
                    i += 1
                    rows_read += 1

                if rows_read < metadata.height:
                    warnings.append(
                        f"Glifo '{char}' (linha {lineno}): só {rows_read} linhas de bitmap, "
                        f"esperado {metadata.height} — completado com zeros"
                    )

                # Preenche linhas faltantes
                while len(bitmap) < metadata.height:
                    bitmap.append(0)

                if char in glyphs:
                    warnings.append(f"Linha {lineno}: glifo '{char}' definido mais de uma vez — usando a última definição")
                glyphs[char] = bitmap
                continue

            # ── Linha não reconhecida ──────────────────────────────────────────
            warnings.append(f"Linha {lineno}: conteúdo não reconhecido ignorado: {stripped!r}")
            i += 1

        return ParseResult(metadata=metadata, glyphs=glyphs, warnings=warnings)

    def _parse_char_spec(self, inner: str, lineno: int) -> str:
        """Interpreta [A], [ ], [65], [0x41] como caractere."""
        # Não faz strip aqui — [ ] é o glifo do espaço (ASCII 32), strip destruiria isso.
        # Só faz strip se o resultado tiver mais de 1 char (pode ser "65" ou " 65 ").
        if len(inner) == 1:
            return inner  # caso comum: [A], [ ], [#], etc.

        stripped = inner.strip()
        if len(stripped) == 1:
            return stripped  # ex: [ A ] → 'A'

        # Código decimal ou hexadecimal: [65], [0x41], [ 65 ]
        try:
            code = int(stripped, 0)
            if not (0 <= code <= 255):
                raise ValueError(f"Linha {lineno}: código ASCII fora do intervalo 0–255: {code}")
            return chr(code)
        except ValueError:
            raise ValueError(f"Linha {lineno}: especificação de glifo inválida: [{inner}]")

    def _parse_row(
        self,
        row: str,
        width: int,
        lineno: int,
        char: str,
        warnings: list[str],
    ) -> int:
        value = 0
        actual = len(row)

        if actual < width:
            warnings.append(
                f"Linha {lineno} (glifo '{char}'): row tem {actual} cols, esperado {width} — completado com zeros"
            )
        elif actual > width:
            warnings.append(
                f"Linha {lineno} (glifo '{char}'): row tem {actual} cols, esperado {width} — colunas extras ignoradas"
            )

        for k in range(min(actual, width)):
            c = row[k]
            if c in PIXEL_ON:
                value |= (1 << (width - 1 - k))
            elif c not in PIXEL_OFF:
                warnings.append(
                    f"Linha {lineno} (glifo '{char}'): caractere de pixel desconhecido '{c}' na coluna {k} — tratado como OFF"
                )

        return value


# ─── Gerador C ────────────────────────────────────────────────────────────────

class CHeaderGenerator:
    def __init__(self, validate: bool = False):
        self.validate = validate

    def generate(self, result: ParseResult, output_path: str) -> None:
        meta    = result.metadata
        glyphs  = result.glyphs
        outpath = Path(output_path)

        guard_name   = f"FONT_{meta.name.upper().replace(' ', '_')}_H"
        array_name   = meta.name.replace(" ", "_")
        default_glyph = glyphs.get(meta.default, [0] * meta.height)

        # Monta array completo de 256 glifos
        all_glyphs: list[list[int]] = []
        for code in range(256):
            char = chr(code)
            if char in glyphs:
                all_glyphs.append(glyphs[char])
            else:
                all_glyphs.append(default_glyph)

        with open(outpath, "w", encoding="utf-8") as f:
            self._write_header(f, meta, guard_name, len(glyphs))
            self._write_array(f, meta, array_name, all_glyphs)
            self._write_helpers(f, meta, array_name)
            self._write_footer(f, guard_name)

    # ── Seções do arquivo ──────────────────────────────────────────────────────

    def _write_header(self, f, meta: FontMetadata, guard: str, defined_count: int) -> None:
        f.write(
            f"/*\n"
            f" * Font: {meta.name}\n"
            f" * Size: {meta.width}x{meta.height} pixels\n"
            f" * Glyphs defined: {defined_count} / 256\n"
            f" * Row type: {meta.c_row_type}\n"
            f" * Generated by font2c.py\n"
            f" */\n\n"
            f"#ifndef {guard}\n"
            f"#define {guard}\n\n"
            f"#include <stdint.h>\n\n"
            f"#define FONT_{meta.name.upper().replace(' ', '_')}_WIDTH  {meta.width}\n"
            f"#define FONT_{meta.name.upper().replace(' ', '_')}_HEIGHT {meta.height}\n\n"
        )

    def _write_array(
        self,
        f,
        meta: FontMetadata,
        array_name: str,
        all_glyphs: list[list[int]],
    ) -> None:
        row_type = meta.c_row_type
        row_fmt  = meta.c_row_fmt
        height   = meta.height

        f.write(
            f"static const {row_type} {array_name}[256][{height}] = {{\n"
        )

        for code, glyph in enumerate(all_glyphs):
            comment = self._char_comment(code)
            hex_vals = ", ".join(row_fmt.format(b) for b in glyph)
            f.write(f"    {{ {hex_vals} }},{comment}\n")

        f.write("};\n\n")

    def _write_helpers(self, f, meta: FontMetadata, array_name: str) -> None:
        name_upper = meta.name.upper().replace(" ", "_")
        f.write(
            f"/* Helper: retorna o ponteiro para o glifo do caractere c */\n"
            f"static inline const {meta.c_row_type} *\n"
            f"{array_name}_glyph(unsigned char c) {{\n"
            f"    return {array_name}[c];\n"
            f"}}\n\n"
        )

    def _write_footer(self, f, guard: str) -> None:
        f.write(f"#endif /* {guard} */\n")

    # ── Utilidades ────────────────────────────────────────────────────────────

    @staticmethod
    def _char_comment(code: int) -> str:
        if 32 <= code <= 126:
            c = chr(code)
            escaped = c.replace("\\", "\\\\").replace("/", "\\/").replace("*", "\\*")
            return f" /* '{escaped}' (0x{code:02X}) */"
        else:
            return f" /* 0x{code:02X} */"


# ─── Validador ────────────────────────────────────────────────────────────────

def validate_glyphs(result: ParseResult) -> list[str]:
    """Verifica se algum glifo está completamente vazio (suspeito)."""
    issues: list[str] = []
    for char, bitmap in result.glyphs.items():
        if all(row == 0 for row in bitmap):
            code = ord(char)
            display = repr(char) if 32 <= code <= 126 else f"ASCII {code}"
            issues.append(f"Glifo {display} está completamente vazio (todos os pixels OFF)")
    return issues


# ─── CLI ──────────────────────────────────────────────────────────────────────

def build_arg_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        description="Converte arquivo de fonte bitmap texto para header C.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    p.add_argument("input",          help="Arquivo fonte (.txt)")
    p.add_argument("output",         help="Arquivo de saída (.h)")
    p.add_argument("--validate", "-V", action="store_true", help="Valida glifos e reporta suspeitos")
    p.add_argument("--stats",    "-s", action="store_true", help="Exibe estatísticas detalhadas")
    p.add_argument("--verbose",  "-v", action="store_true", help="Exibe todos os avisos")
    return p


def print_stats(result: ParseResult, output_path: str) -> None:
    meta   = result.metadata
    glyphs = result.glyphs
    size   = os.path.getsize(output_path)

    printable = sum(1 for c in glyphs if 32 <= ord(c) <= 126)
    control   = sum(1 for c in glyphs if ord(c) < 32)
    extended  = sum(1 for c in glyphs if ord(c) > 126)

    print(f"\n{'─'*50}")
    print(f"  Fonte        : {meta.name}")
    print(f"  Tamanho      : {meta.width}x{meta.height} px")
    print(f"  Tipo de linha: {meta.c_row_type}")
    print(f"  Glifos total : {len(glyphs)} / 256")
    print(f"    Imprimíveis: {printable}  |  Controle: {control}  |  Extendido: {extended}")
    print(f"  Header gerado: {output_path} ({size:,} bytes)")
    print(f"{'─'*50}\n")


def main() -> int:
    parser = build_arg_parser()
    args   = parser.parse_args()

    try:
        # 1. Parse
        font_parser = FontParser(verbose=args.verbose)
        result = font_parser.parse(args.input)

        # 2. Avisos do parse
        if result.warnings:
            for w in result.warnings:
                print(f"[AVISO] {w}", file=sys.stderr)

        # 3. Validação opcional
        if args.validate:
            issues = validate_glyphs(result)
            for issue in issues:
                print(f"[VALIDAÇÃO] {issue}", file=sys.stderr)

        # 4. Geração do header
        generator = CHeaderGenerator(validate=args.validate)
        generator.generate(result, args.output)

        # 5. Feedback
        print(f"✓ Header gerado: {args.output}")
        if args.stats:
            print_stats(result, args.output)
        else:
            print(f"  {result.metadata.name} ({result.metadata.width}x{result.metadata.height}) "
                  f"— {len(result.glyphs)} glifos definidos")

    except (FileNotFoundError, ValueError) as e:
        print(f"[ERRO] {e}", file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        print("\nInterrompido.", file=sys.stderr)
        return 130

    return 0


if __name__ == "__main__":
    sys.exit(main())
