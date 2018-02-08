/********************************************************************
February 1, 2018
V1.0 Copyright (C)2018 by Jan van Akker, FrieslandAV
griptools.io Multi IO interface for the Arduino:
This example demonstrates the use of the special GTIO node package struct.
griptools.io(t) sends/receives a series of values over USB to the Arduino.
The package struct is defined below.
This demo code is in the public domain and you can use/change it as you please.
For more information and free documentation, see http://www.griptools.io
All experimentation is at your own risk.


*********************************************************************
This code is intended as a basic example.
You can adapt the code for your own use as long as you keep the package
definitions intact.
If you have suggestions, for changes, please let us us know: 
support@griptools.io
********************************************************************

*********************************************************************
ADAFRUIT PWN SERVODRIVER
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);

Depending on your servo make, the pulse width min and max may vary, you 
want these to be as small/large as possible without hitting the hard stop
for max range. You'll have to tweak them as necessary to match the servos you
have! 

************************************************************************/


#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

//griptools.io enables the min/max servo pulse length setting in the application
//Be carefull to change/experiment the settings. Hitting the servos hardstop can damage the motors.
#define SERVOMIN  /*80  //*/  150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  /*800 //*/  600 // this is the 'maximum' pulse length count (out of 4096)

 
int received      = 0;
//int i             = 0;
bool ledOn        = false;
//uint16_t pulselen = 0;     // variable to store the curreny servo position
//uint8_t servoNr   = 0;     // the target servo

//-----------------------------------------------------------------
// griptools.io general constants
//-----------------------------------------------------------------
// actuatorInfo Hardware type
const int ACTUATOR_TYPE_UNKNOWN    = 001;
const int ACTUATOR_TYPE_SERVO      = 002;
const int ACTUATOR_TYPE_STEPPER    = 003;
const int ACTUATOR_TYPE_DCMOTOR    = 004;
const int ACTUATOR_TYPE_DMX_LIGHT  = 005;
const int ACTUATOR_TYPE_DYNAMIX_12 = 006;

// actuatorInfo motion values wanted
const int VALUE_TYPE_BYTE          = 001;
const int VALUE_TYPE_INTEGER       = 002;
const int VALUE_TYPE_FLOAT         = 003;
const int VALUE_TYPE_UNKNOWN       = 004;

// support for feedback
const int FEEDBACK_NONE            = 001;
const int FEEDBACK_FLOAT           = 002;
const int FEEDBACK_INT32           = 003;


//-----------------------------------------------------------------
// serial communication packet ID constants
//-----------------------------------------------------------------
const int ACTUATOR_PACKET_TYPE_INFO          = 50;
const int ACTUATOR_PACKET_TYPE_COMMAND       = 51;
const int ACTUATOR_PACKET_TYPE_CHANNEL_DATA  = 54;
const int ACTUATOR_PACKET_TYPE_FEEDBACK_DATA = 55;

//-----------------------------------------------------------------
// Command ID constants from software
//-----------------------------------------------------------------
const int SEND_ACTUATOR_INFO     = 100;
const int SEND_ACTUATOR_FEEDBACK = 101;
// add (future) commands here:


//-----------------------------------------------------------------
// Command Packet definition 
//-----------------------------------------------------------------
struct ACTUATOR_PACKET_COMMAND 
{
    byte packetID;
    byte commandID;
} __attribute__((__packed__)) actuatorCommand;

//-----------------------------------------------------------------
// Info Packet definition 
//-----------------------------------------------------------------
struct ACTUATOR_PACKET_INFO
{
    byte packetID;
    byte nrChannels;
    byte actuatorType;    // servo - stepper - motor - light - unknown
    byte valueType;       // integer - float - byte
    byte supportsFeedback;//; None - Float - Int32
    byte refreshFreq;
    int valueMin;         // 16 bit integer!
    int valueMax;         // 16 bit integer!
} __attribute__((__packed__)) actuatorInfo;

//-----------------------------------------------------------------
// Value Packet definition III ( multi Value )
//-----------------------------------------------------------------
struct CHANNEL_DATA
{
   byte    channelNr;
   float   value;
} __attribute__((__packed__)) channelData;

struct ACTUATOR_PACKET_CHANNEL_DATA
{
   byte packetID;
   byte nrOfChannels;
   CHANNEL_DATA channel[128];
} __attribute__((__packed__)) actuatorChannelData;




//-----------------------------------------------------------------
// Setup
//-----------------------------------------------------------------
void setup()
{
  // declare pin 9 to be an output:
  pinMode(13, OUTPUT);  //internal LED




  // Create serial object to communicate with griptools.io
  //  Serial.begin(115200);
  Serial.begin(256000);  
  // wait until the serial port is open
  while (!Serial);

  pwm.begin();
  pwm.setPWMFreq(60);
    
}

/* DEMO EXAMPLE
void loop() 
{
  // Drive each servo one at a time
  Serial.println(servoNr);
  for (uint16_t pulselen = SERVOMIN; pulselen < SERVOMAX; pulselen++) {
    pwm.setPWM(servoNr, 0, pulselen);
  }
  delay(500);
  for (uint16_t pulselen = SERVOMAX; pulselen > SERVOMIN; pulselen--) {
    pwm.setPWM(servoNr, 0, pulselen);
  }
  delay(500);

  servoNr ++;
  if (servoNr > 3) servoNr = 0;
}

*/
//000000000000000000000000000000000000000000000000000000000000000000000000000000
void loop()
{ 
  // have the arduino wait to receive input
  while(Serial.available() == 0);
  {
    received = 0;
    
    switch( Serial.peek() )
    {
        
       // we have recaived a channel data package from griptools.io
       case ACTUATOR_PACKET_TYPE_CHANNEL_DATA:
        
            // get the package and store it in our local struct              
            received = Serial.readBytes((char*)&actuatorChannelData, Serial.available());

            // check if there is any data in the package, if not skip                          
            if (uint16_t(actuatorChannelData.nrOfChannels) == 0) 
               break;
               
            // dispatch the received channel data
          
            for( uint16_t i = 0; i < actuatorChannelData.nrOfChannels;  i++ )
            { 
               uint16_t servoNr  = i;// actuatorChannelData.channel[i].channelNr;
               uint16_t pulselen = (actuatorChannelData.channel[i].value * SERVOMAX) +SERVOMIN;  
              
               pwm.setPWM(servoNr, 0, pulselen); 
                
            }  // end for loop
             
            break;  // case of
               
    } // switch end
  
    Serial.flush();
    
  } // while end

} // loop end





