#include "terminal.h"

#include <algorithm>

std::vector<Terminal*> Terminal::logTerminals;
std::map<String, command_f> Terminal::commands;

bool Terminal::rawLog = false;

Terminal::Terminal(Stream& stream, size_t buflen)
    : buflen(buflen),
      buf((char*)malloc(buflen * sizeof(char))),
      pos(0),
      stream(&stream),
      prompt(">") {
  logTerminals.push_back(this);
  printPrompt();
}

Terminal::~Terminal() {
  free(buf);
  logTerminals.erase(
      std::remove(logTerminals.begin(), logTerminals.end(), this),
      logTerminals.end());
}

void Terminal::logStart(Loglevel level) {
  clearLine();

  char c[14];
  int m = sprintf(c, "[%10lu] ", millis());
  stream->print(c);

  switch (level) {
    case Loglevel::INFO:
      stream->write("\33[39m");
      break;
    case Loglevel::ERROR:
      stream->write("\33[31m");
      break;
    case Loglevel::WARNING:
      stream->write("\33[33m");
      break;
    case Loglevel::COMMAND:
      stream->write("\33[35m");
      break;
    default:
      break;
  }
}

void Terminal::logMessage(const char* format, va_list args) {
  for (; *format != 0; ++format) {
    if (*format == '%') {
      ++format;
      logFormat(*format, &args);
    } else {
      stream->print(*format);
    }
  }
}

void Terminal::logMessage(const __FlashStringHelper* format, va_list args) {
  PGM_P p = reinterpret_cast<PGM_P>(format);
  char c = pgm_read_byte(p++);
  for (; c != 0; c = pgm_read_byte(p++)) {
    if (c == '%') {
      c = pgm_read_byte(p++);
      logFormat(c, &args);
    } else {
      stream->print(c);
    }
  }
}

void Terminal::logEnd() {
  stream->write("\33[39m");
  stream->write("\r\n");
  printPrompt();
  printBuffer();
}

void Terminal::logFormat(const char format, va_list* args) {
  if (format == '\0')
    return;

  if (format == '%') {
    stream->print(format);
    return;
  }

  if (format == 's') {
    register char* s = (char*)va_arg(*args, int);
    stream->print(s);
    return;
  }

  if (format == 'd' || format == 'i') {
    stream->print(va_arg(*args, int), DEC);
    return;
  }

  if (format == 'D' || format == 'F') {
    stream->print(va_arg(*args, double));
    return;
  }

  if (format == 'x') {
    stream->print(va_arg(*args, int), HEX);
    return;
  }

  if (format == 'X') {
    stream->print("0x");
    stream->print(va_arg(*args, int), HEX);
    return;
  }

  if (format == 'b') {
    stream->print(va_arg(*args, int), BIN);
    return;
  }

  if (format == 'B') {
    stream->print("0b");
    stream->print(va_arg(*args, int), BIN);
    return;
  }

  if (format == 'l') {
    stream->print(va_arg(*args, long), DEC);
    return;
  }

  if (format == 'c') {
    stream->print((char)va_arg(*args, int));
    return;
  }

  if (format == 't') {
    if (va_arg(*args, int) == 1) {
      stream->print("T");
    } else {
      stream->print("F");
    }
    return;
  }

  if (format == 'T') {
    if (va_arg(*args, int) == 1) {
      stream->print(F("true"));
    } else {
      stream->print(F("false"));
    }
    return;
  }
}

void Terminal::clearLine() {
  stream->write("\33[2K\r");
}

void Terminal::printPrompt() {
  // print prompt in green
  stream->write("\33[32m");
  stream->print(prompt);
  // reset to default color
  stream->write("\33[39m");
}

void Terminal::printBuffer() {
  stream->print(buf);
}

void Terminal::setPrompt(const String& newprompt) {
  prompt = newprompt;
}

void Terminal::loop() {
  int i;
  while ((i = stream->read()) >= 0) {
    char c = i;

    switch (c) {
      case '\r':
        // send it to the handler for processing.
        stream->print("\r\n");
        executeCommand((char*)buf, this);
        printPrompt();
        pos = 0;
        buf[pos] = '\0';
        break;

      case KEYCODE_BACKSPACE:
      case KEYCODE_DELETE:
        // backspace
        if (pos > 0) {
          stream->write(KEYCODE_BACKSPACE);
          stream->write(KEYCODE_SPACE);
          stream->write(KEYCODE_BACKSPACE);
          buf[pos--] = '\0';
        }
        break;
      default:
        // normal character entered. add it to the buffer and terminate
        if (pos < buflen) {
          stream->print(c);
          buf[pos++] = c;
        }
        if (pos >= buflen) {
          clearLine();
          stream->println(F("Line buffer overflow"));
          printBuffer();
        }
        buf[pos] = '\0';

        break;
    }
  }
}

size_t Terminal::write(uint8_t c) {
  return stream->write(c);
}
size_t Terminal::write(const uint8_t* buffer, size_t size) {
  return stream->write(buffer, size);
}

void Terminal::addCommand(const String& name, command_f command) {
  commands[name] = command;
}

void Terminal::removeCommand(const String& name) {
  std::map<String, command_f>::iterator it = commands.find(name);
  if (it != commands.end()) {
    commands.erase(it);
  }
}

void Terminal::executeCommand(const char* command) {
  char* copy = strdup(command);
  rawLog = true;
  logLevel(Loglevel::COMMAND, "Execute command\r\n\33[39m");
  LogPrint logprint;
  executeCommand(copy, &logprint);
  rawLog = false;
  logLevel(Loglevel::COMMAND, "Finished execution of command");
  free(copy);
}

void Terminal::executeCommand(char* command, Print* output) {
  addDefaultCommands();

  uint8_t argc, i = 0;
  char* argv[30];

  argv[i] = strtok(command, " ");
  do {
    argv[++i] = strtok(NULL, " ");
  } while ((i < 30) && (argv[i] != NULL));

  // save off the number of arguments for the particular command.
  argc = i;

  String commandString = argv[0];
  commandString.toUpperCase();
  std::map<String, command_f>::iterator it = commands.find(commandString);
  if (it != commands.end()) {
    it->second(argc, argv, output);
  } else {
    output->write("\33[31m");
    output->println(F("Unknown command"));
    output->write("\33[39m");
  }
}

void Terminal::addDefaultCommands() {
  static bool added = false;
  if (added)
    return;
  added = true;
  addCommand(F("HELP"), helpCommand);
}

void Terminal::helpCommand(int argc, char** argv, Print* response) {
  response->println(F("Available commands:"));
  for (std::map<String, command_f>::iterator it = commands.begin();
       it != commands.end(); ++it) {
    response->println(it->first);
  }
}