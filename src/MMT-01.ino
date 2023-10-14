/*
  An example showing rainbow colours on a 1.8" TFT LCD screen
  and to show a basic example of font use.

  Make sure all the display driver and pin connections are correct by
  editing the User_Setup.h file in the TFT_eSPI library folder.

  Note that yield() or delay(0) must be called in long duration for/while
  loops to stop the ESP8266 watchdog triggering.

  #########################################################################
  ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
  #########################################################################
*/

#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <QuadratureEncoder.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

unsigned long targetTime = 0;
byte red = 31;
byte green = 0;
byte blue = 0;
byte state = 0;
unsigned int colour = red << 11;
uint32_t runing = 0;

#define MM 1
#define CM 10


int unit = CM;
int CPR = 2400;
float pi = 3.14159;
float radius = 25;
float perimeter = 2*pi*radius;
int precision = 3;  // Number of digits after decimal point



int iCountPulses = 0;



#define Encoder_output_A 13 // pin 12 of the Arduino
#define Encoder_output_B 12 // pin 14 of the Arduino
#define BUTTON_PIN_RST 0
#define BUTTON_PIN_OK 14

Encoders theEncoder;


void setup(void)
{
  Serial.begin(115200);
  
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);


  perimeter = 2*pi*(radius/unit);
  switch (unit) {
        case MM:
          precision = 1;
          break;
        case CM:
          precision = 2;
          break;
  }

  theEncoder.setup(Encoder_output_A, Encoder_output_B);

}

void loop()
{

    
    if (targetTime < millis()) {
        targetTime = millis() + 100;

        if (!digitalRead(BUTTON_PIN_RST)){
          //iCountPulses = 0;
          theEncoder.setEncoderCount(0);
          tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
          tft.drawRoundRect(0, 0, 70, 85, 5, TFT_RED);
          tft.drawString("RST",10,30,4);
        } else{
          tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
          tft.drawRoundRect(0, 0, 70, 85, 5, TFT_BLACK);
          tft.drawString("RST",10,30,4);
        }

        tft.setTextColor(TFT_WHITE, TFT_BLACK, true);

        float fValue = 0.0;
        int xpos = 80;
        fValue = theEncoder.getEncoderCount() * (perimeter / CPR);
        xpos += tft.drawFloat(fValue, precision, xpos, 54, 6);
        tft.drawString("                ", xpos, 54, 6);
        String s = "";

        switch (unit) {
          case MM:
            s = " mm";
            break;
          case CM:
            s = " cm";
            break;
        }
        xpos += tft.drawString(s + "                   ", xpos, 70, 4);

        tft.drawRoundRect(0, 0, 320, 170, 5, TFT_RED);

        

    }
}






