
/*
   The circuit with shield:
 * LCD RS pin to digital pin 8
 * LCD Enable pin to digital pin 9
 * LCD D4 pin to digital pin 4
 * LCD D5 pin to digital pin 5
 * LCD D6 pin to digital pin 6
 * LCD D7 pin to digital pin 7
 * LCD R/W pin to ground
Select - pump on 600-800 
Left - minus value on 400-600 
Right - plus value on 0-100 
Up - 100-200 
Down - 200-400 
 */

// include the library code:
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <LowPower.h>


// initialize the library with the numbers of the interface pins
//LiquidCrystal lcd(12, 11, 10, 9, 8, 7); //no shield LCD
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //no shield LCD

int menu=0;
int btn=0;
int ptn=0;  //for debugging, to be removed
int mlvl=0; //moisture level
int thr=0; //moisture threshold level
int pmpt=0; //pump on time
long delayTimer = 0; //delay for the pump to avoid multiple triggering 
long delaySleep = 0; //delay for sleep mode
int menuScr = 0; // variable to detect menu scroll
int On = 0; //to mark pump ON state

void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  Serial.begin(9600);      // open the serial port at 9600 bps:
  pinMode (A3, INPUT);  //input from moisture sensor
  digitalWrite(A3, HIGH); //sets PULLUP resistor to avoid interference
  pinMode (A0, INPUT);  //buttons
  digitalWrite(A0, HIGH); //sets PULLUP resistor to avoid interference
  pinMode (2, OUTPUT);  //output for the water pump
  digitalWrite(2 , LOW); 
    pinMode (13, OUTPUT);  //output for the water pump
  digitalWrite(13, LOW); 
  Serial.println("reboot");
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
 
  lcd.setCursor(0,0);

//==============Interface ===========

 btn = analogRead (A0);
ptn = analogRead (A3);
// Serial.println(ptn);
 //Serial.print("    ");
 //Serial.println(ptn);
 
 //=============MENU SCROLLING==========
if (btn>200 && btn<400){  //Down is pressed, navigate to the next menu
    menu++;
    delay(500);
    lcd.clear(); //clears display only when button pressed
    btn = analogRead(A0);
      }
if (btn>=100 && btn<=200){  //Up is pressed, navigate to the next menu
    menu--;

    delay(500);
    lcd.clear(); //clears display only when button pressed
    btn = analogRead(A0);
      }

 //=======MOISTURE THRESHOLD SETUP======      
if (btn>400 && btn<600 && menu==1){  //Minus in Moisture thrshld is pressed,decrease 
    thr=thr-5;
    delay(500);
    lcd.clear(); //clears display only when button pressed
    btn = analogRead(A0);
      }
if (btn>0 && btn<100 && menu == 1){  //Plus is pressed in Moisture thrshld, increase
    thr= thr+5;
    delay(500);
    lcd.clear(); //clears display only when button pressed
    btn = analogRead(A0);
      }
if (btn>600 && btn<800 && menu == 1){  //Select button saves value to EEPROM
    EEPROM.write (0, 0); //clears EEPROM
    EEPROM.write (0, thr); //writes thr value to EEPROM
    delay(500);
    lcd.clear(); //clears display only when button pressed
    btn = analogRead(A0);
      }
  //=======PUMP ON TIME SETUP======      
if (btn>400 && btn<600 && menu==2){  //Minus in Pump on is pressed,decrease 
    pmpt--;
    delay(500);
    lcd.clear(); //clears display only when button pressed
    btn = analogRead(A0);
      }
if (btn>0 && btn<100 && menu ==2 ){  //Plus is pressed in Pump on, increase
    pmpt++;
    delay(500);
    lcd.clear(); //clears display only when button pressed
    btn = analogRead(A0);
      }
if (btn>600 && btn<800 && menu == 2){  //Select button saves value to EEPROM
    EEPROM.write (1, 0); //clears EEPROM
    EEPROM.write (1, pmpt); //writes pmpt value to EEPROM
    delay(500);
    lcd.clear(); //clears display only when button pressed
    btn = analogRead(A0);
}

  //delay(1000); //remove
 if(menu == 3) menu = 0; //boundaries for Menu
 if(menu == -1) menu = 2; //boundaries for Menu

//=======Includes set value displaing at boundaries=========
 
 if(thr == 105) thr = 0; //boundaries for moisture threshold 
 if(thr == -10) thr = 95; //boundaries for moisture threshold
 if(pmpt == 22) pmpt = 0; //boundaries for Pump on
 if(pmpt == -2) pmpt = 20; //boundaries for Pump on
 
//==========Displayed GUI================
  switch (menu) {
    case 0:
    //lcd.clear();
         lcd.setCursor(0,0);
         lcd.print("Moisture level  ");
         lcd.setCursor(0,1);
         mlvl = analogRead(A3);
         if(mlvl <100) mlvl = 100; //boundaries for Menu
         if(mlvl >1000 ) mlvl = 1000; //boundaries for Menu
         mlvl = map (mlvl,100,1000,99,0);
         lcd.setCursor(0,1);
         lcd.print(mlvl);
         //lcd.print("   ");
         lcd.setCursor(3,1);
         lcd.print("%");
      break;
      
      case 1:
         lcd.setCursor(0,0);
         lcd.print("Moisture thrsld");
         lcd.setCursor(0,1);
         if (thr == 100 || thr == -5){
         thr = EEPROM.read(0);
         lcd.print(thr);
         lcd.print("% set value");
         break;
         }
         lcd.print(thr);
         lcd.print("%");
      break;
    case 2:

         lcd.setCursor(0,0);
         lcd.print("Pump ON time");
         lcd.setCursor(0,1);
         if (pmpt == 21 || pmpt == -1){
         pmpt = EEPROM.read(1);
         lcd.print(pmpt);
         lcd.print(" sec set value");
         break;
         }
         lcd.print(pmpt);
         lcd.print(" sec");
      break;
  }


//===Water pump ON. Prevents miltiple triggering before the soil is soaked (30 min pause)=========
//if (mlvl<=EEPROM.read(0) && 30*60000 < (millis()-delayTimer)){ // minutes*60000 milliseconds
if (mlvl<=EEPROM.read(0) && 10000 < (millis()-delayTimer)){ // minutes*60000 milliseconds
//if (mlvl<=EEPROM.read(0)){
  delayTimer = millis();
  //Serial.println(1);
  //Serial.println(1000L*EEPROM.read(1));
 // Serial.println((millis()-delayTimer));
  Serial.println(millis());
  //digitalWrite(2, HIGH);
  On = 1;
  //digitalWrite(13, HIGH);
  //Serial.println(1000*EEPROM.read(1));
  }

     if (1000L*EEPROM.read(1) < (millis()-delayTimer) && On==1){ // minutes*60000 milliseconds no delay waiting
    //if (5000 < (millis()-delayTimer)){ // minutes*60000 milliseconds
    //delay(1000L*EEPROM.read(1));
    //delay(5000);
    // Serial.println((millis()-delayTimer));
    digitalWrite(2, LOW);
    On = 0; 
    lcd.setCursor(0,0);
    lcd.print("Save mode      ");
    lcd.setCursor(0,1);
    lcd.print("      ");
    Serial.println(millis());
    LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
    //digitalWrite(13, LOW);  
  }
//menuScr = menu;

//=============inactivity sleep mode==========
  if (menuScr == menu || 5000 < (millis()-delaySleep)){
    delaySleep = millis();
    menuScr = menu;
    lcd.setCursor(0,0);
    lcd.print("Save mode      ");
    lcd.setCursor(0,1);
    lcd.print("      ");
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
}

