#include "libs/EmbedisWrap.h"
#include <EEPROM.h>


#include "libs/StreamInjector.h"
StreamInjector _serial = StreamInjector(SERIAL_PORT, TERMINAL_BUFFER_SIZE);

EmbedisWrap embedis(_serial, TERMINAL_BUFFER_SIZE);


// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------


unsigned int _settingsKeyCount() {
    unsigned count = 0;
    unsigned pos = SPI_FLASH_SEC_SIZE - 1;
    while (size_t len = EEPROM.read(pos)) {
        pos = pos - len - 2;
        len = EEPROM.read(pos);
        pos = pos - len - 2;
        count ++;
    }
    return count;
}

String _settingsKeyName(unsigned int index) {

    String s;

    unsigned count = 0;
    unsigned pos = SPI_FLASH_SEC_SIZE - 1;
    while (size_t len = EEPROM.read(pos)) {
        pos = pos - len - 2;
        if (count == index) {
            s.reserve(len);
            for (unsigned char i = 0 ; i < len; i++) {
                s += (char) EEPROM.read(pos + i + 1);
            }
            break;
        }
        count++;
        len = EEPROM.read(pos);
        pos = pos - len - 2;
    }

    return s;

}

std::vector<String> _settingsKeys() {

    // Get sorted list of keys
    std::vector<String> keys;

    //unsigned int size = settingsKeyCount();
    unsigned int size = _settingsKeyCount();
    for (unsigned int i=0; i<size; i++) {

        //String key = settingsKeyName(i);
        String key = _settingsKeyName(i);
        bool inserted = false;
        for (unsigned char j=0; j<keys.size(); j++) {

            // Check if we have to insert it before the current element
            if (keys[j].compareTo(key) > 0) {
                keys.insert(keys.begin() + j, key);
                inserted = true;
                break;
            }

        }

        // If we could not insert it, just push it at the end
        if (!inserted) keys.push_back(key);

    }

    return keys;
}

// -----------------------------------------------------------------------------
// Commands
// -----------------------------------------------------------------------------

void _settingsHelpCommand() {

    // Get sorted list of commands
    std::vector<String> commands;
    unsigned char size = embedis.getCommandCount();
    for (unsigned int i=0; i<size; i++) {

        String command = embedis.getCommandName(i);
        bool inserted = false;
        for (unsigned char j=0; j<commands.size(); j++) {

            // Check if we have to insert it before the current element
            if (commands[j].compareTo(command) > 0) {
                commands.insert(commands.begin() + j, command);
                inserted = true;
                break;
            }

        }

        // If we could not insert it, just push it at the end
        if (!inserted) commands.push_back(command);

    }

    // Output the list
    Log.notice(F("Available commands:" CR));
    for (unsigned char i=0; i<commands.size(); i++) {
        Log.notice(F("> %s" CR), (commands[i]).c_str());
    }

}

void _settingsClearCommand() {
    for (unsigned int i = 0; i < SPI_FLASH_SEC_SIZE; i++) {
        EEPROM.write(i, 0xFF);
    }
    EEPROM.commit();
    Log.notice(F("Settings cleared" CR));
}

void _settingsKeysCommand() {

    // Get sorted list of keys
    std::vector<String> keys = _settingsKeys();

    // Write key-values
    Log.notice(F("Current settings" CR));
    for (unsigned int i=0; i<keys.size(); i++) {
        String value = getSetting(keys[i]);
        Log.notice(F("> %s => %s" CR), (keys[i]).c_str(), value.c_str());
    }

    unsigned long freeEEPROM = SPI_FLASH_SEC_SIZE - settingsSize();
    Log.notice(F("Number of keys: %d" CR), keys.size());
    Log.notice(F("Free EEPROM: %d bytes (%d%%)" CR), freeEEPROM, 100 * freeEEPROM / SPI_FLASH_SEC_SIZE);

}

void _settingsInitCommands() {
    settingsRegisterCommand(F("COMMANDS"), [](Embedis* e) {
        _settingsHelpCommand();
    });
    settingsRegisterCommand(F("HELP"), [](Embedis* e) {
        _settingsHelpCommand();
    });
    settingsRegisterCommand(F("CLEAR"), [](Embedis* e) {
        _settingsClearCommand();
    });
    settingsRegisterCommand(F("KEYS"), [](Embedis* e) {
        _settingsKeysCommand();
    });
}

// -----------------------------------------------------------------------------
// Public
// -----------------------------------------------------------------------------


template<typename T> String getSetting(const String& key, T defaultValue) {
    String value;
    if (!Embedis::get(key, value)) value = String(defaultValue);
    return value;
}

String getSetting(const String& key) {
    return getSetting(key, "");
}

template<typename T> bool setSetting(const String& key, T value) {
    return Embedis::set(key, String(value));
}

bool delSetting(const String& key) {
    return Embedis::del(key);
}

bool hasSetting(const String& key) {
    return getSetting(key).length() != 0;
}

void settingsRegisterCommand(const String& name, void (*call)(Embedis*)) {
    Embedis::command(name, call);
};

unsigned long settingsSize() {
    unsigned pos = SPI_FLASH_SEC_SIZE - 1;
    while (size_t len = EEPROM.read(pos)) {
        pos = pos - len - 2;
    }
    return SPI_FLASH_SEC_SIZE - pos;
}

void settingsInject(void *data, size_t len) {
    _serial.inject((char *) data, len);
}

// -----------------------------------------------------------------------------
// Init and loop
// -----------------------------------------------------------------------------

void settingsSetup() {
    // Create a key-value Dictionary in emulated EEPROM (FLASH actually...)
    EEPROM.begin(SPI_FLASH_SEC_SIZE);
    Embedis::dictionary( 
        "EEPROM",
        SPI_FLASH_SEC_SIZE,
        [](size_t pos) -> char { return EEPROM.read(pos); },
        [](size_t pos, char value) { EEPROM.write(pos, value); },
        []() { EEPROM.commit(); }
    );

    _settingsInitCommands();
    registerLoop(settingsLoop);
}

void settingsLoop() {
    embedis.process();
}