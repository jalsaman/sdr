// Feather M0 RFM9x TX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (transmitter)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_RX

#include <SPI.h>
#include <RH_RF95.h>

// Wiring for Feather M0
#define RFM95_CS  8
#define RFM95_RST 4
#define RFM95_INT 3

// Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
#define RFM95_FREQ    434.0
#define RFM95_TXPOWER 5
#define RFM95_MOD     GFSK_Rb250Fd250

// Message
#define msg "123456789012345678901234567890123456789"

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() 
{
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // sending will start only when serial is connected.
  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }

  delay(100);

  Serial.println("Feather M0 RFM9x TX Test!");

  // ******************** manual reset radio ********************
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // ******************** initilize radio ********************
  while (!rf95.init()) {
    Serial.println("RFM9x radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("RFM9x radio init OK!");

  // ******************** set parameters ********************
  if (!rf95.setFrequency(RFM95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RFM95_FREQ);

  rf95.setTxPower(RFM95_TXPOWER, false);
}


void loop()
{
  
  char radiopacket[sizeof(msg)] = msg;
    
  Serial.print("Sending: "); 
  Serial.println(radiopacket);
  rf95.send((uint8_t *)radiopacket, sizeof(msg));

  Serial.println("Waiting for packet to complete..."); 
  rf95.waitPacketSent();

}
