/*
Elliot Lard
Corn Tac Toe
*/
#include <FastLED.h>

//sensors
#define SWITCH0 0
#define SWITCH1 1
#define SWITCH2 2
#define SWITCH3 3
#define SWITCH4 4
#define SWITCH5 5
#define SWITCH6 6
#define SWITCH7 7
#define SWITCH8 8
#define VIBRATIONSENSOR0 13

//outputs
#define SPEAKER 12
#define LEDS0 9
#define LEDS1 10
#define LEDS2 11

//game constants
#define TURNDELAY 500
#define TIMEOUT 8000
#define NUM_LEDS 3
#define BLINKDELAY 500
#define WINDELAY 15000

// function prototypes
void lightGameboard();
void lightLED(int i);
void setLED(int i, CRGB color);

void clearBoard();
void reset();
void win(char winner);

void takenShot(int space);
void madeShot(int space);

void turnTimeout();
void newTurn();
void turnChangeDelay();

bool detectVibrations();
void detectTriggers();


char handleWin(int a, int b, int c);
char checkWin();

CRGB leds0[NUM_LEDS];
CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];

char currentPlayer = 'x';  //x is RED, o is BLUE
short int bagsThrownThisRound = 0;

//timers
unsigned long turnChangeDelayTimer = 0;
unsigned long turnStartTimer = 0;
unsigned long blinkTimer;

unsigned long winTimer = 0;
int winCells[3] = { -1, -1, -1 };
char winPlayer = 0;

//turn switch animation
bool turnSwitchAnimation = false;
int turnBlinkCount = 0;
unsigned long turnBlinkTimer = 0;
bool turnBlinkState = false;

//win animation
bool winAnimation = false;
int winPhase = 0;
int winBlinkCount = 0;
bool winBlinkState = false;
unsigned long winAnimTimer = 0;

CRGB blinkingColor = CRGB::White;

char gameboard[9] = { 0 };
bool triggered[9] = { 0 };
bool lastTriggered[9] = { 0 };

int switches[] = {
  SWITCH0, SWITCH1, SWITCH2,
  SWITCH3, SWITCH4, SWITCH5,
  SWITCH6, SWITCH7, SWITCH8
};

void setup() {
  //inputs
  pinMode(SWITCH0, INPUT);
  pinMode(SWITCH1, INPUT);
  pinMode(SWITCH2, INPUT);
  pinMode(SWITCH3, INPUT);
  pinMode(SWITCH4, INPUT);
  pinMode(SWITCH5, INPUT);
  pinMode(SWITCH6, INPUT);
  pinMode(SWITCH7, INPUT);
  pinMode(SWITCH8, INPUT);
  pinMode(VIBRATIONSENSOR0, INPUT);

  //outputs
  pinMode(SPEAKER, OUTPUT);
  Serial.begin(9600);

  FastLED.addLeds<WS2812, LEDS0, GRB>(leds0, NUM_LEDS);
  FastLED.addLeds<WS2812, LEDS1, GRB>(leds1, NUM_LEDS);
  FastLED.addLeds<WS2812, LEDS2, GRB>(leds2, NUM_LEDS);
  FastLED.setBrightness(100);
  blinkTimer = millis();
}

void loop() {
  if (winAnimation) {
    unsigned long delayTime;

    if (winPhase == 0) {
      delayTime = 100;  // fast blink for winning tiles
    } else {
      delayTime = 200;  // slower blink for whole board
    }

    if (millis() - winAnimTimer > delayTime) {
      winBlinkState = !winBlinkState;
      winAnimTimer = millis();

      if (!winBlinkState) {  // count full blink cycles
        winBlinkCount++;
      }

      if (winPhase == 0 && winBlinkCount >= 3) {
        winPhase = 1;
        winBlinkCount = 0;
      } else if (winPhase == 1 && winBlinkCount >= 3) {
        winPhase = 2;
        winAnimation = false;  // enter final state
      }
    }
  }
  if (turnSwitchAnimation) {                //turn switch animation
    if (millis() - turnBlinkTimer > 150) {  // speed of blink
      turnBlinkState = !turnBlinkState;
      turnBlinkTimer = millis();

      if (!turnBlinkState) {  // count full blink cycles
        turnBlinkCount++;
      }

      if (turnBlinkCount >= 5) {
        turnSwitchAnimation = false;
        blinkingColor = CRGB::White;
        blinkTimer = millis();
      }
    }
  }
  if (millis() - blinkTimer > BLINKDELAY) {  // turn indicator blinking
    if (blinkingColor == CRGB::White) {
      blinkingColor = currentPlayer == 'x' ? CRGB::Red : CRGB::Blue;
    } else {
      blinkingColor = CRGB::White;
    }
    blinkTimer = millis();
  }
  if (winTimer && millis() - winTimer > WINDELAY) {
    winTimer = 0;
    reset();
  }
  if (!winTimer) {
    //delay before turn change
    if (turnChangeDelayTimer && millis() - turnChangeDelayTimer >= TURNDELAY) {
      newTurn();
    }
    //check triggers and update gamestate
    detectTriggers();

    for (int i = 0; i < 9; i++) {
      if (triggered[i]) {
        if (!gameboard[i]) {
          madeShot(i);
        } else {
          takenShot(i);
        }
        if (!turnChangeDelayTimer) {  //start a timer for delay before turn change
          turnChangeDelay();
        }
      }
    }
    //check for a win and carry out the win
    char winner = checkWin();
    if (winner) {
      win(winner);
    }
    //detect bag hits
    if (!turnChangeDelayTimer && detectVibrations()) {  //start a timer for delay before turn change
      turnChangeDelay();
    }
    if (turnStartTimer && millis() - turnStartTimer > TIMEOUT) {
      turnTimeout();
      newTurn();
    }
  }
  lightGameboard();
  FastLED.show();
}

void lightGameboard() {
  for (int i = 0; i < 9; i++) {
    lightLED(i);
  }
}

void lightLED(int i) {
  CRGB color = CRGB::Black;

  // --- WIN STATE ---
  if (winTimer) {
    CRGB winColor = (winPlayer == 'x') ? CRGB::Red : CRGB::Blue;
    bool isWinCell = (i == winCells[0] || i == winCells[1] || i == winCells[2]);

    if (winAnimation && winPhase == 0) {
      // Phase 0: only winning tiles blink
      color = (isWinCell && winBlinkState) ? winColor : CRGB::Black;
    } else if (winAnimation && winPhase == 1) {
      // Phase 1: whole board blinks
      color = winBlinkState ? winColor : CRGB::Black;
    } else {
      // Phase 2: only winning tiles stay on
      color = isWinCell ? winColor : CRGB::Black;
    }
  }

  // --- TURN SWITCH ANIMATION ---
  else if (turnSwitchAnimation) {
    if (gameboard[i] == 'x') {
      color = CRGB::Red;
    } else if (gameboard[i] == 'o') {
      color = CRGB::Blue;
    } else {
      color = turnBlinkState ? CRGB::White : CRGB::Black;
    }
  }

  // --- NORMAL GAME STATE ---
  else {
    if (gameboard[i] == 'x') {
      color = CRGB::Red;
    } else if (gameboard[i] == 'o') {
      color = CRGB::Blue;
    } else {
      color = blinkingColor;
    }
  }

  // Single output point
  setLED(i, color);
}

void setLED(int i, CRGB color) {
  switch (i) {
    case 0: leds0[0] = color; break;
    case 1: leds0[1] = color; break;
    case 2: leds0[2] = color; break;
    case 3: leds1[0] = color; break;
    case 4: leds1[1] = color; break;
    case 5: leds1[2] = color; break;
    case 6: leds2[0] = color; break;
    case 7: leds2[1] = color; break;
    case 8: leds2[2] = color; break;
  }
}

void clearBoard() {
  for (int i = 0; i < 9; i++) {
    gameboard[i] = 0;
  }
}

void reset() {
  Serial.println("Starting new Game");

  // core game state
  clearBoard();
  currentPlayer = 'x';
  bagsThrownThisRound = 0;

  // timers
  turnChangeDelayTimer = 0;
  turnStartTimer = 0;
  blinkTimer = millis();
  winTimer = 0;

  // blinking state
  blinkingColor = CRGB::White;

  // turn animation reset
  turnSwitchAnimation = false;
  turnBlinkCount = 0;
  turnBlinkTimer = 0;
  turnBlinkState = false;

  // win animation reset
  winAnimation = false;
  winPhase = 0;
  winBlinkCount = 0;
  winBlinkState = false;
  winAnimTimer = 0;

  // win tracking
  winCells[0] = -1;
  winCells[1] = -1;
  winCells[2] = -1;
  winPlayer = 0;

  // clear triggers (prevents ghost inputs)
  for (int i = 0; i < 9; i++) {
    triggered[i] = false;
  }
}

void win(char winner) {
  winTimer = millis();
  Serial.print("game won by ");
  Serial.println(winner);

  turnChangeDelayTimer = 0;
  bagsThrownThisRound = 0;
  turnStartTimer = 0;
  blinkTimer = 0;

  winAnimation = true;
  winPhase = 0;
  winBlinkCount = 0;
  winBlinkState = false;
  winAnimTimer = millis();
}

void takenShot(int space) {
  Serial.print("That space was already taken: ");
  Serial.println(space);
  //output feedback here for a space already taken
}

void madeShot(int space) {
  gameboard[space] = currentPlayer;
  Serial.println("You got one: ");
  Serial.println(space);
  //output feedback here for a space already taken
}

void turnTimeout() {
  Serial.println("Well, you miseed or ya waited too long");
  //output feedback for a timeout
}

void newTurn() {
  turnChangeDelayTimer = 0;
  if (bagsThrownThisRound >= 5) {  //start new round
    turnStartTimer = 0;
    bagsThrownThisRound = 0;
  } else {
    bagsThrownThisRound++;
    turnStartTimer = millis();
  }
  currentPlayer = currentPlayer == 'x' ? 'o' : 'x';  //switch turns
  Serial.println("Starting new turn");
  turnSwitchAnimation = true;
  turnBlinkCount = 0;
  turnBlinkTimer = millis();
  turnBlinkState = false;
}

void turnChangeDelay() {
  turnChangeDelayTimer = millis();
  turnStartTimer = millis();
}

bool detectVibrations() {
  return digitalRead(VIBRATIONSENSOR0);
}

char handleWin(int a, int b, int c) {
  char winner = gameboard[a];

  winCells[0] = a;
  winCells[1] = b;
  winCells[2] = c;
  winPlayer = winner;

  clearBoard();
  gameboard[a] = winner;
  gameboard[b] = winner;
  gameboard[c] = winner;

  return winner;
}

char checkWin() {

  if (gameboard[4]) {
    if (gameboard[3] == gameboard[4] && gameboard[4] == gameboard[5])
      return handleWin(3, 4, 5);

    if (gameboard[1] == gameboard[4] && gameboard[4] == gameboard[7])
      return handleWin(1, 4, 7);

    if (gameboard[0] == gameboard[4] && gameboard[4] == gameboard[8])
      return handleWin(0, 4, 8);

    if (gameboard[2] == gameboard[4] && gameboard[4] == gameboard[6])
      return handleWin(2, 4, 6);
  }

  if (gameboard[0]) {
    if (gameboard[0] == gameboard[1] && gameboard[1] == gameboard[2])
      return handleWin(0, 1, 2);

    if (gameboard[0] == gameboard[3] && gameboard[3] == gameboard[6])
      return handleWin(0, 3, 6);
  }

  if (gameboard[8]) {
    if (gameboard[6] == gameboard[7] && gameboard[7] == gameboard[8])
      return handleWin(6, 7, 8);

    if (gameboard[2] == gameboard[5] && gameboard[5] == gameboard[8])
      return handleWin(2, 5, 8);
  }

  return 0;
}

void detectTriggers() {
  for (int i = 0; i < 9; i++) {
    bool current = digitalRead(switches[i]);
    triggered[i] = current && !lastTriggered[i];  // rising edge only
    lastTriggered[i] = current;
  }
}