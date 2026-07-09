#include <PROMPLUS.h>

// make sure there is nothing in EEPROM before example executes
void eraseEEPROM() {
  for (int i = 0; i < 100; i++) {
    PROM[i] = 0xFF;
  }
}

// print a hex dump of the EEPROM memory
#define bytesPerLine 16
void hexDump(int lines) {
  char buff[100];
  for (int i = 0; i < lines; i++) {
    sprintf(buff, "0x0%02X0: ", i);
    Serial.print(buff);
    for (int j = 0; j < bytesPerLine; j++) {
      sprintf(buff, "%02X", EEPROM.read((i * bytesPerLine) + j));
      Serial.print(buff);
      if (j != bytesPerLine - 1) {
        Serial.print(",");
      }
    }
    Serial.print("\n");
  }
}

void singleWrite() {
  Serial.println("Lets first look at a single write action.");
  Serial.println("A single action results in 3 duplicates.");
  PROM.write(0x00, 0x55);
  hexDump(2);
  Serial.println();
}

void multipleWrites() {
  Serial.println("Next look at multiple writes.");
  Serial.println("Duplicate values have been stored for each.");
  PROM.write(0x00, 0x55);
  PROM.write(0x01, 0x77);
  PROM.write(0x02, 0x99);
  hexDump(2);
  Serial.println();
}

void readValues() {
  Serial.println("Reading each value gives the expected results.");
  uint8_t 
    value1 = PROM.read(0),
    value2 = PROM.read(1),
    value3 = PROM.read(2);
  Serial.print("value1: 0x");
  Serial.println(value1, HEX);
  Serial.print("value2: 0x");
  Serial.println(value2, HEX);
  Serial.print("value3: 0x");
  Serial.println(value3, HEX);
  Serial.println();
}

void corruptTheData() {
  Serial.println("Lets manually corrupt the data.");
  EEPROM[0] = 0x00;
  EEPROM[3] = 0x00;
  EEPROM[6] = 0x00;
  hexDump(2);
  Serial.println();
}

void setup() {
  Serial.begin(9600);     // print example outputs to monitor
  eraseEEPROM();          // Make sure eeprom is cleared before example starts
  singleWrite();
  multipleWrites();
  readValues();
  corruptTheData();
  readValues();
}

void loop() {
}
