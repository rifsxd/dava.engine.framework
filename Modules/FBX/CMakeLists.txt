set( MODULE_NAME FBX )

set( MODULE_TYPE STATIC )

set( HPP_FILES_RECURSE Sources/FBX/*.h )
set( CPP_FILES_RECURSE Sources/FBX/*.cpp )

set( FbxRootDir "${CMAKE_CURRENT_SOURCE_DIR}")
set( FbxThirdparty "${FbxRootDir}/Thirdparty")

set(INCLUDES "${FbxRootDir}/Sources" 
             "${FbxThirdparty}/include" 
             )

set(INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR})

set(DEFINITIONS_PRIVATE_WIN      -D_CRT_SECURE_NO_WARNINGS )
set(DEFINITIONS_PRIVATE_WINUAP   -D_CRT_SECURE_NO_WARNINGS )

set( STATIC_LIBRARIES_WIN32_RELEASE  "${FbxThirdparty}/lib/Win32/x86/release/libfbxsdk.lib" )
set( STATIC_LIBRARIES_WIN32_DEBUG    "${FbxThirdparty}/lib/Win32/x86/debug/libfbxsdk.lib" )
set( DYNAMIC_LIBRARIES_WIN32_RELEASE "${FbxThirdparty}/lib/Win32/x86/release/libfbxsdk.dll" )
set( DYNAMIC_LIBRARIES_WIN32_DEBUG   "${FbxThirdparty}/lib/Win32/x86/debug/libfbxsdk.dll" )

set( STATIC_LIBRARIES_WIN64_RELEASE  "${FbxThirdparty}/lib/Win32/x64/release/libfbxsdk.lib" )
set( STATIC_LIBRARIES_WIN64_DEBUG    "${FbxThirdparty}/lib/Win32/x64/debug/libfbxsdk.lib" )
set( DYNAMIC_LIBRARIES_WIN64_RELEASE "${FbxThirdparty}/lib/Win32/x64/release/libfbxsdk.dll" )
set( DYNAMIC_LIBRARIES_WIN64_DEBUG   "${FbxThirdparty}/lib/Win32/x64/debug/libfbxsdk.dll" )

set( DYNAMIC_LIBRARIES_MACOS         ${FbxThirdparty}/lib/MacOS/release/libfbxsdk.dylib )
set( DYNAMIC_LIBRARIES_MACOS_RELEASE ${FbxThirdparty}/lib/MacOS/release/libfbxsdk.dylib )
set( DYNAMIC_LIBRARIES_MACOS_DEBUG   ${FbxThirdparty}/lib/MacOS/debug/libfbxsdk.dylib )

set(BINARY_WIN32_DIR_RELEASE     "${FbxThirdparty}/lib/Win32/x86/release")
set(BINARY_WIN32_DIR_RELWITHDEB  "${FbxThirdparty}/lib/Win32/x86/release")
set(BINARY_WIN32_DIR_DEBUG       "${FbxThirdparty}/lib/Win32/x86/debug")
set(BINARY_WIN64_DIR_RELEASE     "${FbxThirdparty}/lib/Win32/x64/release")
set(BINARY_WIN64_DIR_RELWITHDEB  "${FbxThirdparty}/lib/Win32/x64/release")
set(BINARY_WIN64_DIR_DEBUG       "${FbxThirdparty}/lib/Win32/x64/debug")

setup_main_module()