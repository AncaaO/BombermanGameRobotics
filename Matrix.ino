#include "LedControl.h" 
#include "pitches.h"
#include <LiquidCrystal.h>
#include <EEPROM.h>

// Pins for Joystick
const int pinSW = 2; // digital pin connected to switch output
const int pinX = A0; // A0 - analog pin connected to X output
const int pinY = A1; // A1 - analog pin connected to Y output

// Pins for LED Matrix
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 13; //10

// Pin for button
const int startButtonPin = A5;

// Pin for buzzer
const int buzzerPin = 3;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); //DIN, CLK, LOAD, No. DRIVER

// Matrix variables
const byte matrixSize = 8;
// byte matrixBrightness = 2;
bool matrixChanged = true;
int wallPercentage;  // Percentage of walls generated on the matrix
int matrix[matrixSize][matrixSize] = {};

// Constants for LED movement based on joystick values
const int thresholdLow = 300;
const int thresholdHigh = 800;

// Variables for joystick
int xValue = 0;
int yValue = 0;
int swState;
bool joyMoved = false;
bool joyMovedY = false, joyMovedX = false;
//bool lastSwState = false;

// Timing variables for movement updates
const int moveInterval = 250;
unsigned long lastMoved = 0;

// Player position variables
static int currentRow = 0;
static int currentCol = 0;
static int lastRow = 0;
static int lastCol = 0;

// Player blink variables
long lastPlayerBlink = 0;
const int playerBlinkDelay = 400;
bool showPlayer = false;
bool lastSwState = false;

// Bomb variables
const int bombBlinkDelay = 100;
unsigned long lastBombBlink = 0;
const int detonationTime = 3000;
unsigned long bombPlantTime = 0;
static int bombRow;
static int bombCol;
bool bombPlanted = false;
bool showBomb = false;

// LCD
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
const byte v0 = 10; //3

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const byte leftArrow[8] = {
  B00000,
  B00100,
  B01000,
  B11111,
  B01000,
  B00100,
  B00000,
  B00000
};

const byte rightArrow[8] = {
  B00000,
  B00100,
  B00010,
  B11111,
  B00010,
  B00100,
  B00000,
  B00000
};

const byte upAndDownArrows[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100
};

const byte downArrow[8] = {
  B00000,
  B00000,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
  B00000
};

const byte upArrow[8] = {
  B00000,
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00000,
  B00000
};

enum menuStates {
  START_GAME, SETTINGS, HIGH_SCORE, ABOUT, HOW_TO_PLAY, LCD_BRIGHTNESS, MATRIX_BRIGHTNESS, SOUND, PLAYING, 
  GAME_OVER, GAME_FINISHED, CHOOSE_LEVEL, LEVEL1, LEVEL2, LEVEL3
} menuState;

enum gameOverStates {
  WALLS_DESTROYED,
  PLAYER_KILLED,
  GAME_CONTINUES
} gameOverState;

// Start button variables
byte startButtonState = HIGH; //pressed
byte startButtonReading = HIGH; //not pressed
byte lastStartButtonReading = HIGH; //not pressed

unsigned int lastDebounceTime = 0;
unsigned int debounceDelay = 50;

//scroll
int scrollDelay = 500;
unsigned long lastScrollTime = 0;

// About menu variables
unsigned long lastDisplayTime = 0;
const int displayDelay = 2000;

// LCD brightness control
int lcdBrightnessValue[5] = { 20, 45, 70, 85, 100 };
int currentLcdBrightnessVal = 0;
const byte firstLcdBrightnessVal = 0;
const byte lastLcdBrightnessVal = 4;

int matrixBrightnessValue[5] = { 1, 3, 5, 9, 11 };
int currentMatrixBrightnessVal = 0;
const byte firstMatrixBrightnessVal = 0;
const byte lastMatrixBrightnessVal = 4;

// levels
int currentLevel;
int levelPlayed;

//score
unsigned long gameStartTime = 0;
unsigned long elapsedTime = 0;
char timeString[4];
int currentScore = 0;
int highScore[3];
int firstPlaceScore = 0;
int secondPlaceScore = 0;
int thirdPlaceScore = 0;
// unsigned long lastScoreDisplayTime = 0;
const int displayScoreDelay = 4000;

// buzzer
int themeSong[] = {
  NOTE_D5, NOTE_AS4,
  NOTE_D5, NOTE_AS4,
  NOTE_F5, NOTE_E5,
  NOTE_DS5, NOTE_B4,
  NOTE_DS5, NOTE_D5, NOTE_CS5,
  NOTE_CS4, NOTE_AS4,
  NOTE_G4
};

int themeSongDurations[] = {
  2, 4,
  2, 4,
  2, 4,
  2, 4,
  4, 8, 4,
  2, 4,
  1
};

int shortSoundDuration = 100;
int themeSongDuration = 7000;
bool songOncePlayed = false;
bool soundState = 1;

void setup() { 
  pinMode(pinSW, INPUT_PULLUP);
  pinMode(startButtonPin, INPUT_PULLUP);

  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.createChar(0, leftArrow);
  lcd.createChar(1, rightArrow);
  lcd.createChar(2, upArrow);
  lcd.createChar(3, downArrow);
  lcd.createChar(4, upAndDownArrows);


  lc.shutdown(0, false); // turn off power saving, enables display
  lc.setIntensity(0, 5); // sets brightness (0~15 possible values)
  lc.clearDisplay(0);// clear screen

  menuState = START_GAME;

  // EEPROM.put(0,0);
  // EEPROM.put(1,0);
  // EEPROM.put(2,0);
  // EEPROM.put(3,0);
  // EEPROM.put(5,0);
  // EEPROM.put(50,0);
  // EEPROM.put(51,0);
  // EEPROM.put(52,0);

  // EEPROM.get(50, firstPlaceScore);
  // EEPROM.get(51, secondPlaceScore); 
  // EEPROM.get(52, thirdPlaceScore); 

  analogWrite(v0, 60);
}

void loop() {

  swState = digitalRead(pinSW);
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  startButtonReading = digitalRead(startButtonPin);

  gameIntro();
  displayMenu();
}

void displayMenu(){
  switch (menuState) {
    case START_GAME:
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
      centerLcdText(F("START GAME"), 0);
      lcd.setCursor(15, 0);
      lcd.write(byte(1));
      centerLcdText(F("Push btn->level"), 1);
      handleMenuChoice();
      break;
    case LEVEL1:
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
      centerLcdText(F("PLAY LEVEL 1"), 0);
      lcd.setCursor(15, 0);
      lcd.write(byte(1));
      centerLcdText(F("Push btn->play"), 1);
      levelPlayed = 1;
      showPlayer = showBomb = false;
      handleMenuChoice();
      break;
    case LEVEL2:
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
      centerLcdText(F("PLAY LEVEL 2"), 0);
      lcd.setCursor(15, 0);
      lcd.write(byte(1));
      centerLcdText(F("Push btn->play"), 1);
      levelPlayed = 2;
      showPlayer = showBomb = false;
      handleMenuChoice();
      break;
    case LEVEL3:
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
      centerLcdText(F("PLAY LEVEL 3"), 0);
      lcd.setCursor(15, 0);
      lcd.write(byte(1));
      centerLcdText(F("Push btn->play"), 1);
      levelPlayed = 3;
      showPlayer = showBomb = false;
      handleMenuChoice();
      break;
    case ABOUT:
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
      if (millis() - lastDisplayTime < displayDelay) {
        centerLcdText(F("ABOUT"), 0);
        centerLcdText(F("PIXEL BOOM"), 1);
      } else if (millis() - lastDisplayTime > displayDelay && millis() - lastDisplayTime < 2 * displayDelay) {
        centerLcdText(F("By Anca Oprea"), 0);
        centerLcdText(F("user: AncaaO"), 1);
      } else {
        lastDisplayTime = millis();
        lcd.clear();
      }
      lcd.setCursor(15, 0);
      lcd.write(byte(1));
      handleMenuChoice();
      break;
    case HOW_TO_PLAY:
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
      lcd.setCursor(1, 0);
      centerLcdText(F(" HOW TO PLAY  "), 0);
      lcd.setCursor(15, 0);
      lcd.write(byte(1));
      scrollText();
      handleMenuChoice();
      break;
    case HIGH_SCORE:
      displayScores();
      handleMenuChoice();
      break;
    case PLAYING: 
      lcd.setCursor(1, 0);
      lcd.print(F("PLAYING"));
      lcd.setCursor(13, 0);
      elapsedTime = (millis() - gameStartTime) / 1000;
      sprintf(timeString, "%03d", elapsedTime);
      lcd.print(timeString);
      centerLcdText(F("Push btn->exit"), 1);

      if (gameOver() == GAME_CONTINUES){
        handleGame(currentLevel);
      } else if (gameOver() == WALLS_DESTROYED){
        menuState = GAME_FINISHED;
        turnOffMatrix();
      } else if (gameOver() == PLAYER_KILLED){
        menuState = GAME_OVER;
        turnOffMatrix();
      }
      handleMenuChoice();
      break;
    case GAME_OVER: 
      lcd.setCursor(0, 0);
      centerLcdText(F("   Game over!   "), 0);
      centerLcdText(F("Push btn->return"), 1);
      handleMenuChoice();
      break;
    case GAME_FINISHED: 
      calculateScore();
      lcd.setCursor(0, 0);
      displayFinishMessage();
      handleMenuChoice();
      break;
    case SETTINGS:
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
      centerLcdText(F("SETTINGS"), 0);
      lcd.setCursor(15, 0);
      lcd.write(byte(1));
      centerLcdText(F("Push btn->adjust"), 1);
      handleMenuChoice();
      break;
    case LCD_BRIGHTNESS:
      lcdBrightness();
      turnOffMatrix();
      handleMenuChoice();
      break;
    case MATRIX_BRIGHTNESS:
      matrixBrightness();
      handleMenuChoice();
      break;
    case SOUND:
      soundControl();
      turnOffMatrix();
      handleMenuChoice();
      break;
  }
}

void handleMenuChoice(){
  // Joystick move - DOWN
  if (xValue < thresholdLow && joyMovedX == false) { 
    switch(menuState) {
      case LCD_BRIGHTNESS:
        currentLcdBrightnessVal++;
        if (currentLcdBrightnessVal > lastLcdBrightnessVal) {
          currentLcdBrightnessVal = lastLcdBrightnessVal;
        }
        lcdBrightness();
        break;
      case MATRIX_BRIGHTNESS:
        currentMatrixBrightnessVal--;
        if (currentMatrixBrightnessVal < firstMatrixBrightnessVal) {
          currentMatrixBrightnessVal = firstMatrixBrightnessVal;
        }
        matrixBrightness();
        break;
      case SOUND:
        soundState = !soundState;
        soundControl();
        break;
    }
    joyMovedX = true;
  }

  // Joystick move - UP
  if (xValue > thresholdHigh && joyMovedX == false) {
    switch(menuState) {
      case LCD_BRIGHTNESS:
        currentLcdBrightnessVal--;
        if (currentLcdBrightnessVal < firstLcdBrightnessVal) {
          currentLcdBrightnessVal = firstLcdBrightnessVal;
        }
        lcdBrightness();
        break;
      case MATRIX_BRIGHTNESS:
        currentMatrixBrightnessVal++;
        if (currentMatrixBrightnessVal > lastMatrixBrightnessVal) {
          currentMatrixBrightnessVal = lastMatrixBrightnessVal;
        }
        matrixBrightness();
        break;
      case SOUND:
        soundState = !soundState;
        soundControl();
        break;
    }
    joyMovedX = true;
  }

  if (xValue >= thresholdLow && xValue <= thresholdHigh) {
    joyMovedX = false;
  }

  // Joystick move - LEFT
  if (yValue < thresholdLow && joyMovedY == false) {
    switch (menuState) {
      case START_GAME:
        navigateMenu(false);
        break;
      case LEVEL1:
        navigateLevelsSubmenu(false);
        break;
      case LEVEL2:
        navigateLevelsSubmenu(false);
        break;
      case LEVEL3:
        navigateLevelsSubmenu(false);
        break;
      case SETTINGS:
        navigateMenu(false);
        break;
      case ABOUT:
        navigateMenu(false);
        break;
      case HIGH_SCORE:
        navigateMenu(false);
        break;
      case HOW_TO_PLAY:
        navigateMenu(false);
        break;
      case LCD_BRIGHTNESS:
        navigateSettingsSubmenu(false);
        break;
      case MATRIX_BRIGHTNESS:
        navigateSettingsSubmenu(false);
        break;
      case SOUND:
        navigateSettingsSubmenu(false);
        break;

    }
    joyMovedY = true;
  }
  // Joystick move - RIGHT
  if (yValue > thresholdHigh && joyMovedY == false) {
    switch (menuState) {
      case START_GAME:
        navigateMenu(true);
        break;
      case LEVEL1:
        navigateLevelsSubmenu(true);
        break;
      case LEVEL2:
        navigateLevelsSubmenu(true);
        break;
      case LEVEL3:
        navigateLevelsSubmenu(true);
        break;
      case SETTINGS:
        navigateMenu(true);
        break;
      case ABOUT:
        navigateMenu(true);
        break;
      case HIGH_SCORE:
        navigateMenu(true);
        break;
      case HOW_TO_PLAY:
        navigateMenu(true);
        break;
      case LCD_BRIGHTNESS:
        navigateSettingsSubmenu(true);
        break;
      case MATRIX_BRIGHTNESS:
        navigateSettingsSubmenu(true);
        break;
      case SOUND:
        navigateSettingsSubmenu(true);
        break;
    }
    joyMovedY = true;
  }

  if (yValue >= thresholdLow && yValue <= thresholdHigh) {
    joyMovedY = false;
  }

  if (startButtonReading != lastStartButtonReading) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay){
    if (startButtonReading !=  startButtonState) {
      startButtonState = startButtonReading;
      if (startButtonState == LOW) {
          handleMenuButtonPress();
      }
    }
  }
  lastStartButtonReading = startButtonReading;
}

void handleMenuButtonPress() {
  switch (menuState) {
    case START_GAME:
      menuState = LEVEL1;
      playScrollSound();
      lcd.clear();
      break;
    case LEVEL1:
      menuState = PLAYING;
      playScrollSound();
      wallPercentage = 50;
      currentLevel = 1;
      gameStartTime = millis();
      startGame();
      lcd.clear();
      break;
    case LEVEL2:
      menuState = PLAYING;
      playScrollSound();
      wallPercentage = 75;
      currentLevel = 2;
      gameStartTime = millis();
      startGame();
      lcd.clear();
      break;
    case LEVEL3:
      menuState = PLAYING;
      playScrollSound();
      wallPercentage = 65;
      currentLevel = 3;
      gameStartTime = millis();
      startGame();
      lcd.clear();
      break;
    case HIGH_SCORE:
      firstPlaceScore = secondPlaceScore = thirdPlaceScore = 0;
      lcd.clear();
      break;
    case SETTINGS:
      menuState = LCD_BRIGHTNESS;
      playScrollSound();
      lcd.clear();
      break;
    case LCD_BRIGHTNESS:
      menuState = SETTINGS;
      playScrollSound();
      lcd.clear();
      break;
    case MATRIX_BRIGHTNESS:
      menuState = SETTINGS;
      playScrollSound();
      turnOffMatrix();
      lcd.clear();
      break;
    case SOUND:
      menuState = SETTINGS;
      playScrollSound();
      lcd.clear();
      break;
    case PLAYING:
      menuState = START_GAME;
      playScrollSound();
      lcd.clear();
      turnOffMatrix();
      break;
    case GAME_OVER:
      menuState = START_GAME;
      playScrollSound();
      lcd.clear();
      break;
    case GAME_FINISHED:
      menuState = START_GAME;
      playScrollSound();
      compareScore();
      lcd.clear();
      break;
  }
}

void navigateMenu(bool right) {
  if (right) {
    if (menuState == HOW_TO_PLAY) {
      menuState = START_GAME;
    } else {
      menuState = menuState + 1;
    }
  } else {
    if (menuState == START_GAME) {
      menuState = HOW_TO_PLAY;
    } else {
      menuState = menuState - 1;
    }
  }
  playScrollSound();
  lcd.clear();
}

void navigateSettingsSubmenu(bool right) {
  if (right) {
    if (menuState == SOUND) {
      menuState = LCD_BRIGHTNESS;
    } else {
      menuState = menuState + 1;
    }
  } else {
    if (menuState == LCD_BRIGHTNESS) {
      menuState = SOUND;
    } else {
      menuState = menuState - 1;
    }
  }
  playScrollSound();
  lcd.clear();
}

void navigateLevelsSubmenu(bool right) {
  if (right) {
    if (menuState == LEVEL3) {
      menuState = LEVEL1;
    } else if (menuState == LEVEL1 || menuState == LEVEL2) {
      menuState = menuState + 1;
    }
    playScrollSound();
  } else {
    if (menuState == LEVEL1) {
      menuState = LEVEL3;
    } else if (menuState == LEVEL3 || menuState == LEVEL2){
      menuState = menuState - 1;
    }
  }
  playScrollSound();
  lcd.clear();
}

void displayScores(){
  lcd.setCursor(0, 0);
  lcd.write(byte(0));
  centerLcdText(F("HIGH SCORES"), 0);
  lcd.setCursor(15, 0);
  lcd.write(byte(1));
  lcd.setCursor(0, 1);  
  if (millis() - lastDisplayTime < displayScoreDelay / 4) {
    lcd.print(F("First place: "));
    printScore(firstPlaceScore);
  } else if (millis() - lastDisplayTime < displayScoreDelay / 2) {
    lcd.print(F("Secnd place: "));
    printScore(secondPlaceScore);
  } else if (millis() - lastDisplayTime < 3 * displayScoreDelay / 4) {
    lcd.print(F("Third place: "));
    printScore(thirdPlaceScore);
  } else if (millis() - lastDisplayTime < displayScoreDelay) {
    centerLcdText(F("Push btn->clear "), 1);
  } else {
    lastDisplayTime = millis();
    lcd.clear();
  }
}

bool compareScore(){
  if (currentScore > firstPlaceScore) {
    thirdPlaceScore = secondPlaceScore;
    secondPlaceScore = firstPlaceScore;
    firstPlaceScore = currentScore;
  } else if (currentScore > secondPlaceScore) {
    thirdPlaceScore = secondPlaceScore;
    secondPlaceScore = currentScore;
  } else if (currentScore > thirdPlaceScore) {
    thirdPlaceScore = currentScore;
  }
}

void printScore(int score){
  int numDigits = (score == 0) ? 1 : (int)log10(abs(score)) + 1;

  lcd.setCursor(13, 1);
  if (numDigits == 1) {
      lcd.print("00");
      lcd.setCursor(15, 1);
      lcd.print(score);
  } else if (numDigits == 2) {
      lcd.print("0");
      lcd.setCursor(14, 1);
      lcd.print(score);
  } else if (numDigits == 3) {
      lcd.print(score);
  }
}

void lcdBrightness() {
  lcd.setCursor(0, 0);
  lcd.print("LCD Brightness");

  analogWrite(v0, lcdBrightnessValue[currentLcdBrightnessVal]);

  lcd.setCursor(15, 0);
  if (currentLcdBrightnessVal == 0) {
    lcd.write(byte(3));
  } else if (currentLcdBrightnessVal == 1) {
    lcd.write(byte(4));
  } else if (currentLcdBrightnessVal == 2) {
    lcd.write(byte(4));
  } else if (currentLcdBrightnessVal == 3) {
    lcd.write(byte(4));
  } else if (currentLcdBrightnessVal == 4) {
    lcd.write(byte(2));
  }

  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  centerLcdText(F("Push btn-exit"), 1); 
  lcd.setCursor(15, 1);
  lcd.write(byte(1));
}

void matrixBrightness() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, true); // turns on all LEDs
    }
  }
  lcd.setCursor(0, 0);
  lcd.print("Matrix Bright.");

  lc.setIntensity(0, matrixBrightnessValue[currentMatrixBrightnessVal]);

  lcd.setCursor(15, 0);
  if (currentMatrixBrightnessVal == 0) {
    lcd.write(byte(2));
  } else if (currentMatrixBrightnessVal == 1) {
    lcd.write(byte(4));
  } else if (currentMatrixBrightnessVal == 2) {
    lcd.write(byte(4));
  } else if (currentMatrixBrightnessVal == 3) {
    lcd.write(byte(4));
  } else if (currentMatrixBrightnessVal == 4) {
    lcd.write(byte(3));
  }

  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  centerLcdText(F("Push btn-exit"), 1); 
  lcd.setCursor(15, 1);
  lcd.write(byte(1));
}

void soundControl(){
  lcd.setCursor(0, 0);
  lcd.print("SOUND: ");
  lcd.setCursor(7, 0);
  if (soundState == 1) {
    lcd.print("ON ");
  } else {
    lcd.print("OFF");
  }
  lcd.setCursor(15, 0);
  lcd.write(byte(4));

  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  centerLcdText(F("Push btn-exit"), 1); 
  lcd.setCursor(15, 1);
  lcd.write(byte(1));

}


void startGame(){
  randomSeed(analogRead(A5));  // Seed random number generator
  currentRow = 0;
  currentCol = 0;
  bombPlanted = false;
  matrix[currentRow][currentCol] = 1; // Turn on the initial LED position
  generateWalls();
}

void handleGame(int level) {
  if (swState == LOW && !bombPlanted) {
    plantBomb();
  }

  if ((millis() - bombPlantTime >= detonationTime) && bombPlanted) {
    if(level == 3){
      detonateBombCross();
      playExplosionSound();
    }
    else {
      detonateBombSquare(); 
      playExplosionSound();
    }
  }

  if (millis() - lastMoved > moveInterval) {
    movePlayer();
    lastMoved = millis(); // Reset the movement timer
  }

  blinkPlayer();
  blinkBomb();

  if (matrixChanged) {
    updateMatrixDisplay();
    matrixChanged = false;
  }
}

void calculateScore(){
  if (levelPlayed == 1){
    currentScore = 100 + (10000/elapsedTime);
  } else if (levelPlayed == 2){
    currentScore = 200 + (10000/elapsedTime);
  } else if (levelPlayed == 3){
    currentScore = 300 + (10000/elapsedTime);
  }
}

void movePlayer() {
  lastRow = currentRow;
  lastCol = currentCol;

  // Move if within bounds and not a wall
  if (xValue < thresholdLow && currentRow < matrixSize - 1) {
    if (matrix[currentRow + 1][currentCol] != 1) {
      currentRow++; 
    }
  } else if (xValue > thresholdHigh && currentRow > 0) {
    if (matrix[currentRow - 1][currentCol] != 1) {
      currentRow--;
    }
  } else if (yValue < thresholdLow && currentCol > 0) {
    if (matrix[currentRow][currentCol - 1] != 1) {
      currentCol--;
    }
  } else if (yValue > thresholdHigh && currentCol < matrixSize - 1) {
    if (matrix[currentRow][currentCol + 1] != 1) {
      currentCol++;
    }
  }

  if (currentRow != lastRow || currentCol != lastCol) {
    matrixChanged = true;
    if (matrix[lastRow][lastCol] != 2) {
      matrix[lastRow][lastCol] = 0; // Turn off the LED at the last position
    }
    matrix[currentRow][currentCol] = 1; // Turn on the LED at the new position
  }
}

void plantBomb() {
  bombPlanted = true;
  bombPlantTime = millis();
  lastBombBlink = millis();
  bombRow = currentRow;
  bombCol = currentCol;
  matrix[bombRow][bombCol] = 2; // Set the bomb at the player's position
  matrixChanged = true;
}

void detonateBombSquare() {
  bombPlanted = false;
  matrix[bombRow][bombCol] = 0; // Remove the bomb
  if (bombRow > 0) {
    matrix[bombRow - 1][bombCol] = 0; // Clear above
  }
  if (bombCol > 0) {
    matrix[bombRow][bombCol - 1] = 0; // Clear to the left
  }
  if (bombRow < matrixSize - 1) {
    matrix[bombRow + 1][bombCol] = 0; // Clear below
  }
  if (bombCol < matrixSize - 1) {
    matrix[bombRow][bombCol + 1] = 0; // Clear to the right
  }
  if (bombRow > 0 && bombCol > 0) {
  matrix[bombRow - 1][bombCol - 1] = 0; // Clear top-left diagonal
  }
  if (bombRow > 0 && bombCol < matrixSize - 1) {
    matrix[bombRow - 1][bombCol + 1] = 0; // Clear top-right diagonal
  }
  if (bombRow < matrixSize - 1 && bombCol > 0) {
    matrix[bombRow + 1][bombCol - 1] = 0; // Clear bottom-left diagonal
  }
  if (bombRow < matrixSize - 1 && bombCol < matrixSize - 1) {
    matrix[bombRow + 1][bombCol + 1] = 0; // Clear bottom-right diagonal
  }
  matrixChanged = true;
}

void detonateBombCross() {
  bombPlanted = false;
  matrix[bombRow][bombCol] = 0; // Remove the bomb
  if (bombRow > 0) {
    matrix[bombRow - 1][bombCol] = 0; // Clear above
  }
  if (bombCol > 0) {
    matrix[bombRow][bombCol - 1] = 0; // Clear to the left
  }
  if (bombRow < matrixSize - 1) {
    matrix[bombRow + 1][bombCol] = 0; // Clear below
  }
  if (bombCol < matrixSize - 1) {
    matrix[bombRow][bombCol + 1] = 0; // Clear to the right
  }
  matrixChanged = true;
}

void generateWalls() {
  byte totalCells = matrixSize * matrixSize;
  byte wallsToGenerate = (totalCells * wallPercentage) / 100;
  
  while (wallsToGenerate > 0) {
    byte x = random(0, matrixSize);
    byte y = random(0, matrixSize);
        
    if ((x >= 2 || y >= 3) && matrix[x][y] == 0) {
      matrix[x][y] = 1;  // 1 represents a wall
      wallsToGenerate--;
    }
  }
}

void updateMatrixDisplay() {
  if(menuState == PLAYING) {
    for (int row = 0; row < matrixSize; row++) {
      for (int col = 0; col < matrixSize; col++) {
        if (matrix[row][col] == 2) {
          lc.setLed(0, row, col, showBomb); // Toggle bomb at the bomb's position
        } else if (row == currentRow && col == currentCol) {
          lc.setLed(0, row, col, showPlayer); // Toggle LED at the player's position
        } else {
          lc.setLed(0, row, col, matrix[row][col]); // Update other LEDs normally
        }
      }
    }
  }

}

void blinkPlayer() {
  if (millis() - lastPlayerBlink >= playerBlinkDelay) {
    matrixChanged = true;
    showPlayer = !showPlayer;
    lastPlayerBlink = millis();
  }
}

void blinkBomb() {
  if (bombPlanted && (millis() - lastBombBlink >= bombBlinkDelay)) {
    showBomb = !showBomb;
    lastBombBlink = millis();
    matrixChanged = true;
  }
}

gameOverStates gameOver(){
  bool wallsDetected = false;

  // Check for remaining walls
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      if (matrix[row][col] == 1 && !(row == currentRow && col == currentCol)) {
        wallsDetected = true;
        break;
      }
    }
    if (wallsDetected) {
      break;
    }
  }

  // Check if the player is near the bomb
  bool playerKilled;
  if (currentLevel == 1 || currentLevel == 2) {
    playerKilled = ((currentRow == bombRow && currentCol == bombCol) ||
                    (currentRow == bombRow - 1 && currentCol == bombCol) || // Above
                    (currentRow == bombRow + 1 && currentCol == bombCol) || // Below
                    (currentRow == bombRow && currentCol == bombCol - 1) || // Left
                    (currentRow == bombRow && currentCol == bombCol + 1) || // Right
                    (currentRow == bombRow - 1 && currentCol == bombCol - 1) || // Top-left diagonal
                    (currentRow == bombRow - 1 && currentCol == bombCol + 1) || // Top-right diagonal
                    (currentRow == bombRow + 1 && currentCol == bombCol - 1) || // Bottom-left diagonal
                    (currentRow == bombRow + 1 && currentCol == bombCol + 1)); // Bottom-right diagonal
  } else if (currentLevel == 3) {
    playerKilled = ((currentRow == bombRow && currentCol == bombCol) ||
                    (currentRow == bombRow - 1 && currentCol == bombCol) || // Above
                    (currentRow == bombRow + 1 && currentCol == bombCol) || // Below
                    (currentRow == bombRow && currentCol == bombCol - 1) || // Left
                    (currentRow == bombRow && currentCol == bombCol + 1)); // Right
  }

  if (!wallsDetected && !playerKilled) {
    return WALLS_DESTROYED;
  } else if (millis() - bombPlantTime >= detonationTime && bombPlanted) {
    if (playerKilled) {
      return PLAYER_KILLED;
    }
  } 
  return GAME_CONTINUES;
}

void turnOffMatrix(){
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, false); // turns off LED at col, row
      matrix[row][col] = 0;
    }
  }
}

void displayFinishMessage() {
  if (millis() - lastDisplayTime < displayDelay) {
    if (currentScore > firstPlaceScore || currentScore > secondPlaceScore || currentScore > thirdPlaceScore){
      lcd.print(F("New High Score!"));
    } else {
      lcd.print(F("    You won!    "));
    }
    centerLcdText(F("Push btn->return"), 1);
  } else if (millis() - lastDisplayTime > displayDelay && millis() - lastDisplayTime < 2 * displayDelay) {
    lcd.print(F(" Your score: "));
    lcd.setCursor(13, 0);
    lcd.print(currentScore);
  } else {
    lastDisplayTime = millis();
    lcd.clear();
  }
}

void scrollText(){
  static const char instructions[] = "Move player->plant bombs->break walls";
  static int scrollPos = 0;

  // Scroll instructions text on the second line
  if (millis() - lastScrollTime > scrollDelay) {
    lastScrollTime = millis();
    lcd.setCursor(0, 1);
    lcd.print("                    ");
    lcd.setCursor(0, 1);
    lcd.print(instructions + scrollPos);
    scrollPos++;
    if (scrollPos >= strlen(instructions)) {
      scrollPos = 0;
    }
  }
}
// Centre LCD text
void centerLcdText (String msg, short line) {
  short offset = (16 - msg.length())/2;
  lcd.setCursor(offset, line);
  lcd.print(msg);
}

void gameIntro(){
  int size = sizeof(themeSongDurations) / sizeof(int);
  if (!songOncePlayed && soundState == 1){
    for (int note = 0; note < size; note++) {
      //to calculate the note duration, take one second divided by the note type.
      int duration = 1000 / themeSongDurations[note];
      tone(buzzerPin, themeSong[note], duration);
      int pauseBetweenNotes = duration * 1.30;

      if (millis() < themeSongDuration / 2) {
          lcd.clear();
          centerLcdText(F("WELCOME TO"), 0);
          centerLcdText(F("PIXEL BOOM"), 1);
      } else if (millis() > themeSongDuration / 2 && millis() < themeSongDuration) {
            lcd.clear();
            centerLcdText(F("READY TO FIGHT"), 0);
            centerLcdText(F("THE WALLS?"), 1);

      }
      delay(pauseBetweenNotes);
      noTone(buzzerPin);
    }
    lcd.clear();
    songOncePlayed = true;
  }
}

void playScrollSound() {
  if (soundState == 1){
    tone(buzzerPin, NOTE_C8, shortSoundDuration); 
  }
}

void playExplosionSound() {
  if (soundState == 1){
    tone(buzzerPin, NOTE_A1, shortSoundDuration );
  }
}