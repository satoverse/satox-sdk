#!/usr/bin/env python3
"""
Validate API consistency across different language bindings.
This script compares the API schema with language-specific bindings.
"""

import os
import sys
import json
import argparse
from typing import Dict, List, Any, Set

def load_api_schema(schema_path: str) -> Dict[str, Any]:
    """Load the API schema from JSON file."""
    try:
        with open(schema_path, 'r', encoding='utf-8') as f:
            return json.load(f)
    except Exception as e:
        print(f"Error loading API schema: {e}")
        return {}

def extract_python_api(binding_dir: str) -> Dict[str, Any]:
    """Extract API information from Python bindings."""
    api_info = {
        "classes": [],
        "functions": [],
        "constants": []
    }
    
    try:
        # Look for Python binding files
        python_files = []
        for root, dirs, files in os.walk(binding_dir):
            for file in files:
                if file.endswith('.py') and not file.startswith('__'):
                    python_files.append(os.path.join(root, file))
        
        for py_file in python_files:
            with open(py_file, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Extract class definitions
            import re
            class_pattern = r'class\s+(\w+)'
            classes = re.findall(class_pattern, content)
            for class_name in classes:
                api_info["classes"].append({"name": class_name})
            
            # Extract function definitions
            func_pattern = r'def\s+(\w+)\s*\('
            functions = re.findall(func_pattern, content)
            for func_name in functions:
                api_info["functions"].append({"name": func_name})
            
            # Extract constants
            const_pattern = r'(\w+)\s*=\s*([^#\n]+)'
            constants = re.findall(const_pattern, content)
            for const_name, const_value in constants:
                if const_name.isupper():  # Assume uppercase names are constants
                    api_info["constants"].append({
                        "name": const_name,
                        "value": const_value.strip()
                    })
                    
    except Exception as e:
        print(f"Warning: Could not parse Python bindings: {e}")
    
    return api_info

def extract_javascript_api(binding_dir: str) -> Dict[str, Any]:
    """Extract API information from JavaScript bindings."""
    api_info = {
        "classes": [],
        "functions": [],
        "constants": []
    }
    
    try:
        # Look for JavaScript binding files
        js_files = []
        for root, dirs, files in os.walk(binding_dir):
            for file in files:
                if file.endswith('.js') and not file.startswith('.'):
                    js_files.append(os.path.join(root, file))
        
        for js_file in js_files:
            with open(js_file, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Extract class definitions
            import re
            class_pattern = r'class\s+(\w+)'
            classes = re.findall(class_pattern, content)
            for class_name in classes:
                api_info["classes"].append({"name": class_name})
            
            # Extract function definitions
            func_pattern = r'(\w+)\s*\([^)]*\)\s*\{'
            functions = re.findall(func_pattern, content)
            for func_name in functions:
                if func_name not in ['if', 'for', 'while', 'switch']:
                    api_info["functions"].append({"name": func_name})
            
            # Extract constants
            const_pattern = r'const\s+(\w+)\s*=\s*([^;]+);'
            constants = re.findall(const_pattern, content)
            for const_name, const_value in constants:
                api_info["constants"].append({
                    "name": const_name,
                    "value": const_value.strip()
                })
                    
    except Exception as e:
        print(f"Warning: Could not parse JavaScript bindings: {e}")
    
    return api_info

def extract_rust_api(binding_dir: str) -> Dict[str, Any]:
    """Extract API information from Rust bindings."""
    api_info = {
        "structs": [],
        "functions": [],
        "constants": []
    }
    
    try:
        # Look for Rust binding files
        rs_files = []
        for root, dirs, files in os.walk(binding_dir):
            for file in files:
                if file.endswith('.rs'):
                    rs_files.append(os.path.join(root, file))
        
        for rs_file in rs_files:
            with open(rs_file, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Extract struct definitions
            import re
            struct_pattern = r'pub\s+struct\s+(\w+)'
            structs = re.findall(struct_pattern, content)
            for struct_name in structs:
                api_info["structs"].append({"name": struct_name})
            
            # Extract function definitions
            func_pattern = r'pub\s+fn\s+(\w+)\s*\('
            functions = re.findall(func_pattern, content)
            for func_name in functions:
                api_info["functions"].append({"name": func_name})
            
            # Extract constants
            const_pattern = r'pub\s+const\s+(\w+)\s*:\s*(\w+)\s*=\s*([^;]+);'
            constants = re.findall(const_pattern, content)
            for const_name, const_type, const_value in constants:
                api_info["constants"].append({
                    "name": const_name,
                    "type": const_type,
                    "value": const_value.strip()
                })
                    
    except Exception as e:
        print(f"Warning: Could not parse Rust bindings: {e}")
    
    return api_info

def compare_apis(cpp_api: Dict[str, Any], binding_api: Dict[str, Any], language: str) -> Dict[str, Any]:
    """Compare C++ API with language binding API."""
    comparison = {
        "language": language,
        "status": "unknown",
        "missing_in_binding": [],
        "extra_in_binding": [],
        "consistency_score": 0.0
    }
    
    # Extract names from C++ API
    cpp_classes = {cls["name"] for cls in cpp_api.get("classes", [])}
    cpp_functions = {func["name"] for func in cpp_api.get("functions", [])}
    cpp_constants = {const["name"] for const in cpp_api.get("constants", [])}
    
    # Extract names from binding API
    binding_classes = {cls["name"] for cls in binding_api.get("classes", [])}
    binding_functions = {func["name"] for func in binding_api.get("functions", [])}
    binding_constants = {const["name"] for const in binding_api.get("constants", [])}
    
    # Compare classes/structs
    if language == "rust":
        binding_structs = {struct["name"] for struct in binding_api.get("structs", [])}
        missing_classes = cpp_classes - binding_structs
        extra_classes = binding_structs - cpp_classes
    else:
        missing_classes = cpp_classes - binding_classes
        extra_classes = binding_classes - cpp_classes
    
    # Compare functions
    missing_functions = cpp_functions - binding_functions
    extra_functions = binding_functions - cpp_functions
    
    # Compare constants
    missing_constants = cpp_constants - binding_constants
    extra_constants = binding_constants - cpp_constants
    
    # Calculate consistency score
    total_cpp_items = len(cpp_classes) + len(cpp_functions) + len(cpp_constants)
    total_missing = len(missing_classes) + len(missing_functions) + len(missing_constants)
    
    if total_cpp_items > 0:
        consistency_score = (total_cpp_items - total_missing) / total_cpp_items
    else:
        consistency_score = 1.0
    
    comparison["missing_in_binding"] = {
        "classes": list(missing_classes),
        "functions": list(missing_functions),
        "constants": list(missing_constants)
    }
    
    comparison["extra_in_binding"] = {
        "classes": list(extra_classes),
        "functions": list(extra_functions),
        "constants": list(extra_constants)
    }
    
    comparison["consistency_score"] = consistency_score
    
    # Determine status
    if consistency_score >= 0.9:
        comparison["status"] = "excellent"
    elif consistency_score >= 0.7:
        comparison["status"] = "good"
    elif consistency_score >= 0.5:
        comparison["status"] = "fair"
    else:
        comparison["status"] = "poor"
    
    return comparison

def main():
    parser = argparse.ArgumentParser(description='Validate API consistency across language bindings')
    parser.add_argument('--language', '-l', required=True,
                       choices=['python', 'javascript', 'rust'],
                       help='Language binding to validate')
    parser.add_argument('--schema', '-s', default='api_schema.json',
                       help='Path to API schema JSON file')
    parser.add_argument('--binding-dir', '-b', default='bindings',
                       help='Directory containing language bindings')
    
    args = parser.parse_args()
    
    # Load API schema
    if not os.path.exists(args.schema):
        print(f"Error: API schema file {args.schema} does not exist")
        sys.exit(1)
    
    cpp_api = load_api_schema(args.schema)
    if not cpp_api:
        print("Error: Could not load API schema")
        sys.exit(1)
    
    # Extract binding API
    binding_dir = os.path.join(args.binding_dir, args.language)
    if not os.path.exists(binding_dir):
        print(f"Warning: Binding directory {binding_dir} does not exist")
        print(f"API consistency check skipped for {args.language}")
        sys.exit(0)
    
    if args.language == "python":
        binding_api = extract_python_api(binding_dir)
    elif args.language == "javascript":
        binding_api = extract_javascript_api(binding_dir)
    elif args.language == "rust":
        binding_api = extract_rust_api(binding_dir)
    else:
        print(f"Error: Unsupported language {args.language}")
        sys.exit(1)
    
    # Compare APIs
    comparison = compare_apis(cpp_api, binding_api, args.language)
    
    # Print results
    print(f"API Consistency Check for {args.language.upper()}")
    print(f"Status: {comparison['status'].upper()}")
    print(f"Consistency Score: {comparison['consistency_score']:.2%}")
    print()
    
    if comparison["missing_in_binding"]["classes"]:
        print("Missing classes:")
        for cls in comparison["missing_in_binding"]["classes"]:
            print(f"  - {cls}")
        print()
    
    if comparison["missing_in_binding"]["functions"]:
        print("Missing functions:")
        for func in comparison["missing_in_binding"]["functions"]:
            print(f"  - {func}")
        print()
    
    if comparison["missing_in_binding"]["constants"]:
        print("Missing constants:")
        for const in comparison["missing_in_binding"]["constants"]:
            print(f"  - {const}")
        print()
    
    # Exit with appropriate code
    if comparison["status"] in ["poor", "fair"]:
        sys.exit(1)
    else:
        sys.exit(0)

if __name__ == '__main__':
    main() 