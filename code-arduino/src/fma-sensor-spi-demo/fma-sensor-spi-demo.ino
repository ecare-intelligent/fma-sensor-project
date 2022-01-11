/*
 Circuit:
 FMA sensor attached to pins 10, 12 - 13:

 MISO: pin 12
 SCK: pin 13
 SS/CSB: pin 10

 * SPI 
 SPISettings mySettting(speedMaximum, dataOrder, dataMode)
 speedMaximum = 800000
 dataOrder = MSBFIRST
 dataMode = SPI_MODE0

 * Force Calc
 FMAMSDXX005WCSC3
 5N force range, compression force type, SPI digital output, 20% to 80% transfer function, 3.3 V ±10% supply voltage

 force = (output - output_min) * rated_force_range / (output_max- outputmin)
 
 output_max = 2^14*0.8 = 13107.2 = 0x3333
 output_min = 2^14*0.2 = 3276.8 = 0xCCC
 rated_force_range = 5
 force = (output - 3276) * 5 / (13107- 3276)
 force = (output - 3276) * 5 / 9831
 force = (5*output - 16380) / 9831

 * Temperature Calc
 temperature = output / 2047 * 200 - 50
 RESOLUTION 0.1ºC

 created 8 Dec 2021
 by Edward Yin
 
*/
// the sensor communicates using SPI, so include the library:
#include <SPI.h>

// set pin 10 as the slave select for the digital pot:

const int slaveSelectPin = 10;
uint32_t data = 0;
uint32_t status = 0;
uint32_t force_raw = 0;
uint32_t temperature_raw = 0;
double force = 0;
double temperature = 0;

void setup()
{
    Serial.begin(9600);

    // set the slaveSelectPin as an output:
    pinMode(slaveSelectPin, OUTPUT);

    // initialize SPI:

    //Tring use another setting, but not work yet
    //SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
    SPI.begin();
    
    digitalWrite(slaveSelectPin, HIGH);
}

void loop()
{

    data = readRawData(); //32bit

    status = data >> 30 & 0x03;  //you only needs bits 14,15

    force_raw = data >> 16 & 0x3FFF;

    temperature_raw = data >> 5 & 0x7FF;

    temperature = temperature_raw * 200 / 2047.0 - 50;

    force = (5 * force_raw - 16380) / 9831.0 *1000;

    if (status!=0)
    {
        //Serial.println("Error,Status="+String(status));
    }
    else
    {
        // View in Serial
        //Serial.println("Status="+ String(status)+",Force[mN]=" + String(force)+",Temperature[C]=" + String(temperature,1));

        // View in Serial Graph
        Serial.println("0,"+String(force)+",5000");

        // View all data
        //Serial.println(String(status)+","+String(force)+","+String(temperature,1)+","+String(force_raw)+","+String(temperature_raw));
    }
    
    
}

uint32_t readRawData()
{
    uint32_t result = 0; // result to return

    // take the chip select low to select the device:
    digitalWrite(slaveSelectPin, LOW);

    // send a value of 0 to read the first byte returned:

    result |= (uint32_t)SPI.transfer(0xff) << 24;
    result |= (uint32_t)SPI.transfer(0xff) << 16;
    result |= (uint32_t)SPI.transfer(0xff) << 8;
    result |= (uint32_t)SPI.transfer(0xff);

    // decrement the number of bytes left to read:
    digitalWrite(slaveSelectPin, HIGH);

    // return the result:
    return (result);
}
