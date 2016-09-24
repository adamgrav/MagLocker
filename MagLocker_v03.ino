/*  Adam Gravely
 *  The MagLocker v_03,3
 *  1/22/2016
 *  
 *  credit to: 
 *    Josh Hawley
 *    Vincent Demay
 *  
 *  Spacebar_2016
 */

#include <Key.h>
#include <Keypad.h>
#include <SoftwareSerial.h>
#include <FPS_GT511C1R.h>
#include <serLCD.h>
#include <Servo.h>
#include <Password.h>

//SoftwareSerial mySerial(11, 12); // pin 11 = TX, pin 3 = RX (unused) //CHECK IF YOU NEED
FPS_GT511C1R fps(9, 10); //FPS connected to pin 9 and 10


//**KEYPAD VARIABLES
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
{'1','2','3'},
{'4','5','6'},
{'7','8','9'},
{'*','0','#'}
};
byte rowPins[ROWS] = {8, 7, 6, 5}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {4, 3, 2};    //connect to the column pinouts of the kpd

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//**SOLENOID VARIABLES
int servoPin = 13;
Servo Servo1;

//**LED STUFF
int redLED = A4;
int blueLED = A5;

//**lcdPin
int lcdPin = 11;
serLCD lcd(11);

//**LOOP VARIABLES
char addElementCode[6] = {'1','1','5','7','3','4'}; //Adds an element: 115734
char unlockCode[6] = {'5','7','1','9','1','4'}; //Unlocks: 571914
char resetCode[6] = {'2','2','7','8','5','1'}; //Resets FPS: 227851
boolean registeredElement;
char currentKey[6];

Password newElement = Password("115734");
Password unlockIt = Password("571914");
Password resetIt = Password("227851");

char firstChar;

//SETUP--------------------------------------------------

void setup() {
  // setup code, runs once

  //*** INITIALIZE
  fps.Open(); // initializes FPS
  fps.SetLED(true); //turns on FPS LED

  pinMode(redLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  digitalWrite(redLED, HIGH); //Sets Red LED High
  digitalWrite(blueLED, LOW); //Sets Blue LED Low
  
  Servo1.attach(servoPin);
  Servo1.write(180); //Sets Servo to 180

  //lcd.setBrightness(25);
  //lcd.clear();
  //delay(500);
  //lcd.selectLine(1);
  //lcd.print("----------------");
  //lcd.selectLine(2);
  //lcd.print("The MagLocker");
  //lcd.setSplash();
}

//LOOP--------------------------------------------------

void loop() {
  char keypressed = kpd.getKey();
  lcd.clear();
  lcd.selectLine(1);
  lcd.print("Use sensor or");
  lcd.selectLine(2);
  lcd.print("use keypad");
  
  //***FINGERPRINT SENSOR
  while(!fps.IsPressFinger() || !keypressed) { //*variable~
    keypressed = kpd.getKey();
    if(fps.IsPressFinger()) {
      fingerPrintSensor();
    }
    else if(keypressed) { 
      firstChar = keypressed;
      keyPad();
    }
  }
  
  delay(100);
}

//--------------------------------------------------

void reset() {
  lcd.clear();
  delay(100);
  fps.Open();
  //Delete all stored finger print
  fps.DeleteAll();
  lcd.selectLine(1);
  lcd.print("Locker Reset");
}

//--------------------------------------------------

void fingerPrintSensor() {
  fps.CaptureFinger(false);
    int id = fps.Identify1_N();
    if (id <200)
    {
      lcd.clear();
      lcd.print("Verified ID: ");
      lcd.print(id);
      unlock(); //***UNLOCK MECHANISM
    }
    else
    {
      lcd.clear();
      lcd.selectLine(1);
      lcd.print("Finger not found");
      delay(500);
      loop();
    }
}

//--------------------------------------------------

void Enroll()
{
  // Enroll test

  // find open enroll id
  int enrollid = 1;
  bool usedid = true;
  while (usedid == true)
  {
    usedid = fps.CheckEnrolled(enrollid);
    if (usedid==true) enrollid++;
  }
  fps.EnrollStart(enrollid);

  // enroll
  lcd.clear();
  lcd.selectLine(1);
  lcd.print("Press finger to");
  lcd.selectLine(2);
  lcd.print("Enroll #");
  lcd.print(enrollid);
  while(fps.IsPressFinger() == false) delay(100);
  bool bret = fps.CaptureFinger(true);
  int iret = 0;
  if (bret != false)
  {
    lcd.clear();
    lcd.selectLine(1);
    lcd.print("Remove finger");
    fps.Enroll1(); 
    while(fps.IsPressFinger() == true) delay(100);
    lcd.clear();
    lcd.selectLine(1);
    lcd.print("Press same");
    lcd.selectLine(2);
    lcd.print("finger again");
    while(fps.IsPressFinger() == false) delay(100);
    bret = fps.CaptureFinger(true);
    if (bret != false)
    {
      lcd.clear();
      lcd.selectLine(1);
      lcd.print("Remove finger");
      fps.Enroll2();
      while(fps.IsPressFinger() == true) delay(100);
      lcd.clear();
      lcd.selectLine(1);
      lcd.print("Press same");
      lcd.selectLine(2);
      lcd.print("finger yet again");
      while(fps.IsPressFinger() == false) delay(100);
      bret = fps.CaptureFinger(true);
      if (bret != false)
      {
        lcd.clear();
        lcd.selectLine(1);
        lcd.print("Remove finger");
        iret = fps.Enroll3();
        if (iret == 0)
        {
          lcd.clear();
          lcd.selectLine(1);
          lcd.print("Enrolling");
          lcd.selectLine(2);
          lcd.print("successfull");
          delay(500);
        }
        else
        {
          lcd.clear();
          lcd.print("Enrolling failed with error code:");
          lcd.print(iret);
          delay(500);
          Enroll();
        }
      }
      else {
        lcd.clear();
        lcd.selectLine(1);
        lcd.print("Failed to captu-");
        lcd.selectLine(2);
        lcd.print("re third finger");
        delay(500);
        Enroll();
      }
    }
    else {
      lcd.clear();
      lcd.selectLine(1);
      lcd.print("Failed to captu-");
      lcd.selectLine(2);
      lcd.print("re second finger");
      delay(500);
      Enroll();
    }
  }
  else {
    lcd.clear();
    lcd.selectLine(1);
    lcd.print("Failed to captu-");
    lcd.selectLine(2);
    lcd.print("re first finger");
    delay(500);
    Enroll();
  }
}

//--------------------------------------------------

void keyPad() {
  char keypressed = kpd.getKey();
  
  lcd.clear();
  lcd.selectLine(1);
  lcd.print("Enter access ");
  lcd.selectLine(2);
  lcd.print("code: ");
  lcd.print(firstChar); //prints first key
  
  currentKey[0] = firstChar; //adds first key to first spot in array
  int countKey=1; //counts number of keys pressed
  while(countKey != 6) {
    keypressed = kpd.getKey(); //gets char from keypad
    if(keypressed != NO_KEY) { //prints chars and adds to currentKey
      lcd.print(keypressed);
      currentKey[countKey] = keypressed;
      countKey++;
    }
  }
  conditionals();
}

//--------------------------------------------------

void unlock() {
  lcd.clear();
  lcd.selectLine(1);
  lcd.print("Unlocked");
  digitalWrite(blueLED, HIGH);
  digitalWrite(redLED, LOW);
  //digitalWrite(lockPin, LOW); //turns off the solenoid
  //delay(5000);               //for 5 seconds
  //digitalWrite(lockPin, HIGH);//turns the solenoid back on

  Servo1.write(0); // Rotates servo 90 degrees
  delay(5000);
  Servo1.write(180);
  
  loop();
}

//--------------------------------------------------

void conditionals() {
  
  //**CONDITIONALS FOR ACTION
  if(addElementCheck()==true) {
    lcd.clear();
    lcd.print("Add an element..");
    delay(1000);
    Enroll();
    registeredElement = true;
    loop();
  }
  else if(unlockCheck()==true) {
    unlock(); //UNLOCK MECHANISM
    loop();
  }
  else if(resetCheck()==true) {
    reset();
    lcd.clear();
    lcd.print("Sensor reset");
    delay(1000);
    loop();
  }
  else {
    lcd.clear();
    lcd.selectLine(1);
    lcd.print("Incorrect code. ");
    lcd.selectLine(2);
    lcd.print("Try again.");
    delay(500);
    loop();
  }
}

//--------------------------------------------------

boolean addElementCheck() {
  boolean addElement;
  for(int i=0; i<sizeof(currentKey); i++) {
    if(currentKey[i]==addElementCode[i]) {
      addElement = true;
    }
    else
      addElement = false;
      break;
  }
  return addElement;
}

//--------------------------------------------------

boolean unlockCheck() {
  boolean unlock;
  for(int i=0; i<sizeof(currentKey); i++) {
    if(currentKey[i]==unlockCode[i]) {
      unlock = true;
    }
    else
      unlock = false;
      break;
  }
  return unlock;
}

//--------------------------------------------------

boolean resetCheck() {
  boolean reset;
  for(int i=0; i<sizeof(currentKey); i++) {
    if(currentKey[i]==resetCode[i]) {
      reset = true;
    }
    else
      reset = false;
      break;
  }
  return reset;
}
