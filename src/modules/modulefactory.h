#ifndef MODULEFACTORY_H
#define MODULEFACTORY_H

#include "module.h"

#include <memory>
#include <Arduino.h>
#include <map>
#include <functional>

using namespace std;

// The factory - implements singleton pattern!
class ModuleFactory
{
public:
    // Get the single instance of the factory
    static ModuleFactory * Instance();

    // register a factory function to create an instance of className
    void RegisterModule(String name, function<Module*(void)> classFactoryFunction);

    // create an instance of a registered class
    shared_ptr<Module> Create(String name);

private:
    // a private ctor
    ModuleFactory(){}

    // the registry of factory functions
    std::map<String, function<Module*(void)>> factoryFunctionRegistry;

};

// A helper class to register a factory function
template<class T>
class ModuleRegistrar {
public:
    ModuleRegistrar(String className)
    {
        // register the class factory function 
        ModuleFactory::Instance()->RegisterModule(className,
                [](void) -> Module * { return new T();});
    }
};

#define REGISTER_MODULE(NAME, TYPE) static ModuleRegistrar<TYPE> registrar(NAME);

#endif // MODULEFACTORY_H