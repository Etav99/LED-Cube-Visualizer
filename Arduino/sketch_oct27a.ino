#include <SPI.h>
#define BTN 8
#define MAX_INPUT 64
const byte BufferSize = 64;
char Buffer[BufferSize + 1];

uint8_t cube[8][8];

void setup() {
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  pinMode(BTN, INPUT_PULLUP);
  Serial.begin(115200);
}


void loop() {
  if (Serial.available() > 0) {
    while (Serial.available() > 0) {
      Serial.readBytes(Buffer, BufferSize);
    }
    for(uint8_t i = 0; i < 64; i++){
      cube[i/8][i%8] = Buffer[i];
    }
    memset(Buffer, 0, sizeof(Buffer));
  }

    renderCube();
}

void renderCube() {
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(SS, LOW);
    SPI.transfer(0x01 << i);
    for (uint8_t j = 0; j < 8; j++) {
      SPI.transfer(cube[7-i][7-j]);
    }
    digitalWrite(SS, HIGH);
  }
}

void clearCube() {
  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      cube[i][j] = 0;
    }
  }
}
