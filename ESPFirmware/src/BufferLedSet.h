//
// Created by leonhard on 23.12.21.
//

#ifndef ESPFIRMWARE_BUFFERLEDSET_H
#define ESPFIRMWARE_BUFFERLEDSET_H

//#include <cstring>
#include "Arduino.h"
#include <FastLED.h>


#define BufferTotalFramesCount 25
#define BufferLengthMicrosecounds 1000000
#define SingleFrameLength (BufferLengthMicrosecounds / BufferTotalFramesCount)

using namespace std;

class BufferLedSet {
private:
    uint32_t startTimestamp;
    unsigned char *data;
    uint32_t getFrameStartTime();
public:
    BufferLedSet(String& base64EncodedData);
    ~BufferLedSet();
    /**
     * @brief Set the Leds according to the state it should be at the given time
     * 
     * @param leds The LEDs to be set
     * @param time The time at which the LEDs should be set to
     * @return true This buffer is valid
     * @return false This buffer has expired, please try a newer one. 
     */
    bool setLedsAtTime(CRGB* leds, uint8_t size, const uint32_t& time);
};


#endif //ESPFIRMWARE_BUFFERLEDSET_H
