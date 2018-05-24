#ifndef HARDWARE_H
#define HARDWARE_H

#if defined(HS8266ULN8)
// Info
#define MANUFACTURER "LOOKSOFT"
#define DEVICE "HS8266ULN8"

#define SERIAL_PORT Serial

#define OUTPUT_SUPPORT

#elif defined(UP32I2C)
// Info
#define MANUFACTURER "LOOKSOFT"
#define DEVICE "UP32I2C"

#define SERIAL_PORT Serial

#elif defined(H801)
// Info
#define MANUFACTURER "LOOKSOFT"
#define DEVICE "H801"

#define SERIAL_PORT Serial
#define SERIAL_PORT_OUT Serial1

#endif

#endif  // HARDWARE_H