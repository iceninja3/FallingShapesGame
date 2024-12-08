**This game is complete! No further updates planned!**

## [Presentation Link](https://docs.google.com/presentation/d/1nt9gRMgprMrjdH6lQ8kB16Lm4ZPe6Ehf_eqI9vMX2Y4/edit#slide=id.g31c968ffbe4_2_91)
## [Video Demo Link](https://www.youtube.com/watch?v=UFumihdRavY)

## Quick Summary:
I went a little above and beyond fpr a required project in the Fall 2024 Engr 1IT course at UCLA. This project lets a user play a game with "falling" shapes (sort of like the shapes in tetris). To get rid of the shapes, the user needs to perform specific motions with an STM board that has an accelerometer on it. Machine learning allows the board to identify the user's motions. Serial communication (UART) sends a message from the board to a python file (fall.py) which uses the message as input in the falling shapes game. 

Learned how to use Pygame and PySerial, how to manually configure an STM32 board for UART communication using HAL, and picked up a ton of of C! Also learned a little markdown for this README...

## Background: 
This project was made partially through course materials from the Engr1IT (Internet of Things) course at UCLA. 
There are three main parts to this project:
- ***Machine learning enabled by course materials*** Connecting to an STM32 board (specifically the STM32585AI something something discovery board), the code provided by the course allows users to train motions and then identifies the motions as they are performed. For example, let's say the user wants to train the board to recognize the number 3 written with the board. The user trains the board by writing out 3 with the board segment segment. For example for 3 the user would move the board *right wait down wait left wait right wait down wait left*. After the user performs the "3" motion, the board should print to console that this motion has been identified as "The user has performed motion 3".
- ***Serial communication (the hard part)***. This was original. I manually set up serial communication on the STM board using HAL, essentially slightly abstracted register-level control. I then created a few functions for the board to communicate serially(`sendString()`, `sendChar()`, etc.). The game (coded in python) read the serial message with `serial` from the `pyserial` package.
- ***Falling Shapes game itself***. This was original. The game has shapes falling (squares, triangles, and lines). The user's goal is to keep the shapes from falling. They do this by erasing shapes. They erase shapes by performing the appropriate motion with the board. Once the board identifies the motin, it sends a message over serial to the game indicating one of six fnctions
  - Start game motion performed -> board sends "1!" to game -> Game starts (if game is over or paused)
  - Pause game motion performed -> board sends "2!" to game -> Game pauses (if it was running before)
  - Stop Game motion performed -> board sends "3!" to game -> Game stops (if it was paused or running). The game window closes/quits and the python file terminates
  - Square motion performed -> board sends "4!" to game -> Deletes all squares currently on the screen
  - Triangle motion performed -> board sends "5!" to game -> Deletes all triangles currently on the screen
  - Line motion performed -> board sends "6!" to game -> Deletes all lines currently on the screen

## Executing the game:
- You need an STM board. I'm not sure if you need the specific **STM32585AI something something discovery board** but I think you do because the project should be configured to work with that board.
- You need to know which serial port you are using. Plugging in your STM board, you can figure out which serial port it is connected to by (on MacOS) running `ls /dev/tty.\*`. This will list all serial ports. If you can't tell, unplug your board and notice which port disappears when you run `ls /dev/tty.\*` again. Then put this port name into the appropriate place in the fall.py file
- You can test whether or not the board is sending data to the serial port by running `screen /dev/tty.device_name baud_rate` in terminal (IT IS CRUCIAL THAT THE BAUD RATE MATCHES THE BAUD RATE THE STM BOARD IS CONFIGURED TO). Replace `/dev/tty.device_name` with the name of your serial port. Replace baud_rate in the terminal command with `9600` (or if for some reason you changed the baud rate in the c files, change it to whatever baud rate you configured).
- Download the IOT folder and fall.py folder. Open the IOT folder with the STMCube32 IDE. Open the fall.py folder with your preferred code editor. Configure both appropriately. You shouldn't need to configure anything in the IOT folder. In the fall.py file you need to change the serial port name to the one that works with your device.
- It doesn't really matter in which order you run the files. For simplicitly though, I would reccomend starting with the fall.py file. There is a "handshake" coded into the project that works best if fall.py is run first. I say handshake in quotes because its one-sided, just the board communicating to the python file. The python file has no reason to communicate with the board.

## Notes: 
I made this more to learn than for an "excellent" game. However, for the game to be better (i.e. more playable):
- Modify the motions to be shorter (maybe 3 segments instead of 5).
- Make the speed of the shapes slower or lengthen the game window so the shapes fall for longer.
- Add in a way for motion training to be saved/cached for future runs so motions are consistent and more accurate from game to game
