/*  Based on FabFm Firmware v11 from Aaron Weiss, SparkFun 2012
*   Muthanna Alwahash 2020
*   license: freeware
*----------------------------
*
*/

//-------------------------------------------------------------------------------------------------------------
#include <SparkFunSi4703.h>
#include <Wire.h>

int resetPin  = 4;
int SDIO      = A4;
int SCLK      = A5;
int STC       = 3; //???

Si4703_Breakout radio(resetPin, SDIO, SCLK, STC);

int channel;
int volume;
char rdsBuffer[10];

//-------------------------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  Serial.println("\nWelcom....");
  Serial.println("-----------");  
  Serial.println("a b     Favourite stations");
  Serial.println("+ -     Volume (max 15)");
  Serial.println("u d     Seek up / down");
  Serial.println("r       Listen for RDS Data (15 sec timeout)");
  Serial.println("Send a command letter.");
  

  radio.powerOn();
  radio.setVolume(0);
}

//-------------------------------------------------------------------------------------------------------------
void loop()
{
  if (Serial.available())
  {
    char ch = Serial.read();
    if (ch == 'u') 
    {
      channel = radio.seekUp();
      displayInfo();
    } 
    else if (ch == 'd') 
    {
      channel = radio.seekDown();
      displayInfo();
    } 
    else if (ch == '+') 
    {
      volume ++;
      if (volume == 16) volume = 15;
      radio.setVolume(volume);
      displayInfo();
    } 
    else if (ch == '-') 
    {
      volume --;
      if (volume < 0) volume = 0;
      radio.setVolume(volume);
      displayInfo();
    } 
    else if (ch == 'a')
    {
      channel = 876; 
      radio.setChannel(channel);
      displayInfo();
    }
    else if (ch == 'b')
    {
      channel = 1076;
      radio.setChannel(channel);
      displayInfo();
    }
    else if (ch == 'r')
    {
      Serial.println("RDS listening");
      radio.readRDS(rdsBuffer, 15000);
      Serial.print("RDS heard:");
      Serial.println(rdsBuffer);      
    }
  }
}

//-------------------------------------------------------------------------------------------------------------
void displayInfo()
{
   Serial.print("Channel:"); Serial.print(channel); 
   Serial.print(" Volume:"); Serial.println(volume); 
}
