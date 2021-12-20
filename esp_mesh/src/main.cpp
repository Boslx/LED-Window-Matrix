#include "painlessMesh.h"
#include <iostream>
#include <cstring>
#include <FastLED.h>
extern "C" {
  #include "crypto/base64.h"
}

using namespace std;

// Mesh
#define   MESH_PREFIX     "SuperHotspot 2"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// LEDs
#define   NUM_LEDS        6
#define   DATA_PIN        2
#define   BRIGTHNESS      128 // On startup

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;
CRGB leds[NUM_LEDS];
String lastMessage = "";

void setLEDs() {
  if(lastMessage.isEmpty()){
    return;
  }

  const char* toDecode = lastMessage.c_str();

  size_t outputLength;
  unsigned char* decoded = base64_decode((const unsigned char*) toDecode, strlen(toDecode), &outputLength);

  if(outputLength == 1){
    FastLED.setBrightness(decoded[0]);
    //mesh.sendSingle(from, "Set brightness to " + (uint8_t) decoded[0]);
    return;
  }

  if(outputLength != NUM_LEDS*3){
    //mesh.sendSingle(from, "Invalid message length, expected 18, got " + outputLength);
    return;
  }

  uint8_t index = 0;
  for(int i = 0; i < outputLength; i += 3) {
    leds[index].setRGB(decoded[i], decoded[i + 1], decoded[i + 2]);
    ++index;
  }
  
  FastLED.show();
}

Task taskSetLEDs(TASK_SECOND * 1, TASK_FOREVER, &setLEDs);

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  // TODO here output
  lastMessage = msg;
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  //Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGTHNESS);

  // Show turn on animation
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::Fuchsia;
  }
  FastLED.show();
  delay(1000);
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGB::Black;
  }
  FastLED.show();

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSetLEDs);
  taskSetLEDs.enable();

}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
}