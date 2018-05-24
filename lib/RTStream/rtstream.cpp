#include "rtstream.h"

RTStream::RTStream(Stream& rxstream, Stream& txstream)
    : rxStream(&rxstream), txStream(&txstream) {}

RTStream::~RTStream() {}

int RTStream::available() {
  return rxStream->available();
}

int RTStream::peek() {
  return rxStream->peek();
}
int RTStream::read() {
  return rxStream->read();
}

void RTStream::flush() {
  txStream->flush();
}

size_t RTStream::write(uint8_t c) {
  return txStream->write(c);
}
size_t RTStream::write(const uint8_t* buffer, size_t size) {
  return txStream->write(buffer, size);
}