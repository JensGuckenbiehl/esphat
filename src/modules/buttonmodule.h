#ifndef BUTTONMODULE_H
#define BUTTONMODULE_H

#include "module.h"

#include "libs/MultiButton.h"

class ButtonModule : public Module
{
private:
    int _gpio;
    String _clickCommand;
    String _doubleClickCommand;
    String _longClickCommand;
    MultiButton _multiButton;
public:
    ButtonModule();
    ~ButtonModule();
    void start();
    void stop();
    void loop();
protected: 
    String serialize();
    void deserialize(const String& json);
};

#endif // BUTTONMODULE_H