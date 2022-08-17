/*

  The Smart Electronic Lock is a dual authentication lock that uses a keypad and RFID module as security measures and spins a servo to mimic 'locking' and 'unlocking' a lock.
  
  The components of this project are:
    - 4x4 Keypad 
    - RFID Sensor & 2 ID cards to unlock it
    - 16x2 LCD Display
    - 9g Micro-Servo
    - Potentiometer
  
  What happens in the project:

  When the code first runs it prompts the user on the LCD to set a six-digit password. The LCD displays the password to be set as it is entered.
  Once the password is set the code asks if that is the password you want to set or if you would like to set a different password. If you press 'A' on
  the keypad you can set a new password and if you press any other key the password you initially entered is set as your password. Once the user's password is
  set the electronic lock is set-up and ready to use.

  After this the LCD prompts you to enter your password to get past the first security measure. If the password is incorrect the LCD displays 'Not allowed' 
  and prompts you to press 'B' if you want to change the password. If the user wants to change the password they press 'B' and then enter the correct passowrd and 
  the new password they want to set. If the password is entered correctly then the LCD prompts the user to scan their card. If the card is incorrect you get sent
  to the top of the loop amd have to re-enter your password. If the card is correct the servo is spun open. To close the lock you have to scan your card and this
  will spin the servo in reverse.

  Author: Vish Chaudhary

  Github: https://github.com/VishChaudhary

*/

#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

#define RST_PIN 5   // defining the RST pin for the RFID Module
#define SS_PIN 53   //  defining the SS pin for the RFID Module

const int rs = 13, en = 12, d4 = 47, d5 = 4, d6 = 3, d7 = 2, d0= 49, d1= 48, d2= 46, d3= 45;  //defining the LCD pins
LiquidCrystal lcd(rs, en,d0, d1, d2, d3, d4, d5, d6, d7); //Creating the LCD object to control the LCD

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
Servo myservo;  // create servo object to control a servo

const int rows = 4; //defining the rows for our 4x4 matrix
const int columns =4; //defining the coulmns for our 4x4 matrix

//Setting up a 2D matrix for the 4x4 keypad
char keyConfig [rows][columns] =    
{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
  
byte rowPins[rows] = {38, 39, 36, 37}; //connect to the row pinouts of the keypad
byte colPins[columns] = {32, 33, 30,31}; //connect to the column pinouts of the keypad

Keypad myKeypad = Keypad( makeKeymap(keyConfig), rowPins, colPins, rows, columns ); //  create keypad object to control the keypad
char myPassword[7] = {};  //creating the password array, this will store the users password.

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();          // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  myservo.attach(9);    // attaches the servo on pin 9 to the servo object
  myservo.write(0);     // Sets the servo to position 0
  lcd.clear();
  lcd.setCursor(0,0);   //Set the LCD cursor to (0,0)
  lcd.print("Welcome to the");
  lcd.setCursor(0,1);
  lcd.print("smart lock!");
  delay(2000);
  lcd.clear();
  lcd.home();
  lcd.print("Please set your");
  lcd.setCursor(0,1);
  lcd.print("6-digit password");  //Displays 'Please set your 6-digit password' on the lcd.
  delay(1000);
  setPassword(myPassword);  //Calling the setpassword function.
  delay(100);
  delay(100);
}

void loop() {
  int access = 0;
  int denied = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enter password/"); 
  lcd.setCursor(0,1);
  lcd.print("B = new password");  //prompts user to enter their password.
  delay(500);
  access = checkPass(access); //calls the check password function
  if(access==7) //If access is equal to 7 then go back to the start of the loop (this means the password was reset)
  {
    return;
  }
  
  if (access != 6)  //This means the password was incorrect
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Not Allowed");
    return;
  }
  
  else if (access == 6) //This means the entered password was correct
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Authorized");
    delay(1500);
    
    //RFID code
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Scan your card");  //Prompts the user to scan their card
    delay(1500);
    denied = checkRFID(); //calls the checkRFID function

    if (denied == 1)  //incorrect card scanned
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("ACCESS DENINED");
      delay(1000);
      return;
    }
    else  //Correct card scanned
    {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("ACCESS GRANTED");
    delay(500);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Opening...");
    forwardSpin();  //calls the forward spin function to spin the servo 180 degrees forward
    label_2: lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Scan card to");  //prompts the user to scan their card to close the safe
    lcd.setCursor(0,1);
    lcd.print("close safe");
    delay(500);
    denied = checkRFID(); //calls the checkRFID function to close the safe
    if (denied == 1)  //if the incorrect card is scanned the user is prompted to scan the correct card
    {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Incorrect Card!");
    delay(1000);
    goto label_2;
    }
    else  //If the correct card is scanned then the servo is spun 180 degrees in the opposite direction
    {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Closing...");  
    reverseSpin();  //calls the reverseSpin function which spins the servo 180 degrees in the reverse direction
    return;  
    }
    }
  }
}

//Sets the password
void setPassword(char myPassword[7]){
  char setAnswer;
  
  for (int i=0; i<6; i++)
  {
    myPassword[i]= myKeypad.waitForKey();  //Infinetly waits until a key is pressed
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(myPassword);  //displays the passowrd live as it is entered.
  }
  delay(100);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set as password:");  //Asks for confirmation to set this as the password.
  lcd.setCursor(8,1);
  lcd.print(myPassword);
  delay(1500);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("A=set/other =no"); //Press A to set as password or press any other key to re-enter and set a new password.
  lcd.setCursor(0,1);
  lcd.print(myPassword);
  delay(500);
  setAnswer=myKeypad.waitForKey();
  
  if(setAnswer== 'A') //If key is 'A' then set as password
  {
    return;
  }

  else 
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set passowrd:");
    for(int i=0; i<6; i++)
    {
    myPassword[i] = {}; //resets the password to an empty array
    }
    delay(1000);
    setPassword(myPassword);  //calls the setPassword function
  }
}
//Changes the password
void changePassword(char myPassword[7])
{
  int newPass=0;
  int j;
  int i;
  char checkPassword[7];
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Current Password");  //Asks for the current password
  delay(1000);
  lcd.setCursor(0,1);
  for (i=0; i<6; i++)
  {
    checkPassword[i]= myKeypad.waitForKey();  
    lcd.print(checkPassword[i]);
  }
  delay(500);
  for (int j=0; j<6; j++) //compares the two arrays checkPassword and the original password array.
  {
    if(checkPassword[j]== myPassword[j])
    {
      newPass+=1;
    }
  }
  if(newPass == 6)  // If all six password digits match then set a new password
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Correct! Set new");
    lcd.setCursor(0,1);
    lcd.print("password");
    delay(3000);
    for(int i=0; i<6; i++)
    {
    myPassword[i] = {}; //resets the password to a blank array
    }
    setPassword(myPassword);  //calls the setPassword function
    return;
  }

  else
  {
    lcd.clear();  //If the entered password isn't correct then 'Incorrect is displayed on the LCD
    lcd.setCursor(0,0);
    lcd.print("Incorrect!");
    delay(2000);
    return;
  }
}
//Checks to see if the entered password is correct
int checkPass(int access)
{
  access=0;  
  char enteredPass[7]={};
  for (int i=0; i<6; i++)
  {
    enteredPass[i]= myKeypad.waitForKey(); //gets the input key presses from the user
    if (enteredPass[i]== 'B')   //If the user presses 'B' the change password function is called.
    {
      changePassword(myPassword);
      access=7; 
      return access;
      
    } 
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(enteredPass);
    delay(10);
  }
  
  for (int i=0; i<6; i++)
  {
    if(enteredPass[i]== myPassword[i])   //compares the two arrays enteredPass and the original password array.
    {
      access+=1;
    }
  }
  if (access==6)  // If all six password digits match then set a new password
  {
    delay(500);
    return access;
  }

  else if(access!= 6) //If the entered password isn't correct then 'Incorrect is displayed on the LCD
  {
    lcd.clear();
    lcd.setCursor(0,0); 
    lcd.print("Incorrect");
    delay(800);
    return access;
  }
}
//checks the ID card to see if it's authorized to open the lock.
int checkRFID(){
  label_1: 
  if ( (! mfrc522.PICC_IsNewCardPresent())||(! mfrc522.PICC_ReadCardSerial()))  // If a new card isn't present or it's not a good card read then loop back to the start of the function
  {
    delay(100);
    goto label_1;
  }
  
  else{ 
    String content= "";
    for (int i = 0; i < mfrc522.uid.size; i++) 
    {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    if ((content.substring(1) == "D3 C3 1B 05")||(content.substring(1) == "FC 28 05 49")) //Compares scanned card's UID to correct UID's
    {
      int denied = 0; //If the ID card has the correct UID then return denied = 0 (access authorized)
      delay(500);
      return denied;
    }
    else     //If the UID is incorrect then return denied = 1 (access denied)
    {   
      int denied = 1;
      return denied;
    }
  }
}

void forwardSpin(){
  for (int servoPosForward = 0; servoPosForward <= 180; servoPosForward += 1)   // goes from 0 degrees to 180 degrees in steps of 1 degree
    { 
	    myservo.write(servoPosForward);              // tell servo to go to position in variable 'servoPosForward'
	    delay(15);  
    } 
}

void reverseSpin(){
  for (int servoPosReverse = 180; servoPosReverse >= 0; servoPosReverse -= 1)   // goes from 180 degrees to 0 degrees in steps of 1 degree
    { 
	    myservo.write(servoPosReverse);              // tell servo to go to position in variable 'servoPosForward'
	    delay(15);  
    } 
}



