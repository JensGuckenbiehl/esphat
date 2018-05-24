// -----------------------------------------------------------------------------
// Terminal
// -----------------------------------------------------------------------------

#define TERMINAL_PORT SERIAL_PORT
#ifdef SERIAL_PORT_OUT
RTStream rtstream(SERIAL_PORT, SERIAL_PORT_OUT);
#undef TERMINAL_PORT
#define TERMINAL_PORT rtstream
#endif

Terminal terminal(TERMINAL_PORT);

void terminalSetup() {
#ifdef SERIAL_PORT
  SERIAL_PORT.begin(115200);
#endif
#ifdef SERIAL_PORT_OUT
  SERIAL_PORT_OUT.begin(115200);
#endif

  registerLoop(terminalLoop);
}

void terminalLoop() {
  terminal.loop();
}
