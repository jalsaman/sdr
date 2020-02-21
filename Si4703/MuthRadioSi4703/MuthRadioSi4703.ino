/*  
 *   Based on FabFm Firmware v11 from Aaron Weiss, SparkFun 2012
 *   Muthanna Alwahash 2020
 *   license: freeware
 *   
 *   Hardware used:
 *   -----------------------   
 *   1 x SparkFun Breadboard Power Supply Stick - 5V/3.3V (https://www.sparkfun.com/products/13032)
 *   2 x SparkFun Mono Audio Amp Breakout - TPA2005D1     (https://www.sparkfun.com/products/11044)
 *   1 x SparkFun FM Tuner Basic Breakout - Si4703        (https://www.sparkfun.com/products/11083)
 *   1 x Arduino Pro Mini 328 - 3.3V/8MHz                 (https://www.sparkfun.com/products/11114)
 *   -----------------------   
 *   
 *   Connections:
 *   -----------------------
 *   Si4703  → 3.3V A.Pro Mini
 *   -----------------------
 *   GND     → GND
 *   3.3V    → VCC
 *   SDIO    → A.Pro Mini A4
 *   SCLK    → A.Pro Mini A5
 *   SEN     → NC * The breakout board has SEN pulled high
 *   RST     → A.Pro Mini D4
 *   GPIO1   → LED2
 *   GPIO2   → A.Pro Mini D6
 *   
 *   -----------------------
 *   3.3V A.Pro Mini
 *   -----------------------
 *   GND     → GND
 *   VCC     → 3.3V
 *   A4      → Si4703 SDIO
 *   A5      → Si4703 SCLK
 *   D6      → Si4703 GPIO2 
 *   D2      → Rotary Encoder A
 *   D3      → Rotary Encoder B
 *   D4      → Si4703 RST
 *   D5      → LED1
 *      
 *   -----------------------
 *   Rotary Encoder
 *   -----------------------
 *   A  A.Pro Mini D2
 *   B  A.Pro Mini D3
 *   C  GND
 *   
 *   -----------------------
 *   LEDs
 *   -----------------------
 *   LED1 A      → A.Pro Mini D5 
 *   LED1 C      → GND
 *   LED2 A      → Si4703 GPIO1
 *   LED2 C      → GND
 *   
 *   Description:
 *   -----------------------
 *   This is example code can work for the FamFM radio kit and breadboard builds as per above connections. 
 *   The used controller is Arduino Pro (3.3V, 8MHz) and has an FTDI header to load Arduino sketches.
 *   The hardware includes the Si4703 FM radio module to find the stations along with audio amps. and speakers for the audio.
 *   The circuit has a rotary encoder to tune the stations. 
 *   The circuit displays the radio station over a serial terminal at 115200 and can be controlled from terminal.
 *   Also it saves the station settings on EEPROM and loads it in subsequent power ups.
 *   
 *
 *   Operation:
 *   -----------------------
 *   -The board must be powered with a switch mode 9V DC wall wart.
 *   -Each click will increase/decrease station by 0.1MHz
 *   -The LED blinks for every step
 *   
 */

//-------------------------------------------------------------------------------------------------------------
// Required Libraries
//-------------------------------------------------------------------------------------------------------------
#include <Si4703.h> // library to control Silicon Labs' Si4703 FM Radio Receiver.
#include <Wire.h>   // Used for I2C interface.
#include <EEPROM.h> // To save configuration parameters such as channel and volume.


//-------------------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------------------
// EEPROM Usage Map
#define eeprom_chn_msb  1
#define eeprom_chn_lsb  2
#define eeprom_vol      3
#define eeprom_fav_1    4
#define eeprom_fav_2    5

// FM Frequency Range
#define freqMin         875   // FM Frequency Range from 87.5
#define freqMax         1079  // FM Frequency Range to 107.9 MHz

//-------------------------------------------------------------------------------------------------------------
// Global Constants (defines): these quantities don't change
//-------------------------------------------------------------------------------------------------------------
const int RST         = 4;  // radio reset pin
const int SDIO        = A4; // radio data pin
const int SCLK        = A5; // radio clock pin
const int STC         = 6;  // radio interrupt pin
const int encoderPin1 = 2;  // encoder pin 1
const int encoderPin2 = 3;  // encoder pin 2
const int LED1        = 5;  // LED1 pin
const boolean UP      = true;
const boolean DOWN    = false;

//-------------------------------------------------------------------------------------------------------------
// Varliables
//-------------------------------------------------------------------------------------------------------------

// Settings
int       channel;              // channel value
int       volume;               // volume value 1-15

// Favourate Channels 0..9
int       fav_0         =876;   
int       fav_1         =882;   
int       fav_2         =914;  
int       fav_3         =922;   
int       fav_4         =939;  
int       fav_5         =944;   
int       fav_6         =950;  
int       fav_7         =976;   
int       fav_8         =1048;  
int       fav_9         =1074;   


char      rdsBuffer[10];        // Buffer to store RDS/RBDS text

//-------------------------------------------------------------------------------------------------------------
// Volatile variables are needed if used within interrupts
//-------------------------------------------------------------------------------------------------------------
volatile int      lastEncoded         = 0;
volatile long     encoderValue        = 0;
volatile int      goodEncoderValue;
volatile boolean  updateStation       = false;
volatile boolean  stationDirection;

//-------------------------------------------------------------------------------------------------------------
// create radio instance
//-------------------------------------------------------------------------------------------------------------
Si4703 radio(RST, SDIO, SCLK, STC);

//-------------------------------------------------------------------------------------------------------------
// Arduino initial Setup
//-------------------------------------------------------------------------------------------------------------
void setup()
{
 
  Serial.begin(115200);        // start serial

  pinMode(LED1, OUTPUT);       // LED1 pin is output
  digitalWrite(LED1, LOW);     // turn LED1 OFF

  read_EEPROM();              // load saved settings
  radio.powerOn();            // turns the module on
  radio.setChannel(channel);  // loads saved channel
  radio.setVolume(volume);    // volume setting

  // Enable rotary encoder
  pinMode(encoderPin1, INPUT_PULLUP);         // pin is input and pulled high
  pinMode(encoderPin2, INPUT_PULLUP);         // pin is input and pulled high
  attachInterrupt(0, updateEncoder, CHANGE);  // call updateEncoder() when any high/low changed seen on interrupt 0 (pin 2)
  attachInterrupt(1, updateEncoder, CHANGE);  // call updateEncoder() when any high/low changed seen on interrupt 1 (pin 3)

  // Show ready status
  digitalWrite(LED1, HIGH);           // turn LED1 ON
  radio.writeGPIO(GPIO1, GPIO_High);  // turn LED2 ON

  // Display info
   printWelcome();
  printCurrentSettings();
  printHelp();
  
}

//-------------------------------------------------------------------------------------------------------------
// Arduino main loop
//-------------------------------------------------------------------------------------------------------------
void loop()
{
    
  if (updateStation)      updateStationFreq();  // Interrupt tells us to update the station when updateStation=True
  if (Serial.available()) processCommand();     // Radio control from serial interface
  
  // You can put any additional code here, but keep in mind, the encoder interrupt is running in the background
  
}

//-------------------------------------------------------------------------------------------------------------
// Write current settings to EEPROM
//-------------------------------------------------------------------------------------------------------------
void write_EEPROM()
{
  // Save current channel value
  int msb = channel >> 8;             // move channel over 8 spots to grab MSB
  int lsb = channel & 0x00FF;         // clear the MSB, leaving only the LSB
  EEPROM.write(eeprom_chn_msb, msb);  // write each byte to a single 8-bit position
  EEPROM.write(eeprom_chn_lsb, lsb);

  // Save volume
  EEPROM.write(eeprom_vol, volume);
  
}

//-------------------------------------------------------------------------------------------------------------
// Read settings from EEPROM
//-------------------------------------------------------------------------------------------------------------
void read_EEPROM()
{
  // Read channel value
  int MSB = EEPROM.read(eeprom_chn_msb); // load the msb into one 8-bit register
  int LSB = EEPROM.read(eeprom_chn_lsb); // load the lsb into one 8-bit register
  channel = (MSB << 8)|LSB;              // concatenate the lsb and msb
  
  // Read Volume
  volume = EEPROM.read(eeprom_vol);
}

//-------------------------------------------------------------------------------------------------------------
// Interrupt handler that reads the encoder. It set the updateStation flag when a new indent is found 
//-------------------------------------------------------------------------------------------------------------
void updateEncoder()
{
  int MSB = digitalRead(encoderPin1);       //MSB = most significant bit
  int LSB = digitalRead(encoderPin2);       //LSB = least significant bit

  int encoded = (MSB << 1) |LSB;            //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded;  //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
  {
    stationDirection = DOWN;
    encoderValue--;
  }
  
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
  {
    stationDirection = UP;
    encoderValue++;
  }

  lastEncoded = encoded; //store this value for next time

  updateStation = true;
 
}

//-------------------------------------------------------------------------------------------------------------
// Update Station Freq
//-------------------------------------------------------------------------------------------------------------
void updateStationFreq()
  {
    digitalWrite(LED1, LOW);           // turn LED1 OFF
    radio.writeGPIO(GPIO1, GPIO_Low);  // turn LED2 OFF

    if(stationDirection == UP)
    {
      channel += 1;         //Channels change by 1 (ex: 88.0 to 88.1)
    }
    else if(stationDirection == DOWN)
    {
      channel -= 1;         //Channels change by 1 (ex: 88.4 to 88.3)
    }
    
    // Catch wrap conditions
    if(channel > freqMax) channel = freqMin;
    if(channel < freqMin) channel = freqMax;
    
    radio.setChannel(channel);  // Goto the new channel
    write_EEPROM();             // Save channel to EEPROM
    printCurrentSettings();     // Print channel info
    updateStation = false;      //Clear flag

    digitalWrite(LED1, HIGH);           // When done turn LED1 On
    radio.writeGPIO(GPIO1, GPIO_High);  // turn LED2 ON
  }

//-------------------------------------------------------------------------------------------------------------
// Display Welcome Message.
//-------------------------------------------------------------------------------------------------------------
void printWelcome()
{
  Serial.println("\nWelcome...");
}

//-------------------------------------------------------------------------------------------------------------
// Display current settings such as channel and volume.
//-------------------------------------------------------------------------------------------------------------
void printCurrentSettings()
{
   Serial.print("Ch:");
   Serial.print(float(channel)/10,1);
   Serial.print(" MHz sVOL:");
   Serial.println(volume);
   
}

//-------------------------------------------------------------------------------------------------------------
// Display Help on commands.
//-------------------------------------------------------------------------------------------------------------
void printHelp()
{
  Serial.println("0..9    Favourite stations");
  Serial.println("+ -     Volume (max 15)");
  Serial.println("u d     Seek up / down");
  Serial.println("r       Listen for RDS Data (15 sec timeout)");
  Serial.println("i       Prints current settings");
  Serial.println("f       Prints Favourite stations list");
  Serial.println("h       Prints this help");
  Serial.println("Select a command:");
}

//-------------------------------------------------------------------------------------------------------------
// Prints Favourite Stations List
//-------------------------------------------------------------------------------------------------------------
void printFavouriteList()
{
  Serial.println("List of Favourite Stations");
  
  Serial.print("0 - ");
  Serial.print(float(fav_0)/10,1);
  Serial.println(" MHz");

  Serial.print("1 - ");
  Serial.print(float(fav_1)/10,1);
  Serial.println(" MHz");

  Serial.print("2 - ");
  Serial.print(float(fav_2)/10,1);
  Serial.println(" MHz");

  Serial.print("3 - ");
  Serial.print(float(fav_3)/10,1);
  Serial.println(" MHz");


  Serial.print("4 - ");
  Serial.print(float(fav_4)/10,1);
  Serial.println(" MHz");

  Serial.print("5 - ");
  Serial.print(float(fav_5)/10,1);
  Serial.println(" MHz");

  Serial.print("6 - ");
  Serial.print(float(fav_6)/10,1);
  Serial.println(" MHz");

  Serial.print("7 - ");
  Serial.print(float(fav_7)/10,1);
  Serial.println(" MHz");

  Serial.print("8 - ");
  Serial.print(float(fav_8)/10,1);
  Serial.println(" MHz");

  Serial.print("9 - ");
  Serial.print(float(fav_9)/10,1);
  Serial.println(" MHz");

}
//-------------------------------------------------------------------------------------------------------------
// Process a command from serial terminal
//-------------------------------------------------------------------------------------------------------------
void processCommand()
{
  
  char ch = Serial.read();
  
  if (ch == 'u') 
  {
    channel = radio.seekUp();
    write_EEPROM();             // Save channel to EEPROM
    printCurrentSettings();
  } 
  else if (ch == 'd') 
  {
    channel = radio.seekDown();
    write_EEPROM();             // Save channel to EEPROM
    printCurrentSettings();
  } 
  else if (ch == '+') 
  {
    volume ++;
    if (volume == 16) volume = 15;
    radio.setVolume(volume);
    printCurrentSettings();
  } 
  else if (ch == '-') 
  {
    volume --;
    if (volume < 0) volume = 0;
    radio.setVolume(volume);
    printCurrentSettings();
  } 
  else if (ch == '0')
  {
    channel = fav_0; 
    radio.setChannel(channel);
    write_EEPROM();             // Save channel to EEPROM
    printCurrentSettings();
  }
  else if (ch == '1')
  {
    channel = fav_1;
    radio.setChannel(channel);
    write_EEPROM();             // Save channel to EEPROM
    printCurrentSettings();
  }
  else if (ch == '2')
  {
    channel = fav_2;
    radio.setChannel(channel);
    write_EEPROM();             // Save channel to EEPROM
    printCurrentSettings();
  }
  else if (ch == '3')
  {
    channel = fav_3;
    radio.setChannel(channel);
    write_EEPROM();             // Save channel to EEPROM
    printCurrentSettings();
  }
  else if (ch == '4')
  {
    channel = fav_4;
    radio.setChannel(channel);
    write_EEPROM();             // Save channel to EEPROM
    printCurrentSettings();
  }
  else if (ch == '5')
  {
    channel = fav_5;
    radio.setChannel(channel);
    write_EEPROM();             // Save channel to EEPROM
    printCurrentSettings();
  }
  else if (ch == '6')
  {
    channel = fav_6;
    radio.setChannel(channel);
    write_EEPROM();             // Save channel to EEPROM
    printCurrentSettings();
  }
  else if (ch == '7')
  {
    channel = fav_7;
    radio.setChannel(channel);
    write_EEPROM();             // Save channel to EEPROM
    printCurrentSettings();
  }
  else if (ch == '8')
  {
    channel = fav_8;
    radio.setChannel(channel);
    write_EEPROM();             // Save channel to EEPROM
    printCurrentSettings();
  }
  else if (ch == '9')
  {
    channel = fav_9;
    radio.setChannel(channel);
    write_EEPROM();             // Save channel to EEPROM
    printCurrentSettings();
  }
  else if (ch == 'r')
  {
    Serial.println("RDS listening");
    radio.readRDS(rdsBuffer, 15000);
    Serial.print("RDS heard:");
    Serial.println(rdsBuffer);      
  }
  else if (ch == 'i')
  {
    printCurrentSettings();
  }
    else if (ch == 'f')
  {
    printFavouriteList();
  }
  else if (ch == 'h')
  {
    printHelp();
  }
  else
  {
    Serial.print("Unknown command:'");
    Serial.print(ch);
    Serial.print("'");
    Serial.println(" send 'h' for help.");
  }
}
