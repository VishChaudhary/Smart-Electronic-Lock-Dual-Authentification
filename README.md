# Smart-Electronic-Lock-Dual-Authentification-
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
