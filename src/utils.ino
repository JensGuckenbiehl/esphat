String buildTime() {

    const char time_now[] = __TIME__;   // hh:mm:ss
    unsigned int hour = atoi(&time_now[0]);
    unsigned int minute = atoi(&time_now[3]);
    unsigned int second = atoi(&time_now[6]);

    const char date_now[] = __DATE__;   // Mmm dd yyyy
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    unsigned int month = 0;
    for ( int i = 0; i < 12; i++ ) {
        if (strncmp(date_now, months[i], 3) == 0 ) {
            month = i + 1;
            break;
        }
    }
    unsigned int day = atoi(&date_now[3]);
    unsigned int year = atoi(&date_now[7]);

    char buffer[20];
    snprintf_P(
        buffer, sizeof(buffer), PSTR("%04d-%02d-%02d %02d:%02d:%02d"),
        year, month, day, hour, minute, second
    );

    return String(buffer);
}

String getChipId() {
    char buffer[8];
    #if defined(ARDUINO_ARCH_ESP8266)
        snprintf_P(buffer, sizeof(buffer), PSTR("%06X"), ESP.getChipId());
    #elif defined(ARDUINO_ARCH_ESP32)
        snprintf_P(buffer, sizeof(buffer), PSTR("%08X"), (uint32_t)ESP.getEfuseMac());
    #endif
    
    return String(buffer);
}

unsigned int getFreeHeap() {
    return ESP.getFreeHeap();
}

unsigned long getUptime() {

    static unsigned long last_uptime = 0;
    static unsigned char uptime_overflows = 0;

    if (millis() < last_uptime) ++uptime_overflows;
    last_uptime = millis();
    unsigned long uptime_seconds = uptime_overflows * (UPTIME_OVERFLOW / 1000) + (last_uptime / 1000);

    return uptime_seconds;

}

unsigned int sectors(size_t size) {
    return (int) (size + SPI_FLASH_SEC_SIZE - 1) / SPI_FLASH_SEC_SIZE;
}