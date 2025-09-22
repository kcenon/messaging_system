#!/bin/bash

# Script to update all network_module namespaces to network_system namespaces
# Author: kcenon
# Date: 2025-09-19

echo "Updating namespaces from network_module to network_system..."

# Update core files
find src/core -name "*.cpp" -exec sed -i '' 's/namespace network_module/namespace network_system::core/g' {} \;
find src/core -name "*.h" -exec sed -i '' 's/namespace network_module/namespace network_system::core/g' {} \;

# Update session files
find src/session -name "*.cpp" -exec sed -i '' 's/namespace network_module/namespace network_system::session/g' {} \;
find src/session -name "*.h" -exec sed -i '' 's/namespace network_module/namespace network_system::session/g' {} \;

# Update internal files
find src/internal -name "*.cpp" -exec sed -i '' 's/namespace network_module/namespace network_system::internal/g' {} \;
find src/internal -name "*.h" -exec sed -i '' 's/namespace network_module/namespace network_system::internal/g' {} \;

# Update namespace closing comments
find src -name "*.cpp" -exec sed -i '' 's/} \/\/ namespace network_module/} \/\/ namespace network_system/g' {} \;
find src -name "*.h" -exec sed -i '' 's/} \/\/ namespace network_module/} \/\/ namespace network_system/g' {} \;

# Update include headers in implementation files
find src -name "*.cpp" -exec sed -i '' 's/#include "core\//#include "network_system\/core\//g' {} \;
find src -name "*.cpp" -exec sed -i '' 's/#include "session\//#include "network_system\/session\//g' {} \;
find src -name "*.cpp" -exec sed -i '' 's/#include "internal\//#include "network_system\/internal\//g' {} \;

echo "Namespace update completed!"