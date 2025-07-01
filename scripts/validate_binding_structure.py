#!/usr/bin/env python3
"""
Validate the structure of language bindings.
This script checks if binding directories have the expected structure and files.
"""

import os
import sys
import argparse
from typing import Dict, List, Any

def validate_python_binding(binding_dir: str) -> Dict[str, Any]:
    """Validate Python binding structure."""
    validation = {
        "status": "unknown",
        "missing_files": [],
        "missing_directories": [],
        "recommendations": []
    }
    
    expected_files = [
        "__init__.py",
        "setup.py",
        "requirements.txt",
        "README.md"
    ]
    
    expected_dirs = [
        "tests",
        "examples"
    ]
    
    # Check for expected files
    for file_name in expected_files:
        file_path = os.path.join(binding_dir, file_name)
        if not os.path.exists(file_path):
            validation["missing_files"].append(file_name)
    
    # Check for expected directories
    for dir_name in expected_dirs:
        dir_path = os.path.join(binding_dir, dir_name)
        if not os.path.exists(dir_path):
            validation["missing_directories"].append(dir_name)
    
    # Check for Python source files
    py_files = []
    for root, dirs, files in os.walk(binding_dir):
        for file in files:
            if file.endswith('.py') and not file.startswith('__'):
                py_files.append(os.path.join(root, file))
    
    if not py_files:
        validation["missing_files"].append("*.py (source files)")
    
    # Determine status
    total_missing = len(validation["missing_files"]) + len(validation["missing_directories"])
    if total_missing == 0:
        validation["status"] = "excellent"
    elif total_missing <= 2:
        validation["status"] = "good"
    elif total_missing <= 4:
        validation["status"] = "fair"
    else:
        validation["status"] = "poor"
    
    # Add recommendations
    if validation["missing_files"]:
        validation["recommendations"].append("Create missing files")
    if validation["missing_directories"]:
        validation["recommendations"].append("Create missing directories")
    if not py_files:
        validation["recommendations"].append("Add Python source files")
    
    return validation

def validate_javascript_binding(binding_dir: str) -> Dict[str, Any]:
    """Validate JavaScript binding structure."""
    validation = {
        "status": "unknown",
        "missing_files": [],
        "missing_directories": [],
        "recommendations": []
    }
    
    expected_files = [
        "package.json",
        "README.md",
        ".gitignore"
    ]
    
    expected_dirs = [
        "src",
        "tests",
        "examples"
    ]
    
    # Check for expected files
    for file_name in expected_files:
        file_path = os.path.join(binding_dir, file_name)
        if not os.path.exists(file_path):
            validation["missing_files"].append(file_name)
    
    # Check for expected directories
    for dir_name in expected_dirs:
        dir_path = os.path.join(binding_dir, dir_name)
        if not os.path.exists(dir_path):
            validation["missing_directories"].append(dir_name)
    
    # Check for JavaScript source files
    js_files = []
    for root, dirs, files in os.walk(binding_dir):
        for file in files:
            if file.endswith(('.js', '.ts', '.jsx', '.tsx')) and not file.startswith('.'):
                js_files.append(os.path.join(root, file))
    
    if not js_files:
        validation["missing_files"].append("*.js/*.ts (source files)")
    
    # Determine status
    total_missing = len(validation["missing_files"]) + len(validation["missing_directories"])
    if total_missing == 0:
        validation["status"] = "excellent"
    elif total_missing <= 2:
        validation["status"] = "good"
    elif total_missing <= 4:
        validation["status"] = "fair"
    else:
        validation["status"] = "poor"
    
    # Add recommendations
    if validation["missing_files"]:
        validation["recommendations"].append("Create missing files")
    if validation["missing_directories"]:
        validation["recommendations"].append("Create missing directories")
    if not js_files:
        validation["recommendations"].append("Add JavaScript/TypeScript source files")
    
    return validation

def validate_rust_binding(binding_dir: str) -> Dict[str, Any]:
    """Validate Rust binding structure."""
    validation = {
        "status": "unknown",
        "missing_files": [],
        "missing_directories": [],
        "recommendations": []
    }
    
    expected_files = [
        "Cargo.toml",
        "README.md",
        ".gitignore"
    ]
    
    expected_dirs = [
        "src",
        "tests",
        "examples"
    ]
    
    # Check for expected files
    for file_name in expected_files:
        file_path = os.path.join(binding_dir, file_name)
        if not os.path.exists(file_path):
            validation["missing_files"].append(file_name)
    
    # Check for expected directories
    for dir_name in expected_dirs:
        dir_path = os.path.join(binding_dir, dir_name)
        if not os.path.exists(dir_path):
            validation["missing_directories"].append(dir_name)
    
    # Check for Rust source files
    rs_files = []
    for root, dirs, files in os.walk(binding_dir):
        for file in files:
            if file.endswith('.rs'):
                rs_files.append(os.path.join(root, file))
    
    if not rs_files:
        validation["missing_files"].append("*.rs (source files)")
    
    # Determine status
    total_missing = len(validation["missing_files"]) + len(validation["missing_directories"])
    if total_missing == 0:
        validation["status"] = "excellent"
    elif total_missing <= 2:
        validation["status"] = "good"
    elif total_missing <= 4:
        validation["status"] = "fair"
    else:
        validation["status"] = "poor"
    
    # Add recommendations
    if validation["missing_files"]:
        validation["recommendations"].append("Create missing files")
    if validation["missing_directories"]:
        validation["recommendations"].append("Create missing directories")
    if not rs_files:
        validation["recommendations"].append("Add Rust source files")
    
    return validation

def validate_go_binding(binding_dir: str) -> Dict[str, Any]:
    """Validate Go binding structure."""
    validation = {
        "status": "unknown",
        "missing_files": [],
        "missing_directories": [],
        "recommendations": []
    }
    
    expected_files = [
        "go.mod",
        "go.sum",
        "README.md"
    ]
    
    expected_dirs = [
        "cmd",
        "internal",
        "pkg",
        "test"
    ]
    
    # Check for expected files
    for file_name in expected_files:
        file_path = os.path.join(binding_dir, file_name)
        if not os.path.exists(file_path):
            validation["missing_files"].append(file_name)
    
    # Check for expected directories
    for dir_name in expected_dirs:
        dir_path = os.path.join(binding_dir, dir_name)
        if not os.path.exists(dir_path):
            validation["missing_directories"].append(dir_name)
    
    # Check for Go source files
    go_files = []
    for root, dirs, files in os.walk(binding_dir):
        for file in files:
            if file.endswith('.go'):
                go_files.append(os.path.join(root, file))
    
    if not go_files:
        validation["missing_files"].append("*.go (source files)")
    
    # Determine status
    total_missing = len(validation["missing_files"]) + len(validation["missing_directories"])
    if total_missing == 0:
        validation["status"] = "excellent"
    elif total_missing <= 2:
        validation["status"] = "good"
    elif total_missing <= 4:
        validation["status"] = "fair"
    else:
        validation["status"] = "poor"
    
    # Add recommendations
    if validation["missing_files"]:
        validation["recommendations"].append("Create missing files")
    if validation["missing_directories"]:
        validation["recommendations"].append("Create missing directories")
    if not go_files:
        validation["recommendations"].append("Add Go source files")
    
    return validation

def validate_java_binding(binding_dir: str) -> Dict[str, Any]:
    """Validate Java binding structure."""
    validation = {
        "status": "unknown",
        "missing_files": [],
        "missing_directories": [],
        "recommendations": []
    }
    
    expected_files = [
        "pom.xml",
        "README.md",
        ".gitignore"
    ]
    
    expected_dirs = [
        "src/main/java",
        "src/test/java",
        "src/main/resources"
    ]
    
    # Check for expected files
    for file_name in expected_files:
        file_path = os.path.join(binding_dir, file_name)
        if not os.path.exists(file_path):
            validation["missing_files"].append(file_name)
    
    # Check for expected directories
    for dir_name in expected_dirs:
        dir_path = os.path.join(binding_dir, dir_name)
        if not os.path.exists(dir_path):
            validation["missing_directories"].append(dir_name)
    
    # Check for Java source files
    java_files = []
    for root, dirs, files in os.walk(binding_dir):
        for file in files:
            if file.endswith('.java'):
                java_files.append(os.path.join(root, file))
    
    if not java_files:
        validation["missing_files"].append("*.java (source files)")
    
    # Determine status
    total_missing = len(validation["missing_files"]) + len(validation["missing_directories"])
    if total_missing == 0:
        validation["status"] = "excellent"
    elif total_missing <= 2:
        validation["status"] = "good"
    elif total_missing <= 4:
        validation["status"] = "fair"
    else:
        validation["status"] = "poor"
    
    # Add recommendations
    if validation["missing_files"]:
        validation["recommendations"].append("Create missing files")
    if validation["missing_directories"]:
        validation["recommendations"].append("Create missing directories")
    if not java_files:
        validation["recommendations"].append("Add Java source files")
    
    return validation

def validate_csharp_binding(binding_dir: str) -> Dict[str, Any]:
    """Validate C# binding structure."""
    validation = {
        "status": "unknown",
        "missing_files": [],
        "missing_directories": [],
        "recommendations": []
    }
    
    expected_files = [
        "*.csproj",
        "README.md",
        ".gitignore"
    ]
    
    expected_dirs = [
        "src",
        "tests",
        "examples"
    ]
    
    # Check for expected files (check for any .csproj file)
    csproj_files = []
    for root, dirs, files in os.walk(binding_dir):
        for file in files:
            if file.endswith('.csproj'):
                csproj_files.append(file)
    
    if not csproj_files:
        validation["missing_files"].append("*.csproj (project file)")
    
    # Check for README.md
    if not os.path.exists(os.path.join(binding_dir, "README.md")):
        validation["missing_files"].append("README.md")
    
    # Check for expected directories
    for dir_name in expected_dirs:
        dir_path = os.path.join(binding_dir, dir_name)
        if not os.path.exists(dir_path):
            validation["missing_directories"].append(dir_name)
    
    # Check for C# source files
    cs_files = []
    for root, dirs, files in os.walk(binding_dir):
        for file in files:
            if file.endswith('.cs'):
                cs_files.append(os.path.join(root, file))
    
    if not cs_files:
        validation["missing_files"].append("*.cs (source files)")
    
    # Determine status
    total_missing = len(validation["missing_files"]) + len(validation["missing_directories"])
    if total_missing == 0:
        validation["status"] = "excellent"
    elif total_missing <= 2:
        validation["status"] = "good"
    elif total_missing <= 4:
        validation["status"] = "fair"
    else:
        validation["status"] = "poor"
    
    # Add recommendations
    if validation["missing_files"]:
        validation["recommendations"].append("Create missing files")
    if validation["missing_directories"]:
        validation["recommendations"].append("Create missing directories")
    if not cs_files:
        validation["recommendations"].append("Add C# source files")
    
    return validation

def main():
    parser = argparse.ArgumentParser(description='Validate language binding structure')
    parser.add_argument('--binding', '-b', required=True,
                       choices=['python', 'javascript', 'rust', 'go', 'java', 'csharp'],
                       help='Language binding to validate')
    parser.add_argument('--binding-dir', '-d', default='bindings',
                       help='Directory containing language bindings')
    
    args = parser.parse_args()
    
    binding_dir = os.path.join(args.binding_dir, args.binding)
    
    if not os.path.exists(binding_dir):
        print(f"Error: Binding directory {binding_dir} does not exist")
        sys.exit(1)
    
    # Validate based on language
    if args.binding == "python":
        validation = validate_python_binding(binding_dir)
    elif args.binding == "javascript":
        validation = validate_javascript_binding(binding_dir)
    elif args.binding == "rust":
        validation = validate_rust_binding(binding_dir)
    elif args.binding == "go":
        validation = validate_go_binding(binding_dir)
    elif args.binding == "java":
        validation = validate_java_binding(binding_dir)
    elif args.binding == "csharp":
        validation = validate_csharp_binding(binding_dir)
    else:
        print(f"Error: Unsupported binding {args.binding}")
        sys.exit(1)
    
    # Print results
    print(f"Binding Structure Validation for {args.binding.upper()}")
    print(f"Status: {validation['status'].upper()}")
    print()
    
    if validation["missing_files"]:
        print("Missing files:")
        for file_name in validation["missing_files"]:
            print(f"  - {file_name}")
        print()
    
    if validation["missing_directories"]:
        print("Missing directories:")
        for dir_name in validation["missing_directories"]:
            print(f"  - {dir_name}")
        print()
    
    if validation["recommendations"]:
        print("Recommendations:")
        for recommendation in validation["recommendations"]:
            print(f"  - {recommendation}")
        print()
    
    # Exit with appropriate code
    if validation["status"] in ["poor", "fair"]:
        sys.exit(1)
    else:
        sys.exit(0)

if __name__ == '__main__':
    main() 