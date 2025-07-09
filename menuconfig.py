#!/usr/bin/env python3
import os
import re
from dataclasses import dataclass, field
from dialog import Dialog
from typing import Optional

CONFIG_FILE = ".config"
DEF_FILE = ".def.configs"
FLAGS_FILE = ".config.flags"

d = Dialog(dialog="dialog")
d.set_background_title("Bitix Kernel Configuration")

class ParseError(Exception):
    pass

@dataclass
class Option:
    key: str
    opt_type: str
    desc: str
    cmd: str
    default: str
    options: list[str] = field(default_factory=list)       # List of option names (e.g. "80x25")
    values: dict[str, str] = field(default_factory=dict)   # Map from option name to value (e.g. {"80x25": "1"})

# Lexer tokens
TOKEN_REGEX = [
    ("SECTION",    r"(\w+)\s*:\s*section"),
    ("OPTION",     r"(\w+)\s*:\s*(bool|int|choice)"),
    ("LBRACE",     r"\{"),
    ("RBRACE",     r"\}"),
    ("KEYVAL_STR", r'(\w+)\s*=\s*"([^"]*)"'),
    ("KEYVAL",     r"(\w+)\s*=\s*([^\"\s][^\n]*)"),  # value without quotes
    ("COMMENT",    r"#.*"),
    ("NEWLINE",    r"\n"),
    ("WHITESPACE", r"[ \t]+"),
]

class Lexer:
    def __init__(self, text):
        self.text = text
        self.tokens = []
        self.pos = 0
        self.line = 1

    def tokenize(self):
        while self.pos < len(self.text):
            matched = False
            for name, pattern in TOKEN_REGEX:
                regex = re.compile(pattern)
                m = regex.match(self.text, self.pos)
                if m:
                    matched = True
                    if name == "COMMENT":
                        pass
                    elif name == "WHITESPACE" or name == "NEWLINE":
                        if name == "NEWLINE":
                            self.line += 1
                    else:
                        if name == "OPTION":
                            self.tokens.append((name, (m.group(1), m.group(2)), self.line))
                        elif name == "SECTION":
                            self.tokens.append((name, m.group(1), self.line))
                        elif name in ("KEYVAL_STR", "KEYVAL"):
                            self.tokens.append((name, (m.group(1), m.group(2)), self.line))
                        else:
                            self.tokens.append((name, None, self.line))
                    self.pos = m.end()
                    break
            if not matched:
                raise ParseError(f"Unexpected token at line {self.line} near: {self.text[self.pos:self.pos+20]!r}")
        self.tokens.append(("EOF", None, self.line))

class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def peek(self):
        return self.tokens[self.pos]

    def advance(self):
        self.pos += 1

    def expect(self, expected_type, expected_val=None):
        tok_type, tok_val, line = self.peek()
        if tok_type != expected_type or (expected_val is not None and tok_val != expected_val):
            raise ParseError(f"Expected {expected_type} {expected_val if expected_val else ''} at line {line} but got {tok_type} {tok_val}")
        self.advance()
        return tok_val

    def parse(self):
        sections = []
        options_by_section = {}
        option_details = {}

        while True:
            tok_type, tok_val, line = self.peek()
            if tok_type == "EOF":
                break

            if tok_type != "SECTION":
                raise ParseError(f"Expected section declaration at line {line}, got {tok_type}")

            section_name = tok_val
            self.advance()
            self.expect("LBRACE")

            sections.append(section_name)
            options_by_section[section_name] = []

            # parse options inside section
            while True:
                tok_type2, tok_val2, line2 = self.peek()
                if tok_type2 == "RBRACE":
                    self.advance()
                    break

                if tok_type2 != "OPTION":
                    raise ParseError(f"Expected option declaration inside section {section_name} at line {line2}")

                option_key, option_type = tok_val2
                self.advance()
                self.expect("LBRACE")

                desc = ""
                cmd = ""
                default = ""
                options = []
                values = {}

                # parse option parameters
                while True:
                    tok_type3, tok_val3, line3 = self.peek()
                    if tok_type3 == "RBRACE":
                        self.advance()
                        break
                    if tok_type3 not in ("KEYVAL_STR", "KEYVAL"):
                        raise ParseError(f"Expected key=value inside option {option_key} at line {line3}, got {tok_type3}")

                    key, val = tok_val3
                    self.advance()

                    if key == "name":
                        desc = val
                    elif key == "cmd":
                        cmd = val
                    elif key == "default":
                        default = val
                    elif key == "options":
                        # Split by comma, strip spaces
                        # Also parse option values if given (e.g. "80x25=1, 80x50=2")
                        opts = [o.strip() for o in val.split(",")]
                        for opt in opts:
                            if "=" in opt:
                                k, v = opt.split("=", 1)
                                k = k.strip()
                                v = v.strip()
                                options.append(k)
                                values[k] = v
                            else:
                                options.append(opt)
                        # If choice but no explicit values, values dict empty, fallback on option names as values later
                    else:
                        raise ParseError(f"Unknown parameter '{key}' in option '{option_key}' at line {line3}")

                options_by_section[section_name].append(option_key)
                option_details[option_key] = Option(option_key, option_type, desc, cmd, default, options, values)

        return sections, options_by_section, option_details

def read_file_strip_comments(filepath):
    with open(filepath, "r") as f:
        text = f.read()
    # Remove # comments to make lexer easier but keep newlines for line counting
    text = re.sub(r"#.*", "", text)
    return text

def load_config(option_details):
    config = {}
    for key, opt in option_details.items():
        var = f"CONFIG_{key}"
        config[var] = opt.default if opt.default else ("n" if opt.opt_type == "bool" else "")
    if os.path.exists(CONFIG_FILE):
        with open(CONFIG_FILE, "r") as f:
            for line in f:
                line = line.strip()
                if line.startswith("CONFIG_") and "=" in line:
                    k, v = line.split("=", 1)
                    config[k.strip()] = v.strip().split()[0]
    return config

def edit_section(section, keys, details, config):
    while True:
        menu_items = []

        for key in keys:
            opt = details[key]
            var = f"CONFIG_{key}"
            val = config.get(var, opt.default)
            if opt.opt_type == "bool":
                state = "[*]" if val == "y" else "[ ]"
                menu_items.append((key, f"{state} {opt.desc}"))
            else:
                display_val = val
                # For choice, if val is a mapped value, show name instead of value
                if opt.opt_type == "choice" and opt.values:
                    # Try find key by value
                    inv_map = {v:k for k,v in opt.values.items()}
                    display_val = inv_map.get(val, val)
                menu_items.append((key, f"{opt.desc} (current: {display_val})"))

        menu_items.append(("BACK", "Return to main menu"))

        code, choice = d.menu(f"{section} - Options", choices=menu_items, width=70, height=20)
        if code != d.OK or choice == "BACK":
            break

        opt = details[choice]
        var = f"CONFIG_{choice}"

        if opt.opt_type == "bool":
            config[var] = "n" if config.get(var, "n") == "y" else "y"
        elif opt.opt_type == "int":
            while True:
                code, value = d.inputbox(f"Enter integer for {choice} ({opt.desc}):", init=config.get(var, opt.default))
                if code != d.OK:
                    break
                if value.isdigit():
                    config[var] = value
                    break
                d.msgbox("Please enter a valid integer.")
        elif opt.opt_type == "choice":
            # Present options with their mapped values
            chs = []
            for opt_name in opt.options:
                val = opt.values.get(opt_name, opt_name)  # fallback val = name if no mapping
                label = f"{opt_name} ({val})" if opt.values else opt_name
                chs.append((opt_name, label))
            # Default: find option name from current value
            current_val = config.get(var, opt.default)
            if opt.values:
                inv_map = {v:k for k,v in opt.values.items()}
                default_item = inv_map.get(current_val, opt.default)
            else:
                default_item = current_val

            code, val = d.menu(f"Select value for {choice} ({opt.desc}):", choices=chs, default_item=default_item)
            if code == d.OK:
                # Save mapped value if exists
                config[var] = opt.values.get(val, val)

def save_config(sections, options_by_section, option_details, config):
    with open(CONFIG_FILE, "w") as f, open(FLAGS_FILE, "w") as ff:
        f.write("# Generated by Python menuconfig\n")
        f.write(f"# Source: {DEF_FILE}\n\n")
        cflags, asflags = [], []

        for sec in sections:
            f.write(f"# Section: {sec}\n")
            for key in options_by_section[sec]:
                var = f"CONFIG_{key}"
                val = config.get(var, "")
                opt = option_details[key]
                if val == "y":
                    f.write(f"{var}=y  # {opt.desc}\n")
                    if opt.cmd:
                        cflags.append(opt.cmd)
                        asflags.append(opt.cmd)
                elif val and val != "n":
                    f.write(f"{var}={val}  # {opt.desc}\n")
                    if opt.cmd:
                        cflags.append(f"{opt.cmd}{val}")
                        asflags.append(f"{opt.cmd}{val}")
                else:
                    f.write(f"# {var} is not set  # {opt.desc}\n")
            f.write("\n")

        ff.write(f"CFLAGS +={''.join(f' {f}' for f in cflags)}\n")
        ff.write(f"ASFLAGS +={''.join(f' {f}' for f in asflags)}\n")

def main():
    if not os.path.exists(DEF_FILE):
        d.msgbox(f"Definitions file not found: {DEF_FILE}")
        return

    try:
        text = read_file_strip_comments(DEF_FILE)
        lexer = Lexer(text)
        lexer.tokenize()
        parser = Parser(lexer.tokens)
        sections, options_by_section, option_details = parser.parse()
    except ParseError as e:
        d.msgbox(f"Parse error in {DEF_FILE}:\n{e}")
        return

    config = load_config(option_details)

    while True:
        menu_items = [(sec, f"Edit {sec}") for sec in sections] + [("Save_and_Exit", "Save and exit"), ("Exit", "Exit without saving")]
        code, choice = d.menu("Main Menu", choices=menu_items, width=60, height=20)
        if code != d.OK or choice == "Exit":
            break
        elif choice == "Save_and_Exit":
            save_config(sections, options_by_section, option_details, config)
            break
        else:
            edit_section(choice, options_by_section[choice], option_details, config)

if __name__ == "__main__":
    main()
