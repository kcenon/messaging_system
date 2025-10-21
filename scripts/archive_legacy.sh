#!/bin/bash

# Archive legacy code before removal
# Usage: ./scripts/archive_legacy.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
ARCHIVE_DIR="${PROJECT_ROOT}/_archived/${TIMESTAMP}"

echo "=== Legacy Code Archive Script ==="
echo "Creating archive directory: ${ARCHIVE_DIR}"
mkdir -p "${ARCHIVE_DIR}"

# Define legacy directories to archive
LEGACY_DIRS=(
    "libraries/container_system"
    "libraries/network_system"
    "libraries/thread_system"
)

# Archive each directory
for dir in "${LEGACY_DIRS[@]}"; do
    full_path="${PROJECT_ROOT}/${dir}"
    if [ -d "$full_path" ]; then
        echo "Archiving: ${dir}"
        cp -r "$full_path" "${ARCHIVE_DIR}/"
        echo "  ✓ Archived to ${ARCHIVE_DIR}/$(basename "${dir}")"
    else
        echo "  ⚠ Directory not found: ${dir}"
    fi
done

# Create archive manifest
cat > "${ARCHIVE_DIR}/MANIFEST.md" <<EOF
# Legacy Code Archive

**Archive Date:** ${TIMESTAMP}
**Created By:** System rebuild process

## Archived Components

$(for dir in "${LEGACY_DIRS[@]}"; do
    if [ -d "${PROJECT_ROOT}/${dir}" ]; then
        echo "- ${dir}"
    fi
done)

## Reason for Archive
These components have been replaced with external system packages as part of the Phase 1 rebuild.

## Restoration (if needed)
To restore archived code:
\`\`\`bash
cp -r _archived/${TIMESTAMP}/<component> libraries/
\`\`\`

## External Replacements
- container_system → External container_system package
- network_system → External network_system package
- thread_system → External thread_system package

## Notes
- Do not restore unless absolutely necessary
- External systems provide improved performance and maintainability
- See docs/REBUILD_PLAN.md for migration details
EOF

echo ""
echo "✓ Archive complete: ${ARCHIVE_DIR}"
echo "  Manifest: ${ARCHIVE_DIR}/MANIFEST.md"
echo ""
echo "To remove archived code from build:"
echo "  Edit CMakeLists.txt and remove add_subdirectory() calls"
echo ""
