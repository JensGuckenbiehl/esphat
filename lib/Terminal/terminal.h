#pragma once

#include <Arduino.h>
#include <Stream.h>
#include <stdarg.h>
#include <Vector>
#include <map>

// Keycode defines.
#define KEYCODE_BACKSPACE 8
#define KEYCODE_TAB 9
#define KEYCODE_ENTER 13
#define KEYCODE_SPACE 32
#define KEYCODE_UP 65
#define KEYCODE_DOWN 66
#define KEYCODE_DELETE 127

enum class Loglevel { INFO, WARNING, ERROR, COMMAND };

typedef std::function<void(int, char**, Print*)> command_f;

class Terminal : public Print {
 private:
  class LogPrint : public Print {
   public:
    virtual size_t write(uint8_t c) {
      for (Terminal* terminal : logTerminals) {
        terminal->write(c);
      }
    }
    virtual size_t write(const uint8_t* buffer, size_t size) {
      for (Terminal* terminal : logTerminals) {
        terminal->write(buffer, size);
      }
    }
  };

  size_t buflen;
  char* buf;
  size_t pos;
  Stream* const stream;

  String prompt;

  void printPrompt();
  void clearLine();
  void printBuffer();

  void logStart(Loglevel level);
  void logMessage(const char* format, va_list args);
  void logMessage(const __FlashStringHelper* format, va_list args);
  void logEnd();
  void logFormat(const char format, va_list* args);

  static std::vector<Terminal*> logTerminals;
  static std::map<String, command_f> commands;

  static void addDefaultCommands();
  static void helpCommand(int argc, char** argv, Print* response);
  static void executeCommand(char* command, Print* output);

 public:
  // Constructor / destructor / disallow copy and move
  Terminal(Stream& stream, size_t buflen = 128);
  ~Terminal();
  Terminal(const Terminal&);
  Terminal& operator=(const Terminal&);

  virtual size_t write(uint8_t c);
  virtual size_t write(const uint8_t* buffer, size_t size);

  void setPrompt(const String& prompt);
  void loop();

  template <class T, typename... Args>
  static void log(T msg, Args... args) {
    logLevel(Loglevel::INFO, msg, args...);
  }

  template <class T>
  static void logLevel(Loglevel level, T msg, ...) {
    va_list args;
    va_start(args, msg);
    for (Terminal* terminal : logTerminals) {
      terminal->logStart(level);
      terminal->logMessage(msg, args);
      if (!rawLog)
        terminal->logEnd();
    }
  }

  static bool rawLog;

  static void executeCommand(const char* command);
  static void addCommand(const String& name, command_f command);
  static void removeCommand(const String& name);
};