#ifndef WIRING_PI_PORT_H
#define WIRING_PI_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_WIRING_PI
#include <wiringPi.h>
#include <wiringPiSPI.h>
#else

#define INPUT 0
#define OUTPUT 1
#define PUD_UP 2

#define LOW 0
#define HIGH 1

static inline void pinMode(int pin, int mode)
{
    (void)pin;
    (void)mode;
}

static inline void digitalWrite(int pin, int value)
{
    (void)pin;
    (void)value;
}

static inline int digitalRead(int pin)
{
    (void)pin;
    return 0;
}

static inline void delay(uint32_t ms)
{
    (void)ms;
}

static int wiringPiSetupGpio(void)
{
    return 0;
}

static inline void pullUpDnControl(int pin, int pud)
{
    (void)pin;
    (void)pud;
}

static inline int wiringPiSPISetupMode(int channel, int speed, int mode)
{
    (void)channel;
    (void)speed;
    (void)mode;
    return 0;
}

static uint8_t wiringPiSPIDataRW(int channel, const uint8_t* data, int len)
{
    (void)channel;
    (void)data;
    (void)len;
    return 0;
}

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WIRING_PI_PORT_H */
