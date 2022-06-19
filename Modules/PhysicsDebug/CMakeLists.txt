set(MODULE_NAME PhysicsDebug )

set(MODULE_TYPE STATIC)#INLINE STATIC DYNAMIC 
set(PLATFORMS_IGNORE ANDROID WINUAP_ARM )                

set(HPP_FILES_RECURSE Sources/*.h Sources/*.hpp)
set(CPP_FILES_RECURSE Sources/*.cpp)

set(DEFINITIONS "-D__DAVAENGINE_PHYSICS_DEBUG_DRAW_ENABLED__")

set(INCLUDES Sources)
set(INCLUDES_PRIVATE ${DAVA_INCLUDE_DIR}
                     Libs/include/physx
                     Libs/include/PxShared)

set(PhysicsDebugRootDir "${CMAKE_CURRENT_SOURCE_DIR}")
set(PhysicsRootDir "${PhysicsDebugRootDir}/../Physics")
set(LibsPath "${PhysicsRootDir}/Libs")

set(WIN32_DYNAMIC_DEBUG_LIBS "${LibsPath}/Win32/x86/nvToolsExt32_1.dll")
set(WIN32_DYNAMIC_CHECKED_LIBS "${LibsPath}/Win32/x86/nvToolsExt32_1.dll")
set(WIN32_DYNAMIC_PROFILE_LIBS "${LibsPath}/Win32/x86/nvToolsExt32_1.dll")

set(WIN64_DYNAMIC_DEBUG_LIBS "${LibsPath}/Win32/x64/nvToolsExt64_1.dll")
set(WIN64_DYNAMIC_CHECKED_LIBS "${LibsPath}/Win32/x64/nvToolsExt64_1.dll")
set(WIN64_DYNAMIC_PROFILE_LIBS "${LibsPath}/Win32/x64/nvToolsExt64_1.dll")

set(DYNAMIC_LIBRARIES_WIN32_DEBUG "${WIN32_DYNAMIC_DEBUG_LIBS}")
set(DYNAMIC_LIBRARIES_WIN64_DEBUG "${WIN64_DYNAMIC_DEBUG_LIBS}")

set(MODULE_INITIALIZATION true )

setup_main_module()
