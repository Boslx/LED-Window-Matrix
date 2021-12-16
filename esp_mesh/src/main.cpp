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
#define   BRIGTHNESS      128

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;
CRGB leds[NUM_LEDS];

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());

  const char* toDecode = msg.c_str();

  size_t outputLength;
  unsigned char* decoded = base64_decode((const unsigned char*) toDecode, strlen(toDecode), &outputLength);
 
  Serial.print("Length of decoded message: ");
  Serial.println(outputLength);
 
  Serial.printf("%.*s", outputLength, decoded);

  if(outputLength == 1){
    FastLED.setBrightness(decoded[0]);
    mesh.sendSingle(from, "Set brightness to " + (uint8_t) decoded[0]);
    return;
  }

  if(outputLength != NUM_LEDS*3){
    mesh.sendSingle(from, "Invalid message length, expected 6, got " + outputLength);
    return;
  }

  uint8_t index = 0;
  for(int i = 0; i < outputLength; i += 3) {
    leds[index].setRGB(decoded[i], decoded[i + 1], decoded[i + 2]);
    ++index;
  }
  
  FastLED.show();
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGTHNESS);
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
}