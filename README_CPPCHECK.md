# Cppcheck Configuration

This document explains how to use Cppcheck with the messaging_system project.

## Configuration Files

- `.cppcheck` - Main configuration file with include paths and suppressions
- `.cppcheck-ignore` - File patterns to ignore (documentation, build directories)

## Running Cppcheck

### Basic Usage

```bash
# Run cppcheck with project configuration
cppcheck --project=build-local/compile_commands.json --enable=all
```

### Recommended Usage

```bash
# Run with configuration file (automatically loaded)
cppcheck src/ include/ \
    --enable=warning,style,performance,portability \
    --std=c++20 \
    --suppress=missingIncludeSystem
```

### Generate Compile Commands

```bash
# Configure with compile commands export
cmake --preset dev-local -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

## Understanding Warnings

### Include File Not Found (SUPPRESSED)

These warnings are suppressed because:
- **Standard library headers**: Cppcheck doesn't need them for analysis
- **External system headers**: Configured in include paths

### Shell Script Warnings (FIXED)

Variable quoting issues in `build.sh` and `scripts/archive_legacy.sh` have been fixed:
- Variables in commands are now properly quoted
- Prevents word splitting and globbing issues

### Markdown Style Warnings (IGNORED)

Documentation style warnings are low priority and can be ignored:
- Heading levels
- List formatting
- Line length
- Fenced code block languages

## Suppression Guide

### Suppress Specific Warnings

Add to `.cppcheck`:
```
--suppress=warningId:path/to/file.cpp
```

### Suppress by Pattern

```
--suppress=*:*/external_dependency/*
```

### Suppress in Code

```cpp
// cppcheck-suppress warningId
code_that_triggers_warning();
```

## CI Integration

Example GitHub Actions workflow:

```yaml
- name: Run Cppcheck
  run: |
    cppcheck src/ include/ \
      --enable=warning,style,performance \
      --std=c++20 \
      --suppress=missingIncludeSystem \
      --error-exitcode=1
```

## Known Issues

1. **External system headers**: Paths are hardcoded in `.cppcheck`
   - Update paths if external systems move
   - Use relative paths when possible

2. **Standard library suppression**: `missingIncludeSystem` suppresses all standard headers
   - This is recommended by Cppcheck documentation
   - Does not affect analysis quality

3. **Documentation warnings**: Not relevant for code quality
   - Use markdown linters instead (markdownlint, mdl)
   - Focus on code-related warnings only

## References

- [Cppcheck Manual](http://cppcheck.sourceforge.net/manual.pdf)
- [Suppressions Documentation](https://cppcheck.sourceforge.io/manual.html#idm165)
