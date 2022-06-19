
set( DAVA_STATIC_LIBRARIES_IOS      ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcrypto.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcurl_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfreetype_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libicucommon_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libjpeg_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libmongodb_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libogg_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpng_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libssl.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libtheora_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libwebp.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libxml_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libyaml_ios.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libsqlite3_ios.a
                                    )

set( DAVA_STATIC_LIBRARIES_MACOS    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libFColladaS.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcrypto.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcurl_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfreetype_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libicucommon_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libjpeg_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libmongodb_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libogg_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpng_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpsd.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libssl.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libtheora_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libvorbis_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libvorbisfile_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libwebp.a 
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libxml_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libyaml_macos.a
                                    ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libsqlite3.a
                                    )

set( DAVA_STATIC_LIBRARIES_ANDROID  "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libxml.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpng.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfreetype.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libyaml.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libmongodb.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libjpeg.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcurl.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libssl.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcrypto.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libicucommon.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libwebp.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libsqlite3.a"
                                    "-lEGL"
                                    "-lGLESv1_CM"
                                    "-llog"
                                    "-landroid"
                                    "-lGLESv2"
                                    "-latomic" 
                                    )

set( DAVA_STATIC_LIBRARIES_LINUX
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcurl.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libfreetype.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libjpeg.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/liblua.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libmongodb.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libpng.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libsqlite3.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libtheora.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libunibreak.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libuv.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libvorbis.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libvorbisfile.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libwebp.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libxml.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libyaml.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libogg.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libssl.a"
                                    "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/libcrypto.a"
                                    "-ldl"
                                    "-lz"
                                    )

if( WIN ) 

    set( DAVA_STATIC_LIBRARIES_WIN32_RELEASE
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/detours.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/FColladaVS2010.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/freetype.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/glew32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/icucommon.lib"         
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libcurl.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libeay32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/ssleay32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libjpeg.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libmongodb.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libpsd.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libuv.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libwebp.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libxml2.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libyaml.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libpng.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/unibreak.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/zlib.lib"   
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avcodec.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avdevice.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avfilter.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avformat.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avutil.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/postproc.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/swresample.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/swscale.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/sqlite3.lib"   )

    set( DAVA_STATIC_LIBRARIES_WIN32_DEBUG
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/detours.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/FColladaVS2010.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/freetype.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/glew32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/icucommon.lib"         
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libcurl.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libeay32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/ssleay32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libjpeg.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libmongodb.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libpsd.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libuv.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libwebp.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libxml2.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libyaml.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libpng.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/unibreak.lib"  
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/zlib.lib" 
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avcodec.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avdevice.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avfilter.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avformat.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avutil.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/postproc.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/swresample.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/swscale.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/sqlite3.lib" )

    set( DAVA_STATIC_LIBRARIES_WIN64_RELEASE
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/detours.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/FColladaVS2010.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/freetype.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/glew32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/icucommon.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libjpeg.lib"              
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libcurl.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libeay32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/ssleay32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libmongodb.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libpsd.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libwebp.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libwebpdecoder.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libxml2.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libyaml.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libpng.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/unibreak.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/libuv.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/zlib.lib" 
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avcodec.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avdevice.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avfilter.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avformat.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avutil.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/postproc.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/swresample.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/swscale.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/sqlite3.lib" )

    set( DAVA_STATIC_LIBRARIES_WIN64_DEBUG
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/detours.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libeay32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/ssleay32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/FColladaVS2010.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/freetype.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/glew32.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/icucommon.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libjpeg.lib"            
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libcurl.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libmongodb.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libpsd.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libuv.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libwebp.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libxml2.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libyaml.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/libpng.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/unibreak.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/zlib.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avcodec.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avdevice.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avfilter.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avformat.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/avutil.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/postproc.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/swresample.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/swscale.lib"
                       "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Debug/sqlite3.lib")


    set( DAVA_STATIC_LIBRARIES_WIN32 "Wininet.lib"
                                     "opengl32.lib"
                                     "ws2_32.lib"
                                     "winmm.lib"
                                     "wldap32.lib"
                                     "iphlpapi.lib"
                                     "Gdi32.lib"
                                     "glu32.lib"
                                     "psapi.lib"
                                     "dbghelp.lib"
                                     "userenv.lib"
                                     "delayimp.lib"
                                     "dxgi.lib" )

    set( DAVA_DYNAMIC_LIBRARIES_WIN32 "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avcodec-57.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avdevice-57.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avfilter-6.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avformat-57.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/avutil-55.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/postproc-54.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/swresample-2.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/swscale-4.dll" 
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/D3DCompiler_43.dll" 
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/d3dx9_43.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/glew32.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/msvcr120.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/msvcp120.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/msvcp140.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/vcruntime140.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-console-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-datetime-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-debug-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-errorhandling-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-file-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-file-l1-2-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-file-l2-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-handle-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-heap-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-interlocked-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-libraryloader-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-localization-l1-2-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-memory-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-namedpipe-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-processenvironment-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-processthreads-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-processthreads-l1-1-1.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-profile-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-rtlsupport-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-string-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-synch-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-synch-l1-2-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-sysinfo-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-timezone-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-core-util-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-conio-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-convert-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-environment-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-filesystem-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-heap-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-locale-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-math-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-multibyte-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-private-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-process-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-runtime-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-stdio-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-string-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-time-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/api-ms-win-crt-utility-l1-1-0.dll"
                                      "${DAVA_THIRD_PARTY_LIBRARIES_PATH}/Release/ucrtbase.dll"
    )

    set( DAVA_DYNAMIC_LIBRARIES_WIN64 ${DAVA_DYNAMIC_LIBRARIES_WIN32} )


    set ( DAVA_STATIC_LIBRARIES_WIN64 ${DAVA_STATIC_LIBRARIES_WIN32} )

    list ( APPEND DAVA_STATIC_LIBRARIES_WIN32
            "$ENV{DXSDK_DIR}/lib/x86/d3dx9.lib"
            "$ENV{DXSDK_DIR}/lib/x86/d3d9.lib"
            "$ENV{DXSDK_DIR}/lib/x86/d3d11.lib"
            "$ENV{DXSDK_DIR}/lib/x86/d3dcompiler.lib"
            "$ENV{DXSDK_DIR}/lib/x86/dxguid.lib"
        )

    list ( APPEND DAVA_STATIC_LIBRARIES_WIN64
            "$ENV{DXSDK_DIR}/lib/x64/d3dx9.lib"
            "$ENV{DXSDK_DIR}/lib/x64/d3d9.lib"
            "$ENV{DXSDK_DIR}/lib/x64/d3d11.lib"
            "$ENV{DXSDK_DIR}/lib/x64/d3dcompiler.lib"
            "$ENV{DXSDK_DIR}/lib/x64/dxguid.lib"
        )

endif()

if( WINUAP ) 
    add_static_libs_win_uap ( "${DAVA_WIN_UAP_LIBRARIES_PATH_COMMON}" LIST_SHARED_LIBRARIES )

    set( DAVA_STATIC_LIBRARIES_WINUAP   "OneCore.lib"
                                        "d2d1.lib"
                                        "d3d11.lib"
                                        "d3dcompiler.lib"
                                        "dxgi.lib"
                                        "dxguid.lib"
                                        "dwrite.lib"
                                        ${LIST_SHARED_LIBRARIES} )
                                        
    set( DAVA_STATIC_LIBRARIES_WINUAP_RELEASE ${LIST_SHARED_LIBRARIES_RELEASE} )
    set( DAVA_STATIC_LIBRARIES_WINUAP_DEBUG   ${LIST_SHARED_LIBRARIES_DEBUG} )

endif()
