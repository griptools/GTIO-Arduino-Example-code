/********************************************************************
February 1, 2018 
V1.0 Copyright (C)2018 by Jan van Akker, FrieslandAV
griptools.io Multi IO interface for the Arduino:
This example demonstrates the use of the special GTIO node package struct.
griptools.io(t) sends a series of values over USB to the Arduino.
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
********************************************************************/
 

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
// Local variables
//-----------------------------------------------------------------        
int led        = 9;    // the pin that the LED is attached to
int brightness = 0;    // how bright the LED is
int received   = 0;
int i          = 0;


//-----------------------------------------------------------------
// Setup
//-----------------------------------------------------------------
void setup()
{
  // declare pin 9 to be an output:
  pinMode(led, OUTPUT); 
    
  // Create serial object to communicate with griptools.io
  Serial.begin(115200);
    
  // wait until the serial port is open
  while (!Serial); 
}

//-----------------------------------------------------------------
void loop()
{ 
  int j;
  uint16_t blocks;
  char buf[32];

  
  // have the arduino wait to receive input
  while(Serial.available() == 0);
  {
    received = 0;
    // Serial.println(Serial.available()); 
    
    switch( Serial.peek() )
       {
        // we have recaived a channel data package from griptools.io
        case ACTUATOR_PACKET_TYPE_CHANNEL_DATA:
        {         
             // get the package and store it in our local struct              
             received = Serial.readBytes((char*)&actuatorChannelData, Serial.available());
           
             // check if there is any data in the package, if not skip                          
             if (actuatorChannelData.nrOfChannels == 0) break;
                      
             // dispatch the received channel data
             i=0;
             while (i < actuatorChannelData.nrOfChannels)             
             {
                //Serial.println(i); debug

                // if this is our channel, use the value to set the brightness of the led pin 9:
                if ((int)actuatorChannelData.channel[i].channelNr == 0)                
                {
                  // GTIO send always normalized data (0..1), so map the value to the range we need here
                  brightness = int(actuatorChannelData.channel[0].value * 255);
                  
                  // optional safety check range limits                
                  constrain(brightness, 0, 255);
                  
                  // write to the pin
                  analogWrite(led, brightness);
                }
                // next Channel  
                i++;
                                
             }
             
             break;
        }  
                        
       
       } // switch end
  
    Serial.flush();
    
  } // while end

} // loop end


