cmake_minimum_required(VERSION 3.0)
project(PVRTexLib)

if (NOT WIN32)
    message(FATAL_ERROR "Only win32 platform is supported")
endif()

# Detect target platform bitness
string(FIND ${CMAKE_GENERATOR} "Win64" X64_PROJECT)
if (${X64_PROJECT} EQUAL -1)
    set(X64_MODE FALSE)
else()
    set(X64_MODE TRUE)
endif()

set(SOURCE_FILES
    "dllmain.cpp"
    "PVRTexLib.def"
)

add_library("${PROJECT_NAME}" SHARED
    "${SOURCE_FILES}"
)

if (X64_MODE)
    target_link_libraries("${PROJECT_NAME}"
        debug     "${PROJECT_SOURCE_DIR}/Libs/PVRTexLib64-debug.lib"
        optimized "${PROJECT_SOURCE_DIR}/Libs/PVRTexLib64.lib"
    )
else()
    target_link_libraries("${PROJECT_NAME}"
        debug     "${PROJECT_SOURCE_DIR}/Libs/PVRTexLib32-debug.lib"
        optimized "${PROJECT_SOURCE_DIR}/Libs/PVRTexLib32.lib"
    )
endif()
