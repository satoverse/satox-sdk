#!/usr/bin/env python3
"""
Generate API schema from C++ headers for validation purposes.
This script parses C++ header files and generates a JSON schema of the API.
"""

import os
import sys
import json
import argparse
import re
from pathlib import Path
from typing import Dict, List, Any

def parse_cpp_header(file_path: str) -> Dict[str, Any]:
    """Parse a C++ header file and extract API information."""
    api_info = {
        "classes": [],
        "functions": [],
        "enums": [],
        "structs": [],
        "constants": []
    }
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Extract class definitions
        class_pattern = r'class\s+(\w+)(?:\s*:\s*(?:public|private|protected)\s+(\w+))?\s*\{'
        classes = re.findall(class_pattern, content)
        for class_name, base_class in classes:
            api_info["classes"].append({
                "name": class_name,
                "base_class": base_class if base_class else None
            })
        
        # Extract function declarations
        func_pattern = r'(\w+)\s+(\w+)\s*\([^)]*\)\s*;'
        functions = re.findall(func_pattern, content)
        for return_type, func_name in functions:
            api_info["functions"].append({
                "name": func_name,
                "return_type": return_type
            })
        
        # Extract enum definitions
        enum_pattern = r'enum\s+(?:class\s+)?(\w+)\s*\{[^}]*\}'
        enums = re.findall(enum_pattern, content)
        for enum_name in enums:
            api_info["enums"].append({"name": enum_name})
        
        # Extract struct definitions
        struct_pattern = r'struct\s+(\w+)\s*\{'
        structs = re.findall(struct_pattern, content)
        for struct_name in structs:
            api_info["structs"].append({"name": struct_name})
        
        # Extract constants
        const_pattern = r'const\s+(\w+)\s+(\w+)\s*=\s*([^;]+);'
        constants = re.findall(const_pattern, content)
        for const_type, const_name, const_value in constants:
            api_info["constants"].append({
                "name": const_name,
                "type": const_type,
                "value": const_value.strip()
            })
            
    except Exception as e:
        print(f"Warning: Could not parse {file_path}: {e}")
    
    return api_info

def scan_headers(directory: str) -> Dict[str, Any]:
    """Scan directory for C++ header files and generate API schema."""
    schema = {
        "version": "1.0",
        "generated_at": "",
        "headers": {},
        "summary": {
            "total_classes": 0,
            "total_functions": 0,
            "total_enums": 0,
            "total_structs": 0,
            "total_constants": 0
        }
    }
    
    header_extensions = ['.h', '.hpp', '.hxx']
    header_files = []
    
    # Find all header files
    for root, dirs, files in os.walk(directory):
        for file in files:
            if any(file.endswith(ext) for ext in header_extensions):
                header_files.append(os.path.join(root, file))
    
    # Parse each header file
    for header_file in header_files:
        relative_path = os.path.relpath(header_file, directory)
        api_info = parse_cpp_header(header_file)
        schema["headers"][relative_path] = api_info
        
        # Update summary
        schema["summary"]["total_classes"] += len(api_info["classes"])
        schema["summary"]["total_functions"] += len(api_info["functions"])
        schema["summary"]["total_enums"] += len(api_info["enums"])
        schema["summary"]["total_structs"] += len(api_info["structs"])
        schema["summary"]["total_constants"] += len(api_info["constants"])
    
    return schema

def main():
    parser = argparse.ArgumentParser(description='Generate API schema from C++ headers')
    parser.add_argument('--output', '-o', default='api_schema.json',
                       help='Output JSON file path')
    parser.add_argument('--directory', '-d', default='include',
                       help='Directory containing header files')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.directory):
        print(f"Error: Directory {args.directory} does not exist")
        sys.exit(1)
    
    print(f"Scanning headers in {args.directory}...")
    schema = scan_headers(args.directory)
    
    # Add timestamp
    from datetime import datetime
    schema["generated_at"] = datetime.utcnow().isoformat()
    
    # Write schema to file
    with open(args.output, 'w', encoding='utf-8') as f:
        json.dump(schema, f, indent=2, ensure_ascii=False)
    
    print(f"API schema generated: {args.output}")
    print(f"Summary:")
    print(f"  - Headers processed: {len(schema['headers'])}")
    print(f"  - Classes: {schema['summary']['total_classes']}")
    print(f"  - Functions: {schema['summary']['total_functions']}")
    print(f"  - Enums: {schema['summary']['total_enums']}")
    print(f"  - Structs: {schema['summary']['total_structs']}")
    print(f"  - Constants: {schema['summary']['total_constants']}")

if __name__ == '__main__':
    main() 