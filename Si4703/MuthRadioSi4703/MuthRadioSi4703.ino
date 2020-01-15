/*  
 *   Based on FabFm Firmware v11 from Aaron Weiss, SparkFun 2012
 *   Muthanna Alwahash 2020
 *   license: freeware
 *   
 *   Connections:
 *   -----------------------
 *   Si4703  → 3.3V Pro Mini
 *   -----------------------
 *   GND     → GND
 *   3.3V    → VCC
 *   SDIO    → A4
 *   SCLK    → A5
 *   SEN     → NC * The breakout board has SEN pulled high
 *   RST     → D4
 *   GPI01   → NC * Not used
 *   GPI02   → D3
 *   
 *   Description:
 *   -----------------------
 *   This is example code for the FamFM radio kit. The hardware mimics an Arduino Pro (3.3V, 8MHz)
 *   and has an FTDI header to load Arduino sketches. The hardware includes the Si4703 FM radio 
 *   module to find the stations along with an op-amp and speaker for the audio. The unit has a
 *   potentiometer with a switch for power and volume control, and a rotary encoder to tune the
 *   stations. The unit displays the radio station over a serial terminal at 115200 and also saves
 *   the station on subsequent power ups.
 *   
 *
 *   Operation:
 *   -----------------------
 *   -Power switch and volume are on the left, tuner is on the right.
 *   -The board must be powered with a switch mode 9V DC wall wart.
 *   -Each click will increase/decrease station by 0.1MHz
 *   -The LED blinks for every step
 *   
 */

//-------------------------------------------------------------------------------------------------------------
#include <SparkFunSi4703.h>
#include <Wire.h>
#include <EEPROM.h>

//-------------------------------------------------------------------------------------------------------------
// Global Constants (defines): these quantities don't change

const int RST         = 4;  // radio reset pin
const int SDIO        = A4; // radio data pin
const int SCLK        = A5; // radio clock pin
const int STC         = 6;  // radio interrupt pin

const int encoderPin1 = 2;  // encoder pin 1
const int encoderPin2 = 3;  // encoder pin 2

const int LED         = 5;  // LED pin

int       channel;
int       freqMin     =875; // FM Frequency Range from 87.5
int       freqMax     =1079;// FM Frequency Range to 107.9 MHz
int       fav_a       =876; // Favourate Channel a
int       fav_b       =1048;// Favourate Channel b

int       volume      = 15; // Initial software volume is highest
char      rdsBuffer[10];    // Buffer to store RDS/RBDS text

//Volatile variables are needed if used within interrupts
volatile int      lastEncoded         = 0;
volatile long     encoderValue        = 0;
volatile int      goodEncoderValue;
volatile boolean  updateStation       = false;
volatile boolean  stationDirection;

const boolean UP    = true;
const boolean DOWN  = false;

//-------------------------------------------------------------------------------------------------------------
// create radio instance
//-------------------------------------------------------------------------------------------------------------
Si4703_Breakout radio(RST, SDIO, SCLK, STC);

//-------------------------------------------------------------------------------------------------------------
// Arduino initial Setup
//-------------------------------------------------------------------------------------------------------------
void setup()
{
  // start serial
  Serial.begin(115200);
  
  // both pins on the rotary encoder are inputs and pulled high
  pinMode(encoderPin1, INPUT_PULLUP);
  pinMode(encoderPin2, INPUT_PULLUP);
  
  // LED pin is output
  pinMode(LED, OUTPUT);
  
  // load saved station into channel variable
  read_channel_from_EEPROM();
  
  // start radio module
  radio.powerOn();            // turns the module on
  radio.setChannel(channel);  // loads saved channel
  radio.setVolume(volume);    // volume setting
  digitalWrite(LED, HIGH);    // turn LED ON
  
  // print welcome screen with station number
  
  Serial.print("\nWelcome...\nFM Radio Tuned to: ");
  printChannelInfo();

  Serial.println("\nWelcom....");
  Serial.println("-----------");  
  Serial.println("a b     Favourite stations");
  Serial.println("+ -     Volume (max 15)");
  Serial.println("u d     Seek up / down");
  Serial.println("r       Listen for RDS Data (15 sec timeout)");
  Serial.println("Send a command letter.");


  //call updateEncoder() when any high/low changed seen on interrupt 0 (pin 2), or interrupt 1 (pin 3) 
  attachInterrupt(0, updateEncoder, CHANGE); 
  attachInterrupt(1, updateEncoder, CHANGE);

}

//-------------------------------------------------------------------------------------------------------------
// Arduino main loop
//-------------------------------------------------------------------------------------------------------------
void loop()
{
  
  // Interrupt tells us to update the station when updateStation=True
  if(updateStation)
  {
    digitalWrite(LED, LOW);

    if(stationDirection == UP)
    {
      //Serial.print("Up ");
      channel += 1;         //Channels change by 1 (ex: 88.0 to 88.1)
    }
    else if(stationDirection == DOWN)
    {
      //Serial.print("Down ");
      channel -= 1;         //Channels change by 1 (ex: 88.4 to 88.3)
    }
    
    // Catch wrap conditions
    if(channel > freqMax) channel = freqMin;
    if(channel < freqMin) channel = freqMax;
    
    radio.setChannel(channel);  // Goto the new channel
    save_channel();             // Save channel to EEPROM
    printChannelInfo();         // Print channel info
    updateStation = false;      //Clear flag
    digitalWrite(LED, HIGH);    // When done turn LED On
  }


  // Radio control from serial interface
  if (Serial.available())
  {
    char ch = Serial.read();
    if (ch == 'u') 
    {
      channel = radio.seekUp();
      save_channel();             // Save channel to EEPROM
      printChannelInfo();
    } 
    else if (ch == 'd') 
    {
      channel = radio.seekDown();
      save_channel();             // Save channel to EEPROM
      printChannelInfo();
    } 
    else if (ch == '+') 
    {
      volume ++;
      if (volume == 16) volume = 15;
      radio.setVolume(volume);
      printChannelInfo();
    } 
    else if (ch == '-') 
    {
      volume --;
      if (volume < 0) volume = 0;
      radio.setVolume(volume);
      printChannelInfo();
    } 
    else if (ch == 'a')
    {
      channel = fav_a; 
      radio.setChannel(channel);
      save_channel();             // Save channel to EEPROM
      printChannelInfo();
    }
    else if (ch == 'b')
    {
      channel = fav_b;
      radio.setChannel(channel);
      save_channel();             // Save channel to EEPROM
      printChannelInfo();
    }
    else if (ch == 'r')
    {
      Serial.println("RDS listening");
      radio.readRDS(rdsBuffer, 15000);
      Serial.print("RDS heard:");
      Serial.println(rdsBuffer);      
    }
  }

  // You can put any additional code here, but keep in mind, 
  // the encoder interrupt is running in the background
}

//-------------------------------------------------------------------------------------------------------------
// Splits the channel variable into two separate bytes, 
// then loads each value into one 8-bit EEPROM location
//-------------------------------------------------------------------------------------------------------------
void save_channel()
{
  int msb = channel >> 8; // move channel over 8 spots to grab MSB
  int lsb = channel & 0x00FF; // clear the MSB, leaving only the LSB
  EEPROM.write(1, msb); // write each byte to a single 8-bit position
  EEPROM.write(2, lsb);
}

//-------------------------------------------------------------------------------------------------------------
// Reads the two saved 8-bit values, that defines the 
// channel, and concatenates them together to form a 16-bit number
//-------------------------------------------------------------------------------------------------------------
void read_channel_from_EEPROM()
{
  int msb = EEPROM.read(1); // load the msb into one 8-bit register
  int lsb = EEPROM.read(2); // load the lsb into one 8-bit register
  msb = msb << 8; // shift the msb over 8 bits
  channel = msb|lsb; // concatenate the lsb and msb
}

//-------------------------------------------------------------------------------------------------------------
// This is the interrupt handler that reads the encoder. It set the updateStation flag when a new indent is found 
// Note: The encoder used on the FabFM has a mechanical indent every four counts so most of the time, when you 
// advance 1 click there are four interrupts but there is no guarantee the user will land neaty on an indent, 
// so this code cleans up the read.
// Modified from the bildr article: http://bildr.org/2012/08/rotary-encoder-arduino/
//-------------------------------------------------------------------------------------------------------------
void updateEncoder()
{
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
  {
    stationDirection = DOWN; //Counter clock wise
    encoderValue--;
  }
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
  {
    stationDirection = UP; //Clock wise
    encoderValue++;
  }

  lastEncoded = encoded; //store this value for next time

  //Wait until we are more than 3 ticks from previous used value
  if(abs(goodEncoderValue - encoderValue) > 3)
  {
    //The user can sometimes miss an indent by a count or two
    //This logic tries to correct for that
    //Remember, interrupts are happening in the background so encoderValue can change 
    //throughout this code

    if(encoderValue % 4 == 0) //Then we are on a good indent
    {
      goodEncoderValue = encoderValue; //Remember this good setting
    }
    else if( abs(goodEncoderValue - encoderValue) == 3) //The encoder is one short
    {
      if(encoderValue < 0) goodEncoderValue = encoderValue - 1; //Remember this good setting
      if(encoderValue > 0) goodEncoderValue = encoderValue + 1; //Remember this good setting
    }
    else if( abs(goodEncoderValue - encoderValue) == 5) //The encoder is one too long
    {
      if(encoderValue < 0) goodEncoderValue = encoderValue + 1; //Remember this good setting
      if(encoderValue > 0) goodEncoderValue = encoderValue - 1; //Remember this good setting
    }

    updateStation = true;
  }
}

//-------------------------------------------------------------------------------------------------------------
// Display current channel and volume info.
//-------------------------------------------------------------------------------------------------------------
void printChannelInfo()
{
   Serial.print("\nCh:");
   Serial.print(float(channel)/10,1);
   Serial.print(" MHz sVOL:");
   Serial.println(volume);
}
