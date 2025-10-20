#pragma once

// Legacy System Guard
// This header ensures external systems are properly configured before building

#ifndef MESSAGING_USE_EXTERNAL_SYSTEMS
  #error "Legacy internal systems are deprecated. Set MESSAGING_USE_EXTERNAL_SYSTEMS=ON in CMake"
#endif

// Verify each required system is available

#if !defined(HAS_COMMON_SYSTEM)
  #error "CommonSystem not found. Install common_system package or enable FetchContent"
  #error "  Ubuntu/Debian: apt-get install libcommon-system-dev"
  #error "  macOS: brew install common-system"
  #error "  OR: cmake -DMESSAGING_USE_FETCHCONTENT=ON"
#endif

#if !defined(HAS_THREAD_SYSTEM)
  #error "ThreadSystem not found. Install thread_system package or enable FetchContent"
  #error "  Ubuntu/Debian: apt-get install libthread-system-dev"
  #error "  macOS: brew install thread-system"
  #error "  OR: cmake -DMESSAGING_USE_FETCHCONTENT=ON"
#endif

#if !defined(HAS_LOGGER_SYSTEM)
  #error "LoggerSystem not found. Install logger_system package or enable FetchContent"
  #error "  Ubuntu/Debian: apt-get install liblogger-system-dev"
  #error "  macOS: brew install logger-system"
  #error "  OR: cmake -DMESSAGING_USE_FETCHCONTENT=ON"
#endif

#if !defined(HAS_MONITORING_SYSTEM)
  #error "MonitoringSystem not found. Install monitoring_system package or enable FetchContent"
  #error "  Ubuntu/Debian: apt-get install libmonitoring-system-dev"
  #error "  macOS: brew install monitoring-system"
  #error "  OR: cmake -DMESSAGING_USE_FETCHCONTENT=ON"
#endif

#if !defined(HAS_CONTAINER_SYSTEM)
  #error "ContainerSystem not found. Install container_system package or enable FetchContent"
  #error "  Ubuntu/Debian: apt-get install libcontainer-system-dev"
  #error "  macOS: brew install container-system"
  #error "  OR: cmake -DMESSAGING_USE_FETCHCONTENT=ON"
#endif

#if !defined(HAS_DATABASE_SYSTEM)
  #error "DatabaseSystem not found. Install database_system package or enable FetchContent"
  #error "  Ubuntu/Debian: apt-get install libdatabase-system-dev"
  #error "  macOS: brew install database-system"
  #error "  OR: cmake -DMESSAGING_USE_FETCHCONTENT=ON"
#endif

#if !defined(HAS_NETWORK_SYSTEM)
  #error "NetworkSystem not found. Install network_system package or enable FetchContent"
  #error "  Ubuntu/Debian: apt-get install libnetwork-system-dev"
  #error "  macOS: brew install network-system"
  #error "  OR: cmake -DMESSAGING_USE_FETCHCONTENT=ON"
#endif

// All systems verified - proceed with build
// This header can be included in main CMakeLists.txt or critical source files
