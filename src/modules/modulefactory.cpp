#include "modulefactory.h"

ModuleFactory * ModuleFactory::Instance()
{
    static ModuleFactory factory;
    return &factory;
}


void ModuleFactory::RegisterModule(String name, function<Module*(void)> classFactoryFunction)
{
    // register the class factory function 
    factoryFunctionRegistry[name] = classFactoryFunction;
}


shared_ptr<Module> ModuleFactory::Create(String name)
{
    Module * instance = nullptr;

    // find name in the registry and call factory method.
    auto it = factoryFunctionRegistry.find(name);
    if(it != factoryFunctionRegistry.end())
        instance = it->second();
    
    // wrap instance in a shared ptr and return
    if(instance != nullptr)
        return std::shared_ptr<Module>(instance);
    else
        return nullptr;
}
