#!/bin/bash
# Usage: ./validate_module.sh <module_name>
# Example: ./validate_module.sh 01_usb_layer

MODULE=$1

if [ -z "$MODULE" ]; then
    echo "Usage: $0 <module_name>"
    exit 1
fi

echo "=== Validating module: $MODULE ==="

claude "You are acting as the Validator defined in agents/validator.md.
Read the following:
- workspace/plan.md (requirements)
- workspace/src/$MODULE/ (all files in this directory)
- spec.md (original spec for context)

Produce a full validation report for module: $MODULE"
