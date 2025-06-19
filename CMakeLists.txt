cmake_minimum_required(VERSION 3.10)
project(HRPPDAnalysis)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set install directory
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Installation directory" FORCE)
endif()

# Show ROOT information
message(STATUS "Looking for ROOT...")
message(STATUS "ROOT_DIR = $ENV{ROOTSYS}")

# Find ROOT package
find_package(ROOT REQUIRED COMPONENTS RIO Net Hist Graf Graf3d Gpad Tree Rint Postscript Matrix Physics MathCore Thread MultiProc)
message(STATUS "ROOT include dir: ${ROOT_INCLUDE_DIRS}")
message(STATUS "ROOT libraries: ${ROOT_LIBRARIES}")

# Include ROOT directories
include(${ROOT_USE_FILE})
include_directories(SYSTEM ${ROOT_INCLUDE_DIRS})

# Root dict generation
execute_process(COMMAND root-config --cflags OUTPUT_VARIABLE ROOT_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "ROOT CXX flags: ${ROOT_CXX_FLAGS}")

# Add compiler flags for ROOT
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ROOT_CXX_FLAGS}")

# Include our directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files for library
set(SOURCES
    src/WaveformProcessor.cc
    src/EventAnalyzer.cc
    src/Config.cc
    src/DataIO.cc
    src/Ntupler.cc
)

# Create library
add_library(HRPPDLib SHARED ${SOURCES})
target_include_directories(HRPPDLib PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    PRIVATE ${ROOT_INCLUDE_DIRS}
)
target_link_libraries(HRPPDLib ${ROOT_LIBRARIES})

# Analysis executables
if(EXISTS "${CMAKE_SOURCE_DIR}/analysis/analyzer.cc")
    add_executable(analyzer analysis/analyzer.cc)
    target_include_directories(analyzer PRIVATE ${CMAKE_SOURCE_DIR}/include ${ROOT_INCLUDE_DIRS})
    target_link_libraries(analyzer HRPPDLib ${ROOT_LIBRARIES})

    # Set rpath for the analyzer executable
    set_target_properties(analyzer PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif()

# Create output directories
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/output)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/config)

# Make all scripts executable
file(GLOB SCRIPTS ${CMAKE_SOURCE_DIR}/scripts/*.sh)
foreach(SCRIPT ${SCRIPTS})
    if(EXISTS "${SCRIPT}")
        file(CHMOD ${SCRIPT} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                        GROUP_READ GROUP_EXECUTE
                                        WORLD_READ WORLD_EXECUTE)
    endif()
endforeach()

# Installation paths
install(TARGETS HRPPDLib analyzer
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install header files
install(DIRECTORY include/
    DESTINATION include/HRPPD
    FILES_MATCHING PATTERN "*.h"
)

# Install configuration files
install(DIRECTORY config/
    DESTINATION config
    FILES_MATCHING PATTERN "*.txt" PATTERN "*.conf"
)

# Install scripts
install(DIRECTORY scripts/
    DESTINATION scripts
    FILES_MATCHING PATTERN "*.sh"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)

# Make sure everything is built before installing
install(CODE "
    message(STATUS \"Building all targets before installation...\")
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build .
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        RESULT_VARIABLE BUILD_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    if(NOT BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR \"Build failed before installation\")
    endif()
") 