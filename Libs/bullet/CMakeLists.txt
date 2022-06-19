cmake_minimum_required(VERSION 3.0)
project(bullet)

function(auto_source_groups TOP_GROUP SOURCE_FILES)
    foreach(FILE ${SOURCE_FILES})
        file(RELATIVE_PATH FILE_RELATIVE ${PROJECT_SOURCE_DIR} ${FILE})
        get_filename_component(PARENT_DIR "${FILE_RELATIVE}" DIRECTORY)
        if ("${PARENT_DIR}" STREQUAL "")
            source_group("${TOP_GROUP}" FILES "${FILE_RELATIVE}")
        else()
            string(REPLACE "/" "\\" GROUP "${PARENT_DIR}")
            source_group("${TOP_GROUP}\\${GROUP}" FILES "${FILE_RELATIVE}")
        endif()
    endforeach()
endfunction()

include_directories("${PROJECT_SOURCE_DIR}")

file(GLOB_RECURSE SRC_FILES
    "BulletCollision/*.cpp" "BulletCollision/*.h"
    "BulletDynamics/*.cpp"  "BulletDynamics/*.h"
    "LinearMath/*.cpp"      "LinearMath/*.h"
    "vectormath/*.cpp"      "vectormath/*.h"
    "*.h"
)

auto_source_groups("main" "${SRC_FILES}")

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
