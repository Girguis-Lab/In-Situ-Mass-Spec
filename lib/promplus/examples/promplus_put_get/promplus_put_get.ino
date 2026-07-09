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
  Serial.println();
}

void singleWrite() {
  Serial.println("Lets first look at a single put action.");
  Serial.println("A single action results in 3 duplicates.");
  uint16_t data = 1234;
  PROM.put(0x00, data);
  hexDump(2);
}

void multipleWrites() {
  Serial.println("Next look at multiple puts.");
  Serial.println("Duplicate values have been stored for each.");
  uint16_t
  value1 = 1234,
  value2 = 2345,
  value3 = 3456;
  PROM.put(0x00, value1);
  PROM.put(0x01, value2);
  PROM.put(0x02, value3);
  hexDump(2);
}

void readValues() {
  Serial.println("Getting each value gives the expected results.");
  uint16_t
  value1,
  value2,
  value3;
  PROM.get(0, value1);
  PROM.get(1, value2);
  PROM.get(2, value3);
  Serial.print("value1: ");
  Serial.println(value1, DEC);
  Serial.print("value2: ");
  Serial.println(value2, DEC);
  Serial.print("value3: ");
  Serial.println(value3, DEC);
  Serial.println();
}

void corruptTheData() {
  Serial.println("Lets manually corrupt the data.");
  EEPROM[0] = 0x00;
  EEPROM[3] = 0x00;
  EEPROM[6] = 0x00;
  EEPROM[9] = 0x00;
  EEPROM[12] = 0x00;
  EEPROM[15] = 0x00;
  hexDump(2);
}

void setBaseAddress() {
  eraseEEPROM();
  Serial.println("Lets move the starting location where we write the uint16_t data.");
  uint16_t
  value1 = 1234,
  value2 = 2345,
  value3 = 3456;
  PROM.put(0x00, value1, 0x01);
  PROM.put(0x01, value2, 0x01);
  PROM.put(0x02, value3, 0x01);
  hexDump(2);
}

void readFromMovedAddress() {
  Serial.println("We can iterate through just like a uint16_t array.");
  uint16_t values[3];
  uint8_t arraySize = sizeof(values)/sizeof(values[0]); 
  for (int i = 0; i < arraySize; i++) {
    PROM.get(i, values[i], 0x01);
  }
  Serial.print("values = {"); 
  for (int i = 0; i < arraySize; i++) {
    Serial.print(values[i]); 

    if (i != arraySize - 1) {
      Serial.print(", "); 
    }
  }
  Serial.print("}");
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
  setBaseAddress();
  readFromMovedAddress();
}

void loop() {
}
