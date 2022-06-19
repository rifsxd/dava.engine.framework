#pragma once

#include "TArc/Core/ClientModule.h"

#include <Base/BaseTypes.h>
#include <Base/StaticSingleton.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Functional/Function.h>

namespace DAVA
{
class ModuleCollection : public StaticSingleton<ModuleCollection>
{
public:
    using TypeCreateFn = Function<const ReflectedType*()>;
    void AddGuiModule(const TypeCreateFn& type);
    void AddConsoleModule(const TypeCreateFn& type);

    Vector<const ReflectedType*> GetGuiModules() const;
    Vector<const ReflectedType*> GetConsoleModules() const;

private:
    Vector<TypeCreateFn> guiModules;
    Vector<TypeCreateFn> consoleModules;
};

template <typename T>
struct ModuleInitializer
{
    ModuleInitializer()
    {
        auto tp = std::integral_constant<bool, std::is_base_of<ClientModule, T>::value>();
        AddModuleIntoCollection(tp);
    }
    void AddModuleIntoCollection(std::true_type)
    {
        ModuleCollection::Instance()->AddGuiModule(ModuleCollection::TypeCreateFn(&ModuleInitializer<T>::GetType));
    }

    void AddModuleIntoCollection(std::false_type)
    {
        ModuleCollection::Instance()->AddConsoleModule(ModuleCollection::TypeCreateFn(&ModuleInitializer<T>::GetType));
    }

    static const ReflectedType* GetType()
    {
        return ReflectedTypeDB::Get<T>();
    }
};
} // namespace DAVA

#define DECL_TARC_MODULE(className) ::DAVA::ModuleInitializer<className> initializer_##className
