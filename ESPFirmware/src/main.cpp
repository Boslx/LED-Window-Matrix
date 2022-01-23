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
#define MESH_PREFIX "LED-Window-Matrix"
#define MESH_PASSWORD "Ea2eNoh2aixi"
#define MESH_PORT 5555
#define MESH_HIDDEN 0 // TODO: Set me to 1 in production

// LEDs
#define NUM_LEDS 6
#define DATA_PIN 21
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


void handleControllMessage(String& msg){
  // Inside the brackets, 20 is the capacity of the memory pool in bytes.
  // Don't forget to change this value to match your JSON document.
  // Use https://arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<20> doc;
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, msg);

  if(error){
    Serial.printf("Deserializing the controll message failed: %s\n", error.c_str());
    return;
  }

  // Set Brightness according to controll message
  FastLED.setBrightness(doc["brightness"]);
}

// Needed for painless library
void receivedCallback(uint32_t from, String msg)
{
  if (msg[0] == '{')
  {
    // A JSON
    handleControllMessage(msg);
  }
  else
  {
    // Probably Base64 encoded
    frameBuffers.push(new BufferLedSet(msg));
  }
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
      0x000000};

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

  FastLED.addLeds<WS2811, DATA_PIN, BGR>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setBrightness(BRIGTHNESS);

  initSequenze();

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 1U, MESH_HIDDEN);
  mesh.onReceive(&receivedCallback);
  //mesh.onNewConnection(&newConnectionCallback);
  //mesh.onChangedConnections(&changedConnectionCallback);
  //mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSetLEDs);
  taskSetLEDs.enable();
}

void loop()
{
  // it will run the user scheduler as well
  mesh.update();
}
