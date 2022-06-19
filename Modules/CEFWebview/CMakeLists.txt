set( MODULE_NAME CEFWebview )
set( MODULE_TYPE STATIC )

set( PLATFORMS_SUPPORTED WIN )                

set( HPP_FILES Sources/CEFWebview/*.h )
set( CPP_FILES Sources/CEFWebview/Private/*.cpp     )

set ( UNITY_IGNORE_LIST "CEFController.cpp"
                        "CEFWebViewControl.cpp"
                        "CEFWebPageRender.cpp"
                        "CEFDavaResourceHandler.cpp" )

set( DEFINITIONS      -DENABLE_CEF_WEBVIEW -DDISABLE_NATIVE_WEBVIEW )

set( INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/Sources"
              "${CMAKE_CURRENT_SOURCE_DIR}/Include"
              "${CMAKE_CURRENT_SOURCE_DIR}/Include/cef" )
                      
set( INCLUDES_PRIVATE "${DAVA_INCLUDE_DIR}"
                      "${CMAKE_CURRENT_SOURCE_DIR}/Sources"
                      "${CMAKE_CURRENT_SOURCE_DIR}/Include"
                      "${CMAKE_CURRENT_SOURCE_DIR}/Include/cef" )

set ( CEF_LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32" )

set( STATIC_LIBRARIES_WIN32_RELEASE  "${CEF_LIBS_DIR}/x86/Release/libcef.dll.lib"
                                     "${CEF_LIBS_DIR}/x86/Release/libcef_dll_wrapper.lib" )

set( STATIC_LIBRARIES_WIN32_DEBUG    "${CEF_LIBS_DIR}/x86/Debug/libcef.dll.lib" 
                                     "${CEF_LIBS_DIR}/x86/Debug/libcef_dll_wrapper.lib" )
                                     
set( STATIC_LIBRARIES_WIN64_RELEASE  "${CEF_LIBS_DIR}/x64/Release/libcef.dll.lib"
                                     "${CEF_LIBS_DIR}/x64/Release/libcef_dll_wrapper.lib" )

set( STATIC_LIBRARIES_WIN64_DEBUG    "${CEF_LIBS_DIR}/x64/Debug/libcef.dll.lib"
                                     "${CEF_LIBS_DIR}/x64/Debug/libcef_dll_wrapper.lib" )
                                     
if ( WIN32 )
    set ( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /delayload:libcef.dll" )
endif ()
                                     
set( BINARY_WIN32_DIR_RELEASE    "${CMAKE_CURRENT_SOURCE_DIR}/Data/x86" )
set( BINARY_WIN32_DIR_DEBUG      "${CMAKE_CURRENT_SOURCE_DIR}/Data/x86" )
set( BINARY_WIN32_DIR_RELWITHDEB "${CMAKE_CURRENT_SOURCE_DIR}/Data/x86" )
set( BINARY_WIN64_DIR_RELEASE    "${CMAKE_CURRENT_SOURCE_DIR}/Data/x64" )
set( BINARY_WIN64_DIR_DEBUG      "${CMAKE_CURRENT_SOURCE_DIR}/Data/x64" )
set( BINARY_WIN64_DIR_RELWITHDEB "${CMAKE_CURRENT_SOURCE_DIR}/Data/x64" )

if ( X64_MODE )
    set ( ARCH_PREFIX "x64" )
else()
    set ( ARCH_PREFIX "x86" )    
endif ()

set( DEPLOY_TO_BIN_WIN "${CMAKE_CURRENT_SOURCE_DIR}/Data/${ARCH_PREFIX}/*" )

set( USE_PARENT_DEFINITIONS true )

setup_main_module()
