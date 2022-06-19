set( MODULE_NAME TexturePacker )

set( MODULE_TYPE STATIC )

set( PLATFORMS_SUPPORTED WIN MACOS)                


set( HPP_FILES_RECURSE Sources/*.h )
set( CPP_FILES_RECURSE Sources/*.cpp )

set( INCLUDES  Sources )
set( INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR})

set( DEFINITIONS_PRIVATE_WIN      -D_CRT_SECURE_NO_WARNINGS )
set( DEFINITIONS_PRIVATE_WINUAP   -D_CRT_SECURE_NO_WARNINGS )

set(FIND_MODULE AssetCache 
		TextureCompression)

setup_main_module()
