set( MODULE_NAME RemoteTool )

set( MODULE_TYPE STATIC )

set( HPP_FILES_RECURSE Sources/RemoteTool/Private/*.h  )
set( HPP_FILES_RECURSE Sources/RemoteTool/*.h  )
set( CPP_FILES_RECURSE Sources/RemoteTool/Private/*.cpp )

set( CPP_FILES_EXECUTE Sources/RemoteTool/InitRemoteTool.cpp )

set( QT_UI_FILES "${CMAKE_CURRENT_LIST_DIR}/DataQt/*.ui")

set( INCLUDES Sources )
set( INCLUDES_PRIVATE   ${CMAKE_CURRENT_BINARY_DIR} 
                        ${CMAKE_CURRENT_LIST_DIR} 
                        ${DAVA_INCLUDE_DIR} 
                        Sources/RemoteTool/Private/MemoryTool/Widgets )

set( DEFINITIONS_PRIVATE_WIN      -D_CRT_SECURE_NO_WARNINGS )
set( DEFINITIONS_PRIVATE_WINUAP   -D_CRT_SECURE_NO_WARNINGS )

set( FIND_MODULE NetworkHelpers LoggerService MemoryProfilerService)

set( DEFINITIONS_PRIVATE -D__DAVAENGINE_QT__ )

setup_main_module()
