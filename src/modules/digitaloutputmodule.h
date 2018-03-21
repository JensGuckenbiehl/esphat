#ifndef DIGITALOUTPUTMODULE_H
#define DIGITALOUTPUTMODULE_H

#include "module.h"

class DigitalOutputModule : public Module
{
private:
    int _outputNumber;
    String _statusTopic;
    String _setTopic;
    String _getTopic;
    bool _isValid();
    void _registerTopics();
public:
    DigitalOutputModule();
    ~DigitalOutputModule();
    void start();
    void stop();
    void mqtt(unsigned int type, const char * topic, const char * payload);
protected: 
    String serialize();
    void deserialize(const String& json);
};

#endif // DIGITALOUTPUTMODULE_H