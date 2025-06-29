#!/usr/bin/env python3
import os
import re
from dialog import Dialog

CONFIG_FILE = ".config"
DEF_FILE = ".def.configs"
FLAGS_FILE = ".config.flags"

d = Dialog(dialog="dialog")
d.set_background_title("Bitix Kernel Configuration")

# Custom exception for parsing errors
class ParseError(Exception):
    pass

def read_file_strip_comments(filepath):
    """Read file lines stripping comments and empty lines."""
    with open(filepath, "r") as f:
        lines = f.readlines()
    processed = []
    for line in lines:
        # Remove inline comments starting with #
        line = re.sub(r"#.*$", "", line).strip()
        if line:
            processed.append(line)
    return processed

def parse_definitions(filepath):
    """
    Parse .def.configs file with C-like syntax:
    
    section_name : section {
        OPTION : bool|int|choice {
            name = "Description"
            cmd = "-O"
            default = y|n|number|string
            options = "opt1,opt2,opt3"  # only for choice
        }
    }
    
    Returns:
        sections: list of section names
        options_by_section: dict section -> list of keys
        option_details: dict key -> dict with keys: type, desc, cmd, default, options
    """
    lines = read_file_strip_comments(filepath)
    sections = []
    options_by_section = {}
    option_details = {}

    i = 0
    n = len(lines)

    def expect(expected, context):
        nonlocal i, lines
        if i >= n or lines[i] != expected:
            raise ParseError(f"Expected '{expected}' in {context} at line {i+1}")
        i += 1

    def parse_block(context):
        nonlocal i, lines
        if i >= n or lines[i] != "{":
            raise ParseError(f"Expected '{{' to open block in {context} at line {i+1}")
        i += 1

    def parse_until_closing_brace(context):
        nonlocal i, lines
        block_lines = []
        depth = 1
        while i < n and depth > 0:
            line = lines[i]
            if line == "{":
                depth += 1
            elif line == "}":
                depth -= 1
                if depth == 0:
                    i += 1
                    break
            block_lines.append(line)
            i += 1
        if depth != 0:
            raise ParseError(f"Missing closing '}}' in {context}")
        return block_lines[:-1] if block_lines and block_lines[-1] == "}" else block_lines

    while i < n:
        # Parse section header:
        # Format: section_name : section {
        m = re.match(r"^(\w+)\s*:\s*section\s*$", lines[i])
        if m:
            section = m.group(1)
            i += 1
            parse_block(f"section {section}")
            sections.append(section)
            options_by_section[section] = []

            # Now parse options inside section block
            while i < n and lines[i] != "}":
                # Parse option header: OPTION_NAME : bool|int|choice
                m2 = re.match(r"^(\w+)\s*:\s*(bool|int|choice)\s*$", lines[i])
                if not m2:
                    raise ParseError(f"Expected option declaration inside section '{section}', got '{lines[i]}' at line {i+1}")
                key, opt_type = m2.group(1), m2.group(2)
                i += 1

                parse_block(f"option {key}")

                # Parse option details inside option block
                desc = ""
                cmd = ""
                default = ""
                options = []

                while i < n and lines[i] != "}":
                    line = lines[i]

                    # Parse key = "value" or key = value
                    m3 = re.match(r'^(\w+)\s*=\s*"(.*)"$', line)
                    if not m3:
                        m3 = re.match(r'^(\w+)\s*=\s*(.*)$', line)
                    if not m3:
                        raise ParseError(f"Invalid option param line '{line}' in option '{key}' at line {i+1}")

                    param_key, param_val = m3.group(1), m3.group(2).strip()

                    if param_key == "name":
                        desc = param_val
                    elif param_key == "cmd":
                        cmd = param_val
                    elif param_key == "default":
                        default = param_val
                    elif param_key == "options":
                        options = [opt.strip() for opt in param_val.split(",")]
                    else:
                        raise ParseError(f"Unknown parameter '{param_key}' in option '{key}' at line {i+1}")
                    i += 1

                expect("}", f"option {key}")

                options_by_section[section].append(key)
                option_details[key] = {
                    "type": opt_type,
                    "desc": desc,
                    "cmd": cmd,
                    "default": default,
                    "options": options
                }

            expect("}", f"section {section}")

        else:
            raise ParseError(f"Expected section declaration at line {i+1}, got '{lines[i]}'")

    return sections, options_by_section, option_details

def load_config(option_details):
    """
    Load config from CONFIG_FILE or defaults.

    Returns a dict: CONFIG_KEY -> value (y/n/number/string)
    """
    config = {}

    # Initialize with defaults
    for key, detail in option_details.items():
        var = f"CONFIG_{key}"
        if detail["default"]:
            config[var] = detail["default"]
        else:
            config[var] = "n" if detail["type"] == "bool" else ""

    # Override with values from CONFIG_FILE if exists
    if os.path.exists(CONFIG_FILE):
        with open(CONFIG_FILE, "r") as f:
            for line in f:
                line = line.strip()
                if line.startswith("CONFIG_") and "=" in line:
                    k, v = line.split("=", 1)
                    k = k.strip()
                    v = v.strip().split()[0]  # remove comments
                    config[k] = v

    return config

def edit_section(section, opts, details, config):
    """
    Use dialog to edit options in a section.

    For bool: checklist
    For int: inputbox with validation
    For choice: menu selection
    """
    # First, edit bool options via checklist (can batch)
    checklist = []
    int_opts = []
    choice_opts = []

    for key in opts:
        var = f"CONFIG_{key}"
        val = config.get(var, None)
        detail = details[key]

        if detail["type"] == "bool":
            checklist.append((key, detail["desc"], "on" if val == "y" else "off"))
        elif detail["type"] == "int":
            int_opts.append(key)
        elif detail["type"] == "choice":
            choice_opts.append(key)

    # Bool options checklist
    if checklist:
        code, selected = d.checklist(f"{section} - Boolean Options", choices=checklist, width=70, height=20)
        if code != d.OK:
            return
        for key in opts:
            var = f"CONFIG_{key}"
            detail = details[key]
            if detail["type"] == "bool":
                config[var] = "y" if key in selected else "n"

    # Int options inputbox (one by one)
    for key in int_opts:
        var = f"CONFIG_{key}"
        detail = details[key]
        current = config.get(var, detail["default"])
        while True:
            code, value = d.inputbox(f"Enter integer for {key} ({detail['desc']}):", init=str(current))
            if code != d.OK:
                break
            if value.isdigit():
                config[var] = value
                break
            d.msgbox("Please enter a valid integer.")

    # Choice options menu
    for key in choice_opts:
        var = f"CONFIG_{key}"
        detail = details[key]
        current = config.get(var, detail["default"])
        choices = [(opt, "") for opt in detail["options"]]
        try:
            default_index = detail["options"].index(current) if current in detail["options"] else None
        except Exception:
            default_index = None

        code, choice = d.menu(f"Select value for {key} ({detail['desc']}):", choices=choices, 
                              default_item=current if current else None)
        if code == d.OK:
            config[var] = choice

def save_config(sections, options_by_section, option_details, config):
    """
    Save .config and .config.flags files.
    """
    with open(CONFIG_FILE, "w") as f, open(FLAGS_FILE, "w") as ff:
        f.write("# Generated by Python menuconfig\n")
        f.write(f"# Source: {DEF_FILE}\n\n")
        cflags, asflags = [], []

        for section in sections:
            f.write(f"# Section: {section}\n")
            for key in options_by_section[section]:
                var = f"CONFIG_{key}"
                val = config.get(var, None)
                desc = option_details[key]["desc"]
                cmd = option_details[key]["cmd"]

                if val == "y":
                    f.write(f"{var}=y  # {desc}\n")
                    if cmd:
                        cflags.append(cmd)
                        asflags.append(cmd)
                elif val and val != "n":
                    f.write(f"{var}={val}  # {desc}\n")
                    if cmd:
                        cflags.append(f"{cmd}{val}")
                        asflags.append(f"{cmd}{val}")
                else:
                    f.write(f"# {var} is not set  # {desc}\n")
            f.write("\n")

        ff.write(f"CFLAGS +={''.join(f' {flag}' for flag in cflags)}\n")
        ff.write(f"ASFLAGS +={''.join(f' {flag}' for flag in asflags)}\n")

def main():
    if not os.path.exists(DEF_FILE):
        d.msgbox(f"Definitions file not found: {DEF_FILE}")
        return

    try:
        sections, options_by_section, option_details = parse_definitions(DEF_FILE)
    except ParseError as e:
        d.msgbox(f"Parse error in {DEF_FILE}:\n{e}")
        return

    config = load_config(option_details)

    while True:
        menu_items = [(sec, f"Edit {sec}") for sec in sections]
        menu_items.append(("Save_and_Exit", "Save and exit"))
        menu_items.append(("Exit", "Exit without saving"))

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
