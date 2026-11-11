#include <Arduino.h>
#include "FS.h"
#include "LittleFS.h"

//Pins
#define LATCH_PIN  5
#define CLOCK_PIN  18
#define DATA_PIN  19
const int layerPins[4] = {13, 12, 14, 27};
const int NUM_LAYERS = 4;
const int NUM_FRAMES = 4;


File     animationFile;
uint32_t currentFrame = 0;
uint32_t currentLayer = 0;
uint32_t frameDurationMs = 100;
uint32_t lastFrameTime  = 0;
uint8_t  frameBuffer[64];


void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Booting...");
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(DATA_PIN, OUTPUT);
    for(int i = 0; i < 4; i++) {
        pinMode(layerPins[i], OUTPUT);
        digitalWrite(layerPins[i], LOW);
    }
    if(!LittleFS.begin(false, "/littlefs", 10, "spiffs")){
      Serial.println("LittleFS Mount Failed");
      return;
    }
    animationFile = LittleFS.open("/animations.bin", "r");
    if(!animationFile) {
      Serial.println("Failed to open file");
      return;
    }
    size_t n = animationFile.read(frameBuffer, sizeof(frameBuffer));
    Serial.printf("read %u bytes\n", (unsigned)n);
}   

void shiftOut8(uint8_t value) {
    for(int i = 0; i < 8; i++) {
        digitalWrite(DATA_PIN, (value >> i) & 1);
        digitalWrite(CLOCK_PIN, HIGH);
        digitalWrite(CLOCK_PIN, LOW);
    }
};

void shiftOut16(uint16_t value) {
    shiftOut8(value & 0xFF);
    shiftOut8((value >> 8) & 0xFF);
};

void latch() {
    digitalWrite(LATCH_PIN, HIGH);
    digitalWrite(LATCH_PIN, LOW);
};

void setLayers(uint8_t val) {
    for(int i = 0; i < NUM_LAYERS; i++){
        digitalWrite(layerPins[i],val);
    }
}

void loop() {
    setLayers(HIGH);
    shiftOut16(0x0000);
    latch();
    setLayers(LOW);
    
    uint16_t row = (uint16_t)frameBuffer[currentFrame * 2] | ((uint16_t)frameBuffer[currentFrame * 2 + 1] << 8);
    shiftOut16(row);
    digitalWrite(layerPins[currentLayer], HIGH);
    latch();
    digitalWrite(layerPins[currentLayer], LOW);


    currentLayer = (currentLayer + 1) % NUM_LAYERS;
    if (currentLayer == 0) {
        currentFrame = (currentFrame + 1) % NUM_FRAMES;
    }
    if (currentLayer == 0) {
        Serial.printf("frame %u row=0x%04X\n", currentFrame, row);
    }   
}