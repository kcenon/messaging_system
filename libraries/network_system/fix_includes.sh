#!/bin/bash

# Script to fix include paths for the new network_system structure
# Author: kcenon
# Date: 2025-09-19

echo "Fixing include paths..."

# Fix includes in source files
find src -name "*.cpp" -exec sed -i '' 's/#include "core\//#include "network_system\/core\//g' {} \;
find src -name "*.cpp" -exec sed -i '' 's/#include "session\//#include "network_system\/session\//g' {} \;
find src -name "*.cpp" -exec sed -i '' 's/#include "internal\//#include "network_system\/internal\//g' {} \;
find src -name "*.cpp" -exec sed -i '' 's/#include "network\//#include "network_system\//g' {} \;

# Fix includes in header files (internal headers that reference other headers)
find include/network_system -name "*.h" -exec sed -i '' 's/#include "core\//#include "network_system\/core\//g' {} \;
find include/network_system -name "*.h" -exec sed -i '' 's/#include "session\//#include "network_system\/session\//g' {} \;
find include/network_system -name "*.h" -exec sed -i '' 's/#include "internal\//#include "network_system\/internal\//g' {} \;

# Fix internal source files to use relative paths for internal headers
find src/internal -name "*.cpp" -exec sed -i '' 's/#include "network_system\/internal\//#include "/g' {} \;
find src/internal -name "*.h" -exec sed -i '' 's/#include "network_system\/internal\//#include "/g' {} \;

echo "Include path fixes completed!"