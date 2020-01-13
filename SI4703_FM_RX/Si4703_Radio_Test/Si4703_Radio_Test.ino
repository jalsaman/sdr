/*  
*   Muthanna Alwahash 2020
*   license: freeware
*/

//-------------------------------------------------------------------------------------------------------------
#include <SparkFunSi4703.h>
#include <Wire.h>

int RST       = 4;
int SDIO      = A4;
int SCLK      = A5;
int STC       = 3; //???
//int GPIO1     = ?
//int GPIO2     = ?

Si4703_Breakout radio(RST, SDIO, SCLK, STC);

int channel = 876;
int volume  = 15;
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
  
// Power On Radio and set initial channel/volume
  radio.powerOn();
  radio.setChannel(channel);
  radio.setVolume(volume);
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
   Serial.print("\nChannel:"); Serial.print(channel); 
   Serial.print(" Volume:"); Serial.println(volume); 
}
