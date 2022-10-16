#include <I2CSlaveMode.h>
#include <Arduino.h>
#include <EEPROM.h>

I2CSlaveMode agent = I2CSlaveMode(0x33);

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial.println("Debug I2CSlaveMode");
  Serial.print("I2C Address: 0x");
  Serial.println(agent.getAddress(), HEX);
}


uint8_t const line_len = 16;
byte line_00[line_len] = {0x06, 0x47, 0x00, 0xFF, 0x0A, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
byte line_10[line_len] = {0x05, 0x64, 0x09, 0x60, 0x05, 0xC0, 0x08, 0xD0, 0x09, 0x60, 0x01, 0x32, 0x00, 0x00, 0x00, 0x01};
byte line_20[line_len] = {0x05, 0x64, 0x09, 0x60, 0x05, 0xC0, 0x08, 0xD0, 0x09, 0x60, 0x01, 0x32, 0x00, 0x00, 0x00, 0x01};
byte line_30[line_len] = {0x05, 0x64, 0x09, 0x60, 0x05, 0xC0, 0x08, 0xD0, 0x09, 0x60, 0x01, 0x32, 0x00, 0x00, 0x00, 0x01};
byte line_40[line_len] = {0x05, 0x64, 0x09, 0x60, 0x05, 0x64, 0x09, 0x60, 0x09, 0x60, 0x01, 0x32, 0x00, 0x00, 0x00, 0x01};
byte line_50[line_len] = {0x05, 0x64, 0x09, 0x60, 0x05, 0xC0, 0x07, 0xD0, 0x09, 0x60, 0x01, 0x32, 0x00, 0x00, 0x00, 0x01};
byte line_60[line_len] = {0x02, 0x80, 0x09, 0x60, 0x05, 0xC0, 0x05, 0xC0, 0x05, 0xC0, 0x01, 0x32, 0x00, 0x00, 0x00, 0x00};

void write_program() {
  EEPROM.put(0x00, line_00); 
  EEPROM.put(0x10, line_10); 
  EEPROM.put(0x20, line_20); 
  EEPROM.put(0x30, line_30); 
  EEPROM.put(0x40, line_40); 
  EEPROM.put(0x50, line_50); 
  EEPROM.put(0x60, line_60); 
}

/**
 * d: a pointer to an array of uint8_t
 * oset: the offset to read from eeprom
 */
void load(uint8_t* d, uint8_t oset) {
  for (uint8_t i = 0; i <= 0x0F; i++) {
    d[i] = agent.getRegister(oset+i);
  }  
}

void dump()
{
  Serial.println("\t 0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f");   
  uint8_t reg[16] = {0};
  char line[84] = {0};
  for (uint8_t oset = 0x00; oset <= 0x0F; oset++) {
    load(reg, oset*0x10);
    sprintf(line, "%02X      %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X  %02X", (oset*0x10), reg[0], reg[1], reg[2], reg[3], reg[4], reg[5], reg[6], reg[7], reg[8], reg[9], reg[10], reg[11], reg[12], reg[13], reg[14], reg[15]);
    Serial.println(line);  
  }

  Serial.println();
}

unsigned long eeprom_crc(void) {
  //from https://docs.arduino.cc/learn/programming/eeprom-guide
  const unsigned long crc_table[16] = {

    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,

    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,

    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,

    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c

  };

  unsigned long crc = ~0L;

  for (int index = 0 ; index < EEPROM.length()  ; ++index) {

    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);

    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);

    crc = ~crc;

  }

  return crc;
}

void loop() {
  // if there's any serial available, read it:
  while (Serial.available() > 0) {

    // look for the next valid integer in the incoming serial stream:
    int cmd = Serial.parseInt(SKIP_WHITESPACE);
    if (cmd == 1) {
      dump();
    } else if (cmd == 0) {
      Serial.flush();
    } else if (cmd == 2) {
      Serial.print("CRC32 of EEPROM data: 0x");
      Serial.println(eeprom_crc(), HEX);
    } else if (cmd == 3) {
      write_program();
      Serial.println("Wrote program to EEPROM");
    }
  }
}
