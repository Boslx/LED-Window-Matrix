#include "BufferLedSet.h"

extern "C"
{
    #include "crypto/base64.h"
}


BufferLedSet::BufferLedSet(String& base64EncodedData) 
{
  const char *toDecode = base64EncodedData.c_str();
  size_t outputLength;
  BufferLedSet::data = base64_decode((const unsigned char *)toDecode, strlen(toDecode), &outputLength);
  startTimestamp = getFrameStartTime();
}

BufferLedSet::~BufferLedSet() 
{
    // Required because base64_decode uses os_malloc
    // https://github.com/espressif/esp-idf/blob/aaf12390eb14b95589acd98db5c268a2e56bb67e/components/wpa_supplicant/src/wpa2/utils/base64.c#L119
    free(BufferLedSet::data);
}

uint32_t BufferLedSet::getFrameStartTime() 
{
  uint32_t value = 0;

  // Big endian
  value |= BufferLedSet::data[0] << 24;
  value |= BufferLedSet::data[1] << 16;
  value |= BufferLedSet::data[2] << 8;
  value |= BufferLedSet::data[3];
  return value;
}

bool BufferLedSet::setLedsAtTime(CRGB* leds, uint8_t size, const uint32_t& time){
    int difference = time - startTimestamp;
    if(difference<0){
        // The startTimestamp is in the future. Just and do nothing and wait
        return true;
    } else if(difference>BufferLengthMicrosecounds){
        // The buffer has exired and no more new information.
        return false;
    } else {
        // Begins with frame 0
        uint currentFrame = (difference/SingleFrameLength) % BufferTotalFramesCount;
        uint bufferPosition = 4 + (currentFrame * size * 3);

        for(uint ledIndex = 0; ledIndex < size; ++ledIndex){
            leds[ledIndex].setRGB(
                data[bufferPosition],
                data[bufferPosition + 1],
                data[bufferPosition + 2]
            );

            bufferPosition += 3;
        }

        return true;
    }
}

