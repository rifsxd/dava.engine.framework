#include "Modules/PreferencesModule/PreferencesModule.h"
#include "Modules/PreferencesModule/PreferencesData.h"

#include <TArc/Utils/ModuleCollection.h>

DAVA_VIRTUAL_REFLECTION_IMPL(PreferencesModule)
{
    DAVA::ReflectionRegistrator<PreferencesModule>::Begin()
    .ConstructorByPointer()
    .End();
}

PreferencesModule::PreferencesModule()
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PreferencesData);
}

void PreferencesModule::PostInit()
{
}

DECL_TARC_MODULE(PreferencesModule);
