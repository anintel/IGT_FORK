import re
import json
import sys
from pathlib import Path

def parse_header_file(header_path):
    registers = {}
    grouped_registers = {}
    current_register = None
    
    with open(header_path, 'r') as file:
        for line in file:
            main_match = re.match(r'#define\s+(\w+)\s+(0x[0-9A-Fa-f]+)', line)
            sub_match = re.match(r'#\s*define\s+(\w+)\s+(.+)', line)
            
            if main_match and not line.startswith('# define'):
                reg_name, reg_addr = main_match.groups()
                registers[reg_name] = {"address": reg_addr}
                current_register = reg_name
            elif sub_match and current_register:
                field_name, field_value = sub_match.groups()
                if "bitfields" not in registers[current_register]:
                    registers[current_register]["bitfields"] = []
                
                bitfield_entry = {"name": field_name, "value": field_value.strip()}
                
                registers[current_register]["bitfields"].append(bitfield_entry)
    
    # Grouping based on first two words separated by '_'
    for reg_name, reg_data in registers.items():
        group_key = "_".join(reg_name.split("_")[:2]) if "_" in reg_name else reg_name
        if group_key not in grouped_registers:
            grouped_registers[group_key] = {}
        grouped_registers[group_key][reg_name] = reg_data
    
    # Flatten groups with only one child
    final_data = {}
    for group, items in grouped_registers.items():
        if len(items) == 1:
            final_data.update(items)
        else:
            final_data[group] = items
    
    return final_data

def save_to_json(data, output_path):
    with output_path.open("w", encoding="utf-8") as json_file:
        json.dump(data, json_file, indent=4, ensure_ascii=False)

def main():
    header_path = Path(__file__).resolve().parent.parent / 'data' / 'drm_dp.h'
    json_path = Path(__file__).resolve().parent.parent / 'data' / 'dpcd.json'  # Set your JSON file name here
    
    extracted_data = parse_header_file(header_path)
    save_to_json(extracted_data, json_path)
    print(f"Extracted registers saved to {json_path}")

if __name__ == "__main__":
    main()
