
set( MODULE_NAME Sample )                     # set name to module 

set(PLATFORMS_IGNORE ANDROID )                

set( MODULE_TYPE STATIC )                     # set module type [INLINE, STATIC, DYNAMIC]

set( HPP_FILES_RECURSE Sources/*.h )
set( CPP_FILES_RECURSE Sources/*.cpp )

set( INCLUDES         Sources)                # set search path headers for all project
set( INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR} )   # set search path headers inside module

set( MODULE_INITIALIZATION true )

setup_main_module()                           # induce macros of module formation