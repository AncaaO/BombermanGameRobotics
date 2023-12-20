# Pixel Boom Game
An engaging game built on an 8x8 matrix! Navigate through a maze filled with walls, bombs, and a player character in this action-packed adventure.

## Gameplay
The game revolves around maneuvering the player character through a maze while destroying walls. Be cautious of blinking bombs that can end your journey!

Objective: Destroy as many walls as possible without colliding with bombs.

Controls: Navigate the player character using directional buttons.

Features: Sounds for collisions. Difficulty increases gradually for an exciting challenge.

## Task Requirements

### LCD Menu structure:

**1. Intro Message** - When powering up the project, a theme song is played and a greeting message 
is shown for a few moments. <br />

**2. Menu** <br />

&nbsp; **A. Start game** <br />

&nbsp; &nbsp; • Choose level at the beginning <br />
&nbsp; &nbsp; • Starts the game play <br />
&nbsp; &nbsp; • While playing display time <br />
&nbsp; &nbsp; • Screen upon game endind with message when highscore is achieved, switches to another screen after a few moments and displays the score (this state is closed upon interaction) <br />
          
&nbsp; **B. Settings**  <br />

&nbsp; &nbsp; • LCD brightness control  <br />
&nbsp; &nbsp; • Matrix brightness control <br />
&nbsp; &nbsp; • Sound control on/off  <br />

&nbsp; **C. About** - Includes details about the the game: game name, author and github user  <br />

&nbsp; **D. High score** <br />

&nbsp; &nbsp; • Top 3 scores <br />
&nbsp; &nbsp; • Reset high scores button <br />

&nbsp; **D. About** <br />

&nbsp; &nbsp; • Game name <br />
&nbsp; &nbsp; • Developer name <br />
&nbsp; &nbsp; • Github user <br />

&nbsp; **D. How to play** - short description <br />

### Game Requirements

&nbsp; • Theme song ([HiBit documentation](https://www.hibit.dev/posts/62/playing-popular-songs-with-arduino-and-a-buzzer)) <br />
&nbsp; • Sounds upon interaction (bomb explosions) <br />
&nbsp; • Difficulty progresses <br />
&nbsp; • 3 types of elements: player (blinks slowly), bombs (blinks fast), wall (doesn’t blink) <br />
&nbsp; • Reasonables game length <br />

## Used components <br />

• Arduino Uno Board <br />
• Joystick <br />
• 8x8 LED Matrix <br />
• MAX7219 <br />
• LCD Display <br />
• Buzzer <br />
• Button <br />
• Resistors and capacitors <br />
• Breadboard and connecting wires <br />

## Setup

<p float = "left">
<image src = "https://github.com/AncaaO/BombermanGameRobotics/assets/92025959/74a4fb8c-e8c8-42be-a39b-a60661645b7a" width="40%">
</p>

## Link to a video showcasing functionality
[Here!](https://youtu.be/_AxCCm9FrR4)
