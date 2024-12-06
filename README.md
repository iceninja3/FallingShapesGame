## [Presentation Link](https://docs.google.com/presentation/d/1nt9gRMgprMrjdH6lQ8kB16Lm4ZPe6Ehf_eqI9vMX2Y4/edit#slide=id.g31c968ffbe4_2_91)
## [Video Demo Link](https://www.youtube.com/watch?v=UFumihdRavY)

## Background: 
This project was made partially through course materials from the Engr1IT (internet of things) course at UCLA. There are three main components to this project
- Machine learning enabled by course materials. Connecting to an STM32 board (specifically the STM32585AI something something discovery board), this component allows users to train motions and then identifies the motions as they are performed. For example, let's say the user wants to train the board to recognize the number 3 written with the board. The user trains the board by writing out 3 with the board (segment by segment so like right wait down wait left wait right wait down wait left). If the user then performs this motion (writing out 3), the board should print to console that this motion has been identified as "whatever the user named this motion".
- Serial communication (the hard part). This was original. I manually set up serial communication on the STM board and then created a few functions (sendString(), sendChar(), etc.). I used HAL and UART here. The game (coded in python) read the serial with serial from the pyserial package.
- Falling Shapes game. This was original. The game has shapes falling (squares, triangles, and lines). The user's goal is to keep the shapes from falling. They do this by erasing shapes. They erase shapes by performing the appropriate motion with the board. Once the board identifies the motin, it sends a message over serial to the game indicating one of six fnctions
  - Start game motion performed -> board sends "1!" to game -> Game starts (if game is over or paused)
  - Pause game motion performed -> board sends "2!" to game -> Game pauses (if it was running before)
  - Stop Game motion performed -> board sends "3!" to game -> Game stops (if it was paused or running). The game window closes/quits and the python file terminates
  - Square motion performed -> board sends "4!" to game -> Deletes all squares currently on the screen
  - Triangle motion performed -> board sends "5!" to game -> Deletes all triangles currently on the screen
  - Triangle motion performed -> board sends "6!" to game -> Deletes all lines currently on the screen

## Executing the game:
- You need an STM board (I'm not sure if you need the specific STM32585AI something something discovery board but I think you do because the project should be configured to work with that board).
- You need to know which serial port you are using. Plugging in your STM board, you can figure out which serial port it is connected to by (on MacOS), running "ls /dev/tty.\*" will list all serial ports. If you can't tell, unplug your board and notice which port disappears when you run "ls /dev/tty.\*" again. Then put this port name into the appropriate place in the fall.py file
- You can test whether or not the board is sending data to the serial port by running "screen /dev/tty.device_name baud_rate" in terminal (IT IS CRUCIAL THAT THE BAUD RATE MATCHES THE BAUD RATE THE STM BOARD IS CONFIGURED TO). Replace /dev/tty.device_name with the name of your serial port. Replace baud_rate with 9600 (or if for some reason you changed the baud rate in the c files, change it to whatever baud rate that is).
- Download the IOT folder and fall.py folder. Open the IOT folder with the STMCube32 IDE. Open the fall.py folder with your preferred code editor. Configure both appropriately (you shouldn't need to configure anything in the IOT folder. in the fall.py file you need to change the serial port name)
- It doesn't really matter in which order you run the files. I would reccomend starting with the fall.py file though for simplicity. There is a "handshake" coded into the project that works best if fall.py is run first.

## Notes: 
- I made this more to learn than for an "excellent" game. However, for the game to be better (i.e. more playable) I would modify the motions to be shorter (maybe 3 segments instead of 5). Also, I would make the speed of the shapes slower or lengthen the game window so the shapes fall for longer.
- 

