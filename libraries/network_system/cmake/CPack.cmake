# CPack Configuration for Network System

set(CPACK_PACKAGE_NAME "NetworkSystem")
set(CPACK_PACKAGE_VENDOR "Network System Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "High-performance asynchronous network communication library")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Source package
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "network_system-${CPACK_PACKAGE_VERSION}-src")
set(CPACK_SOURCE_IGNORE_FILES
    "/\\.git/"
    "/\\.github/"
    "/build/"
    "/\\.vscode/"
    "/\\.idea/"
    "\\.gitignore"
    "\\.DS_Store"
    ".*~"
)

# Platform-specific settings
if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Network System ${CPACK_PACKAGE_VERSION}")
    set(CPACK_NSIS_PACKAGE_NAME "Network System")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)

    # Start menu shortcuts
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documentation.lnk' '$INSTDIR\\\\share\\\\doc\\\\NetworkSystem\\\\API_REFERENCE.md'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$SMPROGRAMS\\\\$START_MENU\\\\Documentation.lnk'"
    )

elseif(APPLE)
    set(CPACK_GENERATOR "TGZ;DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "NetworkSystem-${CPACK_PACKAGE_VERSION}")
    set(CPACK_DMG_FORMAT "UDZO")  # Compressed
    set(CPACK_PACKAGE_FILE_NAME "network_system-${CPACK_PACKAGE_VERSION}-macos")

elseif(UNIX)
    set(CPACK_GENERATOR "TGZ;DEB;RPM")

    # Debian package
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Network System Team <support@network-system.io>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.27), libstdc++6 (>= 9)")
    set(CPACK_DEBIAN_PACKAGE_SUGGESTS "network-system-doc")
    set(CPACK_DEBIAN_FILE_NAME "network-system_${CPACK_PACKAGE_VERSION}_amd64.deb")

    # RPM package
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
    set(CPACK_RPM_PACKAGE_REQUIRES "glibc >= 2.27")
    set(CPACK_RPM_FILE_NAME "network-system-${CPACK_PACKAGE_VERSION}.x86_64.rpm")
    set(CPACK_RPM_PACKAGE_RELOCATABLE ON)
endif()

# Component configuration
set(CPACK_COMPONENTS_ALL libraries headers documentation samples)
set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries")
set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION "Network System runtime libraries")
set(CPACK_COMPONENT_LIBRARIES_REQUIRED ON)

set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "Development Headers")
set(CPACK_COMPONENT_HEADERS_DESCRIPTION "C++ header files for Network System development")
set(CPACK_COMPONENT_HEADERS_DEPENDS libraries)

set(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "Documentation")
set(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION "API documentation and guides")

set(CPACK_COMPONENT_SAMPLES_DISPLAY_NAME "Sample Applications")
set(CPACK_COMPONENT_SAMPLES_DESCRIPTION "Example programs demonstrating Network System usage")
set(CPACK_COMPONENT_SAMPLES_DEPENDS libraries headers)

# Archive naming
set(CPACK_PACKAGE_FILE_NAME "network_system-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

include(CPack)