cmake_minimum_required(VERSION 3.16)
project(messaging_system_python VERSION 1.0.0)

# Find Python
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# Python installation message
message(STATUS "Python bindings configured:")
message(STATUS "  Python3_EXECUTABLE: ${Python3_EXECUTABLE}")
message(STATUS "  Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
message(STATUS "  Python3_VERSION: ${Python3_VERSION}")

# Install Python packages
install(DIRECTORY messaging_system
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages
        FILES_MATCHING PATTERN "*.py")

# Install setup.py and other files
install(FILES setup.py pyproject.toml README.md
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/messaging_system/python)

# Custom target to build Python package
add_custom_target(python_package
    COMMAND ${Python3_EXECUTABLE} setup.py build
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building Python package"
)

# Custom target to install Python package
add_custom_target(python_install
    COMMAND ${Python3_EXECUTABLE} setup.py install --prefix=${CMAKE_INSTALL_PREFIX}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Installing Python package"
)