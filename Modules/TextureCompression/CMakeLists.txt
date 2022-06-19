set( MODULE_NAME TextureCompression )

set( MODULE_TYPE STATIC )

set( PLATFORMS_SUPPORTED WIN MACOS)                

set( HPP_FILES_RECURSE Sources/*.h )
set( CPP_FILES_RECURSE Sources/*.cpp )

set( INCLUDES  	Sources)
set( INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR} Libs/Include)

# win32 libs x86
set( STATIC_LIBRARIES_WIN32_RELEASE      ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x86/Release/libdxt.lib
                                         ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x86/Release/PVRTexLib.lib
                                         ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x86/Release/TextureConverter.lib)
set( STATIC_LIBRARIES_WIN32_DEBUG        ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x86/Debug/libdxt.lib
                                         ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x86/Debug/PVRTexLib.lib
                                         ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x86/Debug/TextureConverterD.lib)

set( DYNAMIC_LIBRARIES_WIN32_RELEASE     ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x86/Release/PVRTexLib.dll 
                                         ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x86/Release/TextureConverter.dll)
set( DYNAMIC_LIBRARIES_WIN32_DEBUG       ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x86/Debug/PVRTexLib.dll
                                         ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x86/Debug/TextureConverter_d.dll)


# win32 libs x64
set( STATIC_LIBRARIES_WIN64_RELEASE      ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x64/Release/libdxt.lib
                                         ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x64/Release/PVRTexLib.lib
                                         ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x64/Release/TextureConverter.lib)
set( STATIC_LIBRARIES_WIN64_DEBUG        ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x64/Debug/libdxt.lib
                                         ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x64/Debug/PVRTexLib.lib
                                         ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x64/Debug/TextureConverter_d.lib)

set( DYNAMIC_LIBRARIES_WIN64_RELEASE     ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x64/Release/PVRTexLib.dll
                                         ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x64/Release/TextureConverter.dll)
set( DYNAMIC_LIBRARIES_WIN64_DEBUG       ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x64/Debug/PVRTexLib.dll
                                         ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win32/x64/Debug/TextureConverter_d.dll)


# win10 libs
set( STATIC_LIBRARIES_WINUAP_RELEASE     ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win10/$(Platform)/Release/libdxt.lib)
set( STATIC_LIBRARIES_WINUAP_DEBUG       ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Win10/$(Platform)/Debug/libdxtd.lib)


# macos
set( STATIC_LIBRARIES_MACOS	         ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Mac/libdxt_macos.a
                                         ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Mac/libPVRTexLib.a)
set( DYNAMIC_LIBRARIES_MACOS             ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Mac/libTextureConverter.dylib)


# linux
set(STATIC_LIBRARIES_LINUX               ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Linux/libdxt.a)


set( DEFINITIONS_PRIVATE_WIN            -D_CRT_SECURE_NO_WARNINGS )
set( DEFINITIONS_PRIVATE_WINUAP         -D_CRT_SECURE_NO_WARNINGS )

#set path for data
set( MIX_APP_DATA_MACOS                 "Data/ = ${CMAKE_CURRENT_SOURCE_DIR}/Bin/PVRTexToolCLI")
set( MIX_APP_DATA_WIN                   "Data/ = ${CMAKE_CURRENT_SOURCE_DIR}/Bin/PVRTexToolCLI.exe")


set( MODULE_INITIALIZATION true )

setup_main_module()