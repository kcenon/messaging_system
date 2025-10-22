# Code Quality and Linting Guide

This document explains the linting and static analysis tools used in the messaging_system project.

## Overview

We use specialized tools for different file types:
- **Cppcheck**: C++ source code analysis
- **markdownlint**: Markdown documentation style
- **shellcheck**: Shell script validation

## Cppcheck (C++ Analysis)

### Configuration
- `.cppcheck` - Main configuration
- `.cppcheck-ignore` - Ignored patterns

### Usage
```bash
# Analyze C++ code only
cppcheck src/ include/ \
    --enable=warning,style,performance,portability \
    --std=c++20 \
    --suppress=missingIncludeSystem
```

### What Cppcheck Checks
- ✅ C++ source files (`*.cpp`, `*.h`)
- ✅ Memory leaks and undefined behavior
- ✅ Performance issues
- ✅ Style violations
- ❌ Markdown files (use markdownlint)
- ❌ Shell scripts (use shellcheck)

See `README_CPPCHECK.md` for detailed Cppcheck usage.

## markdownlint (Documentation)

### Configuration
- `.markdownlint.json` - Style rules

### Installation
```bash
# npm
npm install -g markdownlint-cli

# or using docker
docker pull 06kellyjac/markdownlint-cli
```

### Usage
```bash
# Check all markdown files
markdownlint '**/*.md'

# Fix automatically when possible
markdownlint '**/*.md' --fix

# Check specific directories
markdownlint 'docs/**/*.md' 'README*.md'
```

### Rules Overview

Our configuration balances readability with flexibility:

**Enabled (Strict)**:
- MD001: Heading levels increment by one
- MD022: Headings surrounded by blank lines
- MD025: Single H1 per document
- MD031/MD032: Fenced code blocks surrounded by blank lines
- MD047: Files end with newline

**Relaxed**:
- MD013: Line length 120 chars (flexible for code/tables)
- MD024: Duplicate headings allowed if not siblings
- MD033: HTML allowed (br, details, summary, img)
- MD040: Language specification for code blocks (optional)
- MD041: First line doesn't need to be H1

**Disabled**:
- MD012: Multiple blank lines (allowed for readability)
- MD034: Bare URLs (allowed in documentation)
- MD036: Emphasis as headings (allowed)

## shellcheck (Shell Scripts)

### Installation
```bash
# macOS
brew install shellcheck

# Ubuntu/Debian
apt-get install shellcheck

# Or use docker
docker pull koalaman/shellcheck
```

### Usage
```bash
# Check all shell scripts
shellcheck build.sh scripts/*.sh

# Check with specific shell
shellcheck --shell=bash build.sh

# Show only errors and warnings
shellcheck --severity=warning build.sh
```

### Common Fixes

**Variable Quoting** (Already Fixed):
```bash
# Bad
cmake --preset $PRESET

# Good
cmake --preset "$PRESET"
```

**Array Iteration**:
```bash
# Bad
for file in $(ls *.txt); do

# Good
for file in *.txt; do
```

## CI Integration

### GitHub Actions Example

```yaml
name: Code Quality

on: [push, pull_request]

jobs:
  cppcheck:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install Cppcheck
        run: sudo apt-get install cppcheck
      - name: Run Cppcheck
        run: |
          cppcheck src/ include/ \
            --enable=warning,style,performance \
            --std=c++20 \
            --suppress=missingIncludeSystem \
            --error-exitcode=1

  markdown:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Lint Markdown
        uses: nosborn/github-action-markdown-cli@v3.2.0
        with:
          files: .
          config_file: .markdownlint.json

  shellcheck:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Run shellcheck
        uses: ludeeus/action-shellcheck@master
        with:
          severity: warning
```

## Pre-commit Hooks (Optional)

Create `.git/hooks/pre-commit`:

```bash
#!/bin/bash

echo "Running code quality checks..."

# C++ files
if git diff --cached --name-only | grep -E '\.(cpp|h)$' > /dev/null; then
    echo "Checking C++ files with cppcheck..."
    git diff --cached --name-only | grep -E '\.(cpp|h)$' | xargs cppcheck \
        --enable=warning,style,performance \
        --std=c++20 \
        --suppress=missingIncludeSystem \
        --error-exitcode=1 || exit 1
fi

# Markdown files
if git diff --cached --name-only | grep '\.md$' > /dev/null; then
    echo "Checking Markdown files..."
    git diff --cached --name-only | grep '\.md$' | xargs markdownlint || exit 1
fi

# Shell scripts
if git diff --cached --name-only | grep '\.sh$' > /dev/null; then
    echo "Checking shell scripts..."
    git diff --cached --name-only | grep '\.sh$' | xargs shellcheck || exit 1
fi

echo "All checks passed!"
```

Make it executable:
```bash
chmod +x .git/hooks/pre-commit
```

## VS Code Integration

### Recommended Extensions

Add to `.vscode/extensions.json`:

```json
{
  "recommendations": [
    "davidanson.vscode-markdownlint",
    "ms-vscode.cpptools",
    "timonwong.shellcheck"
  ]
}
```

### Settings

Add to `.vscode/settings.json`:

```json
{
  "markdownlint.config": {
    "extends": ".markdownlint.json"
  },
  "C_Cpp.codeAnalysis.clangTidy.enabled": true,
  "shellcheck.enable": true
}
```

## Summary

| Tool | Purpose | Config File | File Types |
|------|---------|-------------|------------|
| Cppcheck | C++ analysis | `.cppcheck` | `*.cpp`, `*.h` |
| markdownlint | Doc style | `.markdownlint.json` | `*.md` |
| shellcheck | Script validation | None | `*.sh` |

**Best Practices**:
1. Use the right tool for each file type
2. Fix issues before committing
3. Don't mix tool responsibilities
4. Focus on code quality, not doc formatting

**Priority**:
1. 🔴 **Critical**: Cppcheck errors (memory safety, bugs)
2. 🟡 **Medium**: shellcheck warnings (script safety)
3. 🟢 **Low**: Markdown style (documentation formatting)
