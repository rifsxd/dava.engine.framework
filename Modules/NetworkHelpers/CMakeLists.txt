set(MODULE_NAME NetworkHelpers)

set(MODULE_TYPE STATIC)

set(HPP_FILES_RECURSE Sources/NetworkHelpers/*.h)
set(CPP_FILES_RECURSE Sources/NetworkHelpers/Private/*.cpp)

set(INCLUDES Sources)
set(INCLUDES_PRIVATE ${CMAKE_CURRENT_LIST_DIR}
                     ${DAVA_INCLUDE_DIR})

setup_main_module()
