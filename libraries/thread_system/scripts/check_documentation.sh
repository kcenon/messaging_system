#!/bin/bash

# Documentation Quality Check Script for Thread System
# This script validates documentation completeness and quality

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Project root directory
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DOCS_DIR="$PROJECT_ROOT/docs"
SOURCE_DIR="$PROJECT_ROOT/sources"

echo "Thread System Documentation Quality Check"
echo "========================================"

# Initialize counters
TOTAL_CHECKS=0
PASSED_CHECKS=0
WARNINGS=0

# Function to print results
print_result() {
    local check_name=$1
    local status=$2
    local message=$3
    
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    
    if [ "$status" = "PASS" ]; then
        echo -e "${GREEN}✓${NC} $check_name"
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
    elif [ "$status" = "WARN" ]; then
        echo -e "${YELLOW}⚠${NC} $check_name: $message"
        WARNINGS=$((WARNINGS + 1))
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
    else
        echo -e "${RED}✗${NC} $check_name: $message"
    fi
}

# Check 1: Required documentation files exist
echo -e "\n1. Checking required documentation files..."

required_files=(
    "README.md"
    "docs/README.md"
    "docs/FAQ.md"
    "docs/tutorials/getting-started/quick-start.md"
    "docs/examples/basic/file_processing.md"
)

for file in "${required_files[@]}"; do
    if [ -f "$PROJECT_ROOT/$file" ]; then
        print_result "File exists: $file" "PASS" ""
    else
        print_result "File exists: $file" "FAIL" "Missing required file"
    fi
done

# Check 2: Documentation structure
echo -e "\n2. Checking documentation structure..."

required_dirs=(
    "docs/tutorials"
    "docs/examples"
    "docs/api"
    "docs/guides"
)

for dir in "${required_dirs[@]}"; do
    if [ -d "$PROJECT_ROOT/$dir" ]; then
        print_result "Directory exists: $dir" "PASS" ""
    else
        print_result "Directory exists: $dir" "FAIL" "Missing required directory"
    fi
done

# Check 3: Markdown file quality
echo -e "\n3. Checking markdown file quality..."

# Check for broken internal links
find "$DOCS_DIR" -name "*.md" -type f | while read -r file; do
    broken_links=0
    while IFS= read -r link; do
        # Extract link path
        link_path=$(echo "$link" | sed 's/.*](\(.*\)).*/\1/')
        if [[ "$link_path" == "./"* ]] || [[ "$link_path" == "../"* ]]; then
            # Resolve relative path
            dir=$(dirname "$file")
            resolved_path=$(cd "$dir" && realpath -m "$link_path" 2>/dev/null || echo "")
            if [ ! -f "$resolved_path" ] && [ ! -d "$resolved_path" ]; then
                broken_links=$((broken_links + 1))
            fi
        fi
    done < <(grep -o '\[.*\](.*\.md\(#.*\)\?)' "$file" 2>/dev/null || true)
    
    if [ $broken_links -eq 0 ]; then
        print_result "No broken links in $(basename "$file")" "PASS" ""
    else
        print_result "Broken links in $(basename "$file")" "WARN" "$broken_links broken link(s) found"
    fi
done

# Check 4: Code documentation coverage
echo -e "\n4. Checking code documentation coverage..."

if [ -d "$SOURCE_DIR" ]; then
    total_headers=$(find "$SOURCE_DIR" -name "*.h" -o -name "*.hpp" | wc -l)
    documented_headers=$(find "$SOURCE_DIR" -name "*.h" -o -name "*.hpp" -exec grep -l "@brief" {} \; | wc -l)
    
    if [ "$total_headers" -gt 0 ]; then
        coverage=$((documented_headers * 100 / total_headers))
        if [ $coverage -ge 80 ]; then
            print_result "Header documentation coverage" "PASS" "$coverage% ($documented_headers/$total_headers files)"
        elif [ $coverage -ge 60 ]; then
            print_result "Header documentation coverage" "WARN" "$coverage% ($documented_headers/$total_headers files)"
        else
            print_result "Header documentation coverage" "FAIL" "$coverage% ($documented_headers/$total_headers files)"
        fi
    fi
fi

# Check 5: Doxygen configuration
echo -e "\n5. Checking Doxygen configuration..."

if [ -f "$PROJECT_ROOT/Doxyfile" ]; then
    print_result "Doxyfile exists" "PASS" ""
    
    # Check if HTML output is enabled
    if grep -q "GENERATE_HTML.*=.*YES" "$PROJECT_ROOT/Doxyfile"; then
        print_result "HTML generation enabled" "PASS" ""
    else
        print_result "HTML generation enabled" "FAIL" "HTML output not enabled in Doxyfile"
    fi
else
    print_result "Doxyfile exists" "FAIL" "Doxyfile not found"
fi

# Check 6: Example code compilation
echo -e "\n6. Checking example code snippets..."

# Extract code blocks from markdown files and check basic syntax
example_count=0
valid_examples=0

find "$DOCS_DIR" -name "*.md" -type f | while read -r file; do
    while IFS= read -r -d '' code_block; do
        example_count=$((example_count + 1))
        # Basic syntax check (looking for common C++ patterns)
        if echo "$code_block" | grep -q -E "(#include|int main|class|struct|namespace)"; then
            valid_examples=$((valid_examples + 1))
        fi
    done < <(grep -Pzo '(?s)```cpp\n\K.*?(?=\n```)' "$file" 2>/dev/null || true)
done

if [ $example_count -gt 0 ]; then
    print_result "Code examples syntax" "PASS" "$valid_examples/$example_count examples appear valid"
fi

# Check 7: README completeness
echo -e "\n7. Checking README completeness..."

readme_checks=(
    "Installation"
    "Usage"
    "Examples"
    "License"
    "Contributing"
)

if [ -f "$PROJECT_ROOT/README.md" ]; then
    for section in "${readme_checks[@]}"; do
        if grep -qi "$section" "$PROJECT_ROOT/README.md"; then
            print_result "README contains $section section" "PASS" ""
        else
            print_result "README contains $section section" "WARN" "Section might be missing"
        fi
    done
fi

# Check 8: API documentation generation
echo -e "\n8. Checking API documentation..."

if [ -d "$PROJECT_ROOT/documents/html" ]; then
    html_files=$(find "$PROJECT_ROOT/documents/html" -name "*.html" | wc -l)
    if [ $html_files -gt 0 ]; then
        print_result "Generated API documentation" "PASS" "$html_files HTML files found"
    else
        print_result "Generated API documentation" "WARN" "No HTML files in documents/html"
    fi
else
    print_result "Generated API documentation" "WARN" "documents/html directory not found (run Doxygen to generate)"
fi

# Summary
echo -e "\n========================================"
echo "Documentation Check Summary"
echo "========================================"
echo -e "Total checks: $TOTAL_CHECKS"
echo -e "Passed: ${GREEN}$PASSED_CHECKS${NC}"
echo -e "Warnings: ${YELLOW}$WARNINGS${NC}"
echo -e "Failed: ${RED}$((TOTAL_CHECKS - PASSED_CHECKS))${NC}"

# Calculate score
SCORE=$((PASSED_CHECKS * 100 / TOTAL_CHECKS))
echo -e "\nDocumentation Quality Score: ${GREEN}${SCORE}%${NC}"

# Exit with appropriate code
if [ $SCORE -lt 70 ]; then
    echo -e "\n${RED}Documentation needs improvement!${NC}"
    exit 1
elif [ $SCORE -lt 90 ]; then
    echo -e "\n${YELLOW}Documentation is good but could be better.${NC}"
    exit 0
else
    echo -e "\n${GREEN}Excellent documentation quality!${NC}"
    exit 0
fi