#include "TArc/Utils/ModuleCollection.h"

namespace DAVA
{
void ModuleCollection::AddGuiModule(const TypeCreateFn& type)
{
    guiModules.push_back(type);
}

void ModuleCollection::AddConsoleModule(const TypeCreateFn& type)
{
    consoleModules.push_back(type);
}

Vector<const ReflectedType*> ModuleCollection::GetGuiModules() const
{
    Vector<const ReflectedType*> types;
    for (const TypeCreateFn& fn : guiModules)
    {
        types.push_back(fn());
    }

    return types;
}

Vector<const ReflectedType*> ModuleCollection::GetConsoleModules() const
{
    Vector<const ReflectedType*> types;
    for (const TypeCreateFn& fn : consoleModules)
    {
        types.push_back(fn());
    }

    return types;
}
} // namespace DAVA
