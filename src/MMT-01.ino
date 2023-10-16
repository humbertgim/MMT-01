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
#include <EEPROM.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

unsigned long targetTime = 0;
unsigned long tPushOK = 0;
int iLengthStateChange = 2000;
byte red = 31;
byte green = 0;
byte blue = 0;
byte state = 0;
unsigned int colour = red << 11;
uint32_t runing = 0;

#define MM 1
#define CM 10
#define INCH 2.54

#define STATE_NORMAL 1
#define STATE_CALIBRATION 2
int iState = 1;

int unit = CM;
int CPR = 2400;
float pi = 3.14159;
float radius = 25;
float perimeter = 2*pi*radius;
int precision = 3;  // Number of digits after decimal point

float fBladeOffset = 0.5; //Blade offset in the selected units


int iCountPulses = 0;

bool isBladeOffsetNeeded = false;
bool isBOKReleased = true;


#define Encoder_output_A 13 // pin 12 of the Arduino
#define Encoder_output_B 12 // pin 14 of the Arduino
#define BUTTON_PIN_RST 0
#define BUTTON_PIN_OK 14

Encoders theEncoder;


void setup(void)
{
  Serial.begin(115200);
  EEPROM.begin(8);
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
  fBladeOffset = EEPROM.readFloat(0);
  theEncoder.setup(Encoder_output_B, Encoder_output_A);

}

void loop()
{
    //We should or could include the OTA update
    
    if (targetTime < millis()) {
        targetTime = millis() + 100;
        String sUnits = "";

        switch (unit) {
          case MM:
            sUnits = " mm";
            break;
          case CM:
            sUnits = " cm";
            break;
        }
        //Get the encoder value at the very beginning
        float fValue = 0.0;
        
        
        fValue = theEncoder.getEncoderCount() * (perimeter / CPR);

        //Button logic for main menu
        if (!digitalRead(BUTTON_PIN_RST)){
          //Paint area when button pressed, and reset the counter
          theEncoder.setEncoderCount(0);
          tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
          tft.drawRoundRect(0, 0, 70, 85, 5, TFT_RED);
          tft.drawString("RST",10,30,4);
        } else{
          //Clean area when not pressed
          tft.fillRoundRect(0, 0, 70, 85, 5, TFT_BLACK);
          
        }

        if (!digitalRead(BUTTON_PIN_OK)){
          //Switch between blade offset mode and normal mode
          tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
          if(isBOKReleased){ //Only do it once
            isBOKReleased = false;
            tPushOK = millis();
              if(iState == STATE_NORMAL){ //The behavior of the button changes depending on the state we are in
                isBladeOffsetNeeded = !isBladeOffsetNeeded;
                if(isBladeOffsetNeeded){
                  tft.drawString(" Blade offset activated: " + String(fBladeOffset) + sUnits,80,150,2);
                }else{
                  tft.drawString(" No blade offset                      " ,80,150,2);  
                }
              }else if(iState == STATE_CALIBRATION){
                fBladeOffset = fValue;
                iState = STATE_NORMAL;
                tft.drawString(" Blade offset activated: " + String(fBladeOffset) + sUnits,80,150,2);
                isBladeOffsetNeeded = true;
                EEPROM.writeFloat(0,fBladeOffset); //Persist the value in EEPROM
                EEPROM.commit(); //Commit the change!!
              }
          }
          
          tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
          tft.drawString(" Maintain pushed to calibrate blade",20,5,2);
          tft.drawRoundRect(0, 85, 70, 85, 5, TFT_RED);

          //Logic to switch to calibration blade mode after the defined time in iLengthStateChange
          //Also paint a progress bar to indicate the user that certain time has passed
          unsigned long now = millis();
          float percent = 0.0f;
          if(now - tPushOK < iLengthStateChange){
            percent = abs((float)(now - tPushOK) / (float)iLengthStateChange);
          }else{
            percent = 1.0f;
            iState = STATE_CALIBRATION;
            isBladeOffsetNeeded = false;
            tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
            tft.drawString("Calibrating blade                 ",80,150,2);
          }
          int step = (int)(85.0*percent);
          if(step < 10){
            step = 10; //This solves a bug in the rendering of the roundrect (it is discounting the radius, so it paints in negative areas!!!!)
          }
          Serial.println("Percent: " + String(percent) + " Step: " + String(step));
          tft.fillRoundRect(0,85,70, step,5,TFT_RED);
          tft.setTextColor(TFT_WHITE, TFT_BLACK, false);

          tft.drawString("BO",10,115,4);
        } else{
          isBOKReleased = true;
          tft.setTextColor(TFT_BLACK, TFT_BLACK, true);
          tft.drawString(" Maintain pushed to calibrate blade",20,5,2);
          tft.fillRoundRect(0, 85, 70, 170, 5, TFT_BLACK);
          //tft.drawString("BO",10,115,4);
        }

        

        //Draw the value taking into account the blade offset option
        tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
        if(isBladeOffsetNeeded){
          if(fValue < fBladeOffset){
            tft.setTextColor(TFT_RED, TFT_BLACK, true);
            fValue = 0.0;
          }else{
            fValue = fValue - fBladeOffset;
          }
        }
        int xpos = 80;
        xpos += tft.drawFloat(fValue, precision, xpos, 54, 6);
        tft.drawString("                ", xpos, 54, 6);
        
        xpos += tft.drawString(sUnits + "                   ", xpos, 70, 4);

        tft.drawRoundRect(0, 0, 320, 170, 5, TFT_RED);


        //Draw the circle with the moving line
        int iRadius = 20;
        tft.fillCircle(290,30,iRadius,TFT_WHITE);
        tft.drawCircle(290,30,iRadius,TFT_BLUE);
        int x = 0;
        int y = 0;
        int counter = theEncoder.getEncoderCount();
        
        float fAngle = (((float)counter / (float)CPR)*360.0f) * (pi/-180.0f);
        x = (int)(sinf(fAngle) * (float)iRadius);
        y = (int)(cosf(fAngle) * (float)iRadius)*-1; //The -1 is to make it go in the right direction with the wheel
        //Serial.println("Counter: " + String(counter) + " fAngle: " + String(fAngle) + " x,y: " + String(x) + "," + String(y));
        tft.drawLine(290,30,290 + x,30 + y,TFT_RED);
        

        

    }
}






