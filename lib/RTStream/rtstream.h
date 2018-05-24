#pragma once

#include <Stream.h>

class RTStream : public Stream {
 private:
  Stream* const rxStream;
  Stream* const txStream;

 public:
  RTStream(Stream& rxstream, Stream& txstream);
  virtual ~RTStream();

  virtual int available();
  virtual int peek();
  virtual int read();

  virtual void flush();

  virtual size_t write(uint8_t c);
  virtual size_t write(const uint8_t* buffer, size_t size);
};