#ifndef LEDMODULE_H
#define LEDMODULE_H

#include "module.h"

#include <TTLED.h>

class LedModule : public Module
{
private:
    int _gpio;
    String _mode;
    TTLED *_led; 
public:
    LedModule();
    ~LedModule();
    void start();
    void stop();
    void loop();
protected: 
    String serialize();
    void deserialize(const String& json);
};

#endif // LEDMODULE_H