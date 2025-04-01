from lxml import etree
import os
import json
from pathlib import Path
import re

def parse_xml_to_dict(xml_file):
    try:
        tree = etree.parse(xml_file)
        root = tree.getroot()
        
        root = root[0] if root.tag == "BSpec" else root
        
        def element_to_dict(element):

            if element.tag == "Address":
                val = {k: str(v) for k, v in element.attrib.items()}
                return val 
            
            node = {"@attributes": {k: str(v) for k, v in element.attrib.items()}} if element.attrib else {}
            text = element.text.strip() if element.text and element.text.strip() else None
            
            if text:
                node["#text"] = text
            children = {}
            
            for child in element:
                if child.tag == "ProgrammingNote":
                    continue
                child_dict = element_to_dict(child)
                
                if child.tag not in children:
                    children[child.tag] = child_dict
                else:
                    if not isinstance(children[child.tag], list):
                        children[child.tag] = [children[child.tag]]
                    children[child.tag].append(child_dict)
            if children:
                node.update(children)
            return node if node else None

        parsed_dict = {root.tag: element_to_dict(root)}
        
        # Find address using the findAddress function
        symbol = parsed_dict.get("Register", {}).get("@attributes", {}).get("Name")
        if symbol:
            address_entries = findAddress(symbol)
            if address_entries:
                if "Address" not in parsed_dict["Register"]:
                    parsed_dict["Register"]["Address"] = address_entries
                else:
                    if isinstance(parsed_dict["Register"]["Address"], list):
                        parsed_dict["Register"]["Address"].extend(address_entries)
                    else:
                        parsed_dict["Register"]["Address"] = [parsed_dict["Register"]["Address"]] + address_entries
        
        return parsed_dict
    
    except Exception as e:
        print(f"Error parsing XML file {xml_file}: {e}")
        return {}

def traverse_and_build_hierarchy(input_dir):
    def build_hierarchy(current_path):
        print(f"Processing {current_path}...")
        hierarchy = {"@type": "folder", "content": {}}
        for entry in sorted(os.listdir(current_path)):
            entry_path = os.path.join(current_path, entry)
            if os.path.isdir(entry_path):
                sub_hierarchy = build_hierarchy(entry_path)
                if sub_hierarchy and sub_hierarchy["content"]:  # Only add non-empty folders
                    hierarchy["content"][entry] = sub_hierarchy
            elif entry.startswith('Register_') and entry.endswith('.xml'):
                file_name = entry[len('Register_'):-len('.bspec.xml')]
                hierarchy["content"][file_name] = {
                    "@type": "file", 
                    "content": parse_xml_to_dict(entry_path)
                }
        return hierarchy if hierarchy["content"] else None

    return build_hierarchy(input_dir)

def findAddress(symbol):
    try:
        path = Path(__file__).resolve().parent.parent / 'data' / 'Registers.json'
        with open(path, 'r') as f:
            registers = json.load(f)
        
        matching_entries = []
        pattern = re.compile(rf"{symbol}_\d+_\w+")

        def search_entries(data):
            if isinstance(data, dict):
                for key, value in data.items():
                    if key == "Symbol" and pattern.match(value):
                        matching_entries.append(data)
                    else:
                        search_entries(value)
            elif isinstance(data, list):
                for item in data:
                    search_entries(item)
        
        search_entries(registers)
        return matching_entries
    
    except Exception as e:
        print(f"Error reading Registers.json file: {e}")
        return []

def main():
    input_dir = Path(__file__).resolve().parent.parent / 'data' / 'BXML' 
    output_file = Path(__file__).resolve().parent.parent / 'data' / 'Singleson.json'

    if not os.path.isdir(input_dir):
        print("Invalid directory. Please provide a valid path.")
        return

    print("Processing, please wait...")

    all_data = traverse_and_build_hierarchy(input_dir)
    
    def convert_keys_to_str(data):
        if isinstance(data, dict):
            return {str(key): convert_keys_to_str(value) for key, value in data.items()}
        elif isinstance(data, list):
            return [convert_keys_to_str(item) for item in data]
        else:
            return data

    all_data = convert_keys_to_str(all_data)
    
    try:
        with open(output_file, "w") as f:
            json.dump(all_data, f, indent=4)
        print(f"JSON output written to {output_file}")
    except Exception as e:
        print(f"Error writing JSON file: {e}")

if __name__ == "__main__":
    main()
