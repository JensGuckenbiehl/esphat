/*

OUTPUT MODULE

*/

#ifdef OUTPUT_SUPPORT

#include <Adafruit_MCP23017.h>

Adafruit_MCP23017 mcp; // Create MCP
bool _manualMode;
uint8_t _manualOutput;
uint8_t _output;
uint8_t _selectedOutput;

void _writeOutput() {
    if(_manualMode) {
        EEPROM.write(EEPROM_OUTPUT_STATUS, ~_manualOutput);
        mcp.writeGPIO(0, _manualOutput);
        Log.notice(F("[OUTPUT] Write manual output: %d\n"), _manualOutput);
    } else {
        EEPROM.write(EEPROM_OUTPUT_STATUS, ~_output);
        mcp.writeGPIO(0, _output);
        Log.notice(F("[OUTPUT] Write output: %d\n"), _output);
    }
    EEPROM.commit();
}

void _writeSelected() {
    Log.notice(F("[OUTPUT] Write selected: %d\n"), _selectedOutput);
    mcp.writeGPIO(1, _selectedOutput);
}

bool manualModeOutput() {
    return _manualMode;
}

void toggleManualModeOutput() {
    setManualModeOutput(!manualModeOutput());
}

void setManualModeOutput(bool manualMode) {
    _manualMode = manualMode;
    _manualOutput = _output;
    _writeOutput();
}

void setOutput(int id, bool value) {
    _selectedOutput = 1 << id;
    if(value) {
        _output = _output | _selectedOutput;
    } else {
        _output = _output & ~_selectedOutput;
    }
    _writeOutput();
}

bool getOutput(int id) {
    _selectedOutput = 1 << id;
    return _output & _selectedOutput;
}

void toggleSelectedOutput() {
    _manualOutput = _manualOutput ^ _selectedOutput;
    _writeOutput();
}

void selectNextOutput() {
    _selectedOutput = static_cast<uint8_t>((_selectedOutput << 1) | (_selectedOutput >> 7));
    _writeSelected();
}

void selectPreviousOutput() {
    _selectedOutput = static_cast<uint8_t>((_selectedOutput >> 1) | (_selectedOutput << 7));
    _writeSelected();
}

void outputSetup() {
    _manualMode = false;
    _selectedOutput = 1;

    mcp.begin();      // Start MCP on Hardware address 0x20

    // Reload status from EEPROM
    _output = EEPROM.read(EEPROM_OUTPUT_STATUS);
    _output = ~_output;
    _manualOutput = _output;

    _writeOutput();
    _writeSelected();

    _outputInitCommands();

}

// -----------------------------------------------------------------------------
// SETTINGS
// -----------------------------------------------------------------------------

void _outputInitCommands() {

    settingsRegisterCommand(F("OUTPUT.NEXTOUTPUT"), [](Embedis* e) {
        selectNextOutput();
    });

    settingsRegisterCommand(F("OUTPUT.PREVIOUSOUTPUT"), [](Embedis* e) {
        selectPreviousOutput();
    });

    settingsRegisterCommand(F("OUTPUT.TOOGLEOUTPUT"), [](Embedis* e) {
        toggleSelectedOutput();
    });

    settingsRegisterCommand(F("OUTPUT.TOOGLEMANUALMODE"), [](Embedis* e) {
        toggleManualModeOutput();
    });
}

#endif