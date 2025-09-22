#!/bin/bash

# Network System Release Script
# Creates release packages and prepares for distribution

set -e

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Default values
VERSION=""
BUILD_DIR="build-release"
SIGN_PACKAGES=false
UPLOAD_RELEASE=false
GITHUB_TOKEN=""

print_msg() {
    echo -e "${GREEN}[Release]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

show_help() {
    cat << EOF
Network System Release Script

Usage: $0 -v VERSION [OPTIONS]

Options:
    -v VERSION      Version number (e.g., 1.0.0)
    -s              Sign packages with GPG
    -u              Upload to GitHub releases (requires GITHUB_TOKEN)
    -t TOKEN        GitHub token for release upload
    -h              Show this help message

Examples:
    $0 -v 1.0.0                    # Create release packages
    $0 -v 1.0.0 -s                 # Create and sign packages
    $0 -v 1.0.0 -u -t ghp_xxx     # Create and upload to GitHub

EOF
}

# Parse arguments
while getopts "v:sut:h" opt; do
    case $opt in
        v)
            VERSION="$OPTARG"
            ;;
        s)
            SIGN_PACKAGES=true
            ;;
        u)
            UPLOAD_RELEASE=true
            ;;
        t)
            GITHUB_TOKEN="$OPTARG"
            ;;
        h)
            show_help
            exit 0
            ;;
        *)
            print_error "Invalid option"
            show_help
            exit 1
            ;;
    esac
done

# Validate version
if [ -z "$VERSION" ]; then
    print_error "Version is required"
    show_help
    exit 1
fi

# Check for GitHub token if uploading
if [ "$UPLOAD_RELEASE" = true ] && [ -z "$GITHUB_TOKEN" ]; then
    print_error "GitHub token required for upload (-t TOKEN)"
    exit 1
fi

print_msg "Starting release process for version $VERSION"

# Update version file
print_msg "Updating version file..."
echo "$VERSION" > VERSION

# Clean and create build directory
print_msg "Preparing build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Configure with release settings
print_msg "Configuring release build..."
cd "$BUILD_DIR"
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DBUILD_SAMPLES=ON \
    -DBUILD_SHARED_LIBS=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr/local

# Build
print_msg "Building project..."
cmake --build . --parallel

# Run tests
print_msg "Running tests..."
ctest --output-on-failure || {
    print_error "Tests failed! Aborting release."
    exit 1
}

# Create packages
print_msg "Creating distribution packages..."

# Source package
print_msg "Creating source package..."
make package_source

# Binary packages
print_msg "Creating binary packages..."
cpack -G TGZ
cpack -G ZIP

# Platform-specific packages
if [ "$(uname)" = "Linux" ]; then
    cpack -G DEB
    cpack -G RPM
elif [ "$(uname)" = "Darwin" ]; then
    cpack -G DragNDrop
fi

# Generate checksums
print_msg "Generating checksums..."
for file in *.tar.gz *.zip *.deb *.rpm *.dmg 2>/dev/null; do
    if [ -f "$file" ]; then
        sha256sum "$file" > "$file.sha256"
        print_msg "  - $file"
    fi
done

# Sign packages if requested
if [ "$SIGN_PACKAGES" = true ]; then
    print_msg "Signing packages..."

    # Check for GPG
    if ! command -v gpg &> /dev/null; then
        print_error "GPG not found. Cannot sign packages."
        exit 1
    fi

    for file in *.tar.gz *.zip *.deb *.rpm *.dmg 2>/dev/null; do
        if [ -f "$file" ]; then
            gpg --armor --detach-sign "$file"
            print_msg "  - Signed $file"
        fi
    done
fi

# Create release notes
print_msg "Generating release notes..."
cd ..

cat > "release-$VERSION.md" << EOF
# Network System v$VERSION

## Release Artifacts

### Source Code
- network_system-$VERSION-src.tar.gz
- network_system-$VERSION-src.zip

### Binary Packages
$(ls $BUILD_DIR/*.tar.gz $BUILD_DIR/*.zip 2>/dev/null | grep -v src | xargs -n1 basename | sed 's/^/- /')

### Platform Packages
$(ls $BUILD_DIR/*.deb $BUILD_DIR/*.rpm $BUILD_DIR/*.dmg 2>/dev/null | xargs -n1 basename | sed 's/^/- /')

## Checksums
\`\`\`
$(cat $BUILD_DIR/*.sha256 2>/dev/null)
\`\`\`

## Installation

### From Source
\`\`\`bash
tar xzf network_system-$VERSION-src.tar.gz
cd network_system-$VERSION
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
\`\`\`

### Binary Package
\`\`\`bash
tar xzf network_system-$VERSION-Linux-x86_64.tar.gz
sudo cp -r * /usr/local/
\`\`\`

### Debian/Ubuntu
\`\`\`bash
sudo dpkg -i network-system_$VERSION_amd64.deb
\`\`\`

### RedHat/CentOS
\`\`\`bash
sudo rpm -i network-system-$VERSION.x86_64.rpm
\`\`\`

## Verification

Verify package integrity:
\`\`\`bash
sha256sum -c network_system-$VERSION-Linux-x86_64.tar.gz.sha256
\`\`\`

EOF

print_msg "Release notes saved to release-$VERSION.md"

# Upload to GitHub if requested
if [ "$UPLOAD_RELEASE" = true ]; then
    print_msg "Uploading to GitHub releases..."

    # Check for gh CLI
    if ! command -v gh &> /dev/null; then
        print_error "GitHub CLI (gh) not found. Please install: https://cli.github.com"
        exit 1
    fi

    # Create GitHub release
    export GH_TOKEN="$GITHUB_TOKEN"

    gh release create "v$VERSION" \
        --title "Network System v$VERSION" \
        --notes-file "release-$VERSION.md" \
        $BUILD_DIR/*.tar.gz \
        $BUILD_DIR/*.zip \
        $BUILD_DIR/*.deb \
        $BUILD_DIR/*.rpm \
        $BUILD_DIR/*.dmg \
        $BUILD_DIR/*.sha256 \
        $BUILD_DIR/*.asc 2>/dev/null || true

    print_msg "Release uploaded to GitHub"
fi

# Summary
print_msg "Release preparation complete!"
print_msg "Version: $VERSION"
print_msg "Packages created in: $BUILD_DIR/"

ls -lh "$BUILD_DIR"/*.{tar.gz,zip,deb,rpm,dmg} 2>/dev/null || true

if [ "$UPLOAD_RELEASE" = true ]; then
    print_msg "GitHub release: https://github.com/your-org/network_system/releases/tag/v$VERSION"
else
    print_warning "To upload release, run: $0 -v $VERSION -u -t YOUR_GITHUB_TOKEN"
fi