set( MODULE_NAME Spine )
set( MODULE_TYPE STATIC )

set( HPP_FILES_RECURSE Sources/*.h )
set( CPP_FILES_RECURSE Sources/*.cpp )

set( INCLUDES Sources )

set( INCLUDES_PRIVATE Libs/include
                      ${DAVA_ENGINE_DIR} )

set( LIBRARIES_SPINE ${CMAKE_CURRENT_LIST_DIR}/Libs )

set( STATIC_LIBRARIES_ANDROID        ${LIBRARIES_SPINE}/Android/${ANDROID_NDK_ABI_NAME}/spine.a )

set( STATIC_LIBRARIES_IOS            ${LIBRARIES_SPINE}/Ios/spine.a )

set( STATIC_LIBRARIES_MACOS          ${LIBRARIES_SPINE}/Mac/spine.a )

set( STATIC_LIBRARIES_WIN32_RELEASE  ${LIBRARIES_SPINE}/Win32/x86/Release/spine.lib )

set( STATIC_LIBRARIES_WIN32_DEBUG    ${LIBRARIES_SPINE}/Win32/x86/Debug/spine.lib )

set( STATIC_LIBRARIES_WIN64_RELEASE  ${LIBRARIES_SPINE}/Win32/x64/Release/spine.lib )

set( STATIC_LIBRARIES_WIN64_DEBUG    ${LIBRARIES_SPINE}/Win32/x64/Debug/spine.lib )

set( STATIC_LIBRARIES_WINUAP_RELEASE ${LIBRARIES_SPINE}/Win10/$(Platform)/Release/spine.lib )

set( STATIC_LIBRARIES_WINUAP_DEBUG   ${LIBRARIES_SPINE}/Win10/$(Platform)/Debug/spined.lib )

set( EXCLUDE_FROM_ALL true )

set( MODULE_INITIALIZATION true )

setup_main_module()