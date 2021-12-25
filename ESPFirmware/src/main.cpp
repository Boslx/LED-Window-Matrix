#include "painlessMesh.h"
#include <iostream>
#include <cstring>
#include <FastLED.h>
#include "BufferLedSet.h"
#include <CircularBuffer.h>
extern "C"
{
#include "crypto/base64.h"
}

using namespace std;

// Mesh
#define MESH_PREFIX "SuperHotspot 2"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

// LEDs
#define NUM_LEDS 6
#define BUFFER_FRAMES 25
#define DATA_PIN 13
#define BRIGTHNESS 64 // On startup

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;
CRGB leds[NUM_LEDS];

CircularBuffer<BufferLedSet *, 4> frameBuffers;

void setLEDs()
{
  while (!(frameBuffers.isEmpty() || frameBuffers.first()->setLedsAtTime(leds, NUM_LEDS, mesh.getNodeTime())))
  {
    delete frameBuffers.shift();
  }

  FastLED.show();
}

Task taskSetLEDs(TASK_MILLISECOND * 30, TASK_FOREVER, &setLEDs);

// User stub
void sendMessage(); // Prototype so PlatformIO doesn't complain

// Needed for painless library
void receivedCallback(uint32_t from, String msg)
{
  frameBuffers.push(new BufferLedSet(msg));
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void initSequenze()
{
  vector<uint32_t> colors{
      0xFF0000, // Red
      0x00FF00, // Green
      0x0000FF, // Blue
      0x6667AB, // Color of the year 2022
      0x000000
  };

  for (auto color : colors)
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i].setColorCode(color);
      FastLED.show();
    }
    delay(500);
  }
}

void setup()
{
  Serial.begin(115200);

  FastLED.addLeds<WS2812, DATA_PIN, BRG>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGTHNESS);

  initSequenze();

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSetLEDs);
  taskSetLEDs.enable();
}

void loop()
{
  // it will run the user scheduler as well
  mesh.update();
}