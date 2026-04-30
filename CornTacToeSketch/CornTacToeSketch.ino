#include <FastLED.h>

#define LED_PIN 11
#define NUM_SENSORS 5
#define NUM_LEDS 9
int ledMap[NUM_LEDS] = {
  6,  // 0 -> 6
  7,  // 1 -> 7
  8,  // 2 -> 8
  5,  // 3 -> 5
  4,  // 4 -> 4
  3,  // 5 -> 3
  0,  // 6 -> 0
  1,  // 7 -> 1
  2   // 8 -> 2
};

#define IR_RECEIVER0 2
#define IR_RECEIVER1 3
#define IR_RECEIVER2 4
#define IR_RECEIVER3 6
#define IR_RECEIVER4 5

#define PLAY 0
#define SHOT_DETECT 1
#define MADE_SHOT 2
#define MISSED_SHOT 3
#define WINNING 4
#define NEW_ROUND 5

#define VIBRATION_SENSOR 7

#define BUZZER 10

#define SOUNDS_ON false

CRGB X_COLOR = CRGB::Red;
CRGB O_COLOR = CRGB::Blue;
CRGB NEUTRAL_COLOR = CRGB::White;

unsigned long currentTime;

#define PLAY_BLINK_DELAY 400
unsigned long playBlinkTimer;
CRGB playBlinkColor = NEUTRAL_COLOR;

#define SHOT_DETECT_DELAY 600
unsigned long shotDetectTimer;

#define MADE_SHOT_DELAY 800
#define MADE_SHOT_BLINK_DELAY 100
#define INVALID 255
byte winningLine[] = { INVALID, INVALID, INVALID };
byte madeShotHole = INVALID;
unsigned long madeShotTimer;
unsigned long madeShotBlinkTimer;
CRGB madeShotBlinkColor = NEUTRAL_COLOR;

#define MISSED_SHOT_DELAY 1000
#define MISSED_SHOT_BLINK_DELAY 200
unsigned long missedShotTimer;
unsigned long missedShotBlinkTimer;
CRGB missedShotBlinkColor = NEUTRAL_COLOR;

#define WINNING_DELAY 6000
#define WINNING_STAGE_DELAY 2000
#define WINNING_BLINK_DELAY 100
unsigned long winningTimer;
unsigned long winningBlinkTimer;
unsigned long winningStageTimer;
CRGB winningBlinkColor = NEUTRAL_COLOR;

#define TURN_DELAY 6000
#define ROUND_LENGTH 8
unsigned long turnTimer;
byte turn = ROUND_LENGTH;

#define NEW_ROUND_DELAY 20000
#define NEW_ROUND_STAGES 9
#define NEW_ROUND_STAGE_DELAY (NEW_ROUND_DELAY / (NEW_ROUND_STAGES*3))
byte newRoundBlinkStage = 0;
unsigned long newRoundTimer;
unsigned long newRoundBlinkTimer;

int irReceivers[NUM_SENSORS] = {
  IR_RECEIVER0,
  IR_RECEIVER1,
  IR_RECEIVER2,
  IR_RECEIVER3,
  IR_RECEIVER4
};


CRGB leds[NUM_LEDS];
char board[9] = {};

bool IR_triggered_round[NUM_SENSORS] = {};
char currentPlayer = 'x';

int playState;

void setup() {
  Serial.begin(9600);
  //IRSENSORS
  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(irReceivers[i], INPUT);
  }
  pinMode(VIBRATION_SENSOR, INPUT);
  if(SOUNDS_ON){
    pinMode(BUZZER, OUTPUT);
  }
  //LEDS
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
    board[i] = 0;
  }
  for (int i = 0; i < NUM_SENSORS; i++) {
    IR_triggered_round[i] = 0;
  }
  FastLED.show();
  currentTime = millis();
  play();
}

void loop() {
  currentTime = millis();
  //Blink Animation to indicate currentPlayer
  if (playState == PLAY) {
    if (currentTime - playBlinkTimer > PLAY_BLINK_DELAY) {
      if (playBlinkColor == NEUTRAL_COLOR) {
        playBlinkColor = currentPlayer == 'x' ? X_COLOR : O_COLOR;
      } else {
        playBlinkColor = NEUTRAL_COLOR;
      }
      playBlinkTimer = currentTime;
    }
    if (currentTime - turnTimer > TURN_DELAY) {
      missedShot();
      turnTimer = currentTime;
    }
    if (detectTriggers() || digitalRead(VIBRATION_SENSOR)) {
      shotDetect();
    }
  } else if (playState == SHOT_DETECT) {
    if (detectTriggers()) {
      // Serial.print("2nd beam after ");
      // Serial.print(currentTime - shotDetectTimer);
      // Serial.println("ms");
    }
    if (shotDetectTimer && currentTime - shotDetectTimer > SHOT_DETECT_DELAY) {
      if (resolveHole()) {
        madeShot();
      } else {
        missedShot();
      }
      shotDetectTimer = 0;
    }
  } else if (playState == MADE_SHOT) {
    if (currentTime - madeShotTimer > MADE_SHOT_DELAY) {
      if (checkWin()) {
        winning();
      } else {
        play();
      }
      madeShotHole = INVALID;
      madeShotTimer = 0;
      madeShotBlinkTimer = 0;
      madeShotBlinkColor = NEUTRAL_COLOR;
    } else if (currentTime - madeShotBlinkTimer > MADE_SHOT_BLINK_DELAY) {
      if (madeShotBlinkColor == NEUTRAL_COLOR) {
        madeShotBlinkColor = currentPlayer == 'x' ? X_COLOR : O_COLOR;
      } else {
        madeShotBlinkColor = NEUTRAL_COLOR;
      }
      madeShotBlinkTimer = currentTime;
    }
  } else if (playState == MISSED_SHOT) {
    if (currentTime - missedShotTimer > MISSED_SHOT_DELAY) {
      play();
    } else if (currentTime - missedShotBlinkTimer > MISSED_SHOT_BLINK_DELAY) {
      missedShotBlinkColor = missedShotBlinkColor == NEUTRAL_COLOR ? CRGB::Black : NEUTRAL_COLOR;
      missedShotBlinkTimer = currentTime;
    }
  } else if (playState == NEW_ROUND) {
    if (currentTime - newRoundTimer > NEW_ROUND_DELAY) {
      play();
    } else if (currentTime - newRoundBlinkTimer > NEW_ROUND_STAGE_DELAY) {
      newRoundBlinkStage = (newRoundBlinkStage + 1) % NEW_ROUND_STAGES;
      newRoundBlinkTimer = currentTime;
    }
  } else if (playState == WINNING) {
    if (currentTime - winningTimer > WINNING_DELAY) {
      winningTimer = 0;
      winningBlinkTimer = 0;
      setWinningLine(INVALID, INVALID, INVALID);
      resetBoard();
      play();
    } else if (currentTime - winningBlinkTimer > WINNING_BLINK_DELAY) {
      if (winningBlinkColor == NEUTRAL_COLOR) {
        winningBlinkColor = currentPlayer == 'x' ? X_COLOR : O_COLOR;
      } else {
        winningBlinkColor = NEUTRAL_COLOR;
      }
      winningBlinkTimer = currentTime;
    }
  }
  renderBoard();
  FastLED.show();
}

void resetBoard() {
  for (int i = 0; i < NUM_LEDS; i++) {
    board[i] = 0;
    leds[i] = 0;
  }
  turn = ROUND_LENGTH-1;
}

bool detectTriggers() {
  bool anyTriggered = false;
  for (int i = 0; i < NUM_SENSORS; i++) {
    bool triggered = digitalRead(irReceivers[i]);  // ACTIVE LOW
    if (!triggered && !IR_triggered_round[i]) {
      anyTriggered = true;
      // Serial.print("IR ");
      // Serial.print(i);
      // Serial.println(" triggered");
      IR_triggered_round[i] = true;
    }
  }
  return anyTriggered;
}

void play() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    IR_triggered_round[i] = 0;
  }

  if(turn!= ROUND_LENGTH){
    currentPlayer = currentPlayer == 'x' ? 'o' : 'x';
  }
    playBlinkColor = currentPlayer == 'x' ? X_COLOR : O_COLOR;
  playBlinkTimer = currentTime;

  turnTimer = currentTime;
  turn = (turn + 1) % (ROUND_LENGTH + 1);

  if (turn == ROUND_LENGTH) {
    newRound();
  } else {
    playState = PLAY;
  }
  // Serial.println("Starting new turn");
}

void shotDetect() {
  shotDetectTimer = currentTime;
  playState = SHOT_DETECT;
}

void madeShot() {
  madeShotTimer = currentTime;
  madeShotBlinkTimer = currentTime;
  playState = MADE_SHOT;
}

void missedShot() {
  missedShotTimer = currentTime;
  missedShotBlinkTimer = currentTime;
  missedShotBlinkColor = NEUTRAL_COLOR;
  playState = MISSED_SHOT;
}

void newRound() {
  playState = NEW_ROUND;
  newRoundBlinkTimer = currentTime;
  newRoundTimer = currentTime;
  newRoundBlinkStage = 0;
}

void winning() {
  winningTimer = currentTime;
  winningBlinkTimer = currentTime;
  winningStageTimer = currentTime;
  playState = WINNING;
}

void renderBoard() {
  // hole -> LED mapping
  for (int i = 0; i < NUM_LEDS; i++) {
    int ledIndex = ledMap[i];
    if (board[i] == 'x') {
      leds[ledIndex] = X_COLOR;
    } else if (board[i] == 'o') {
      leds[ledIndex] = O_COLOR;
    } else {
      leds[ledIndex] = playBlinkColor;
    }
  }
  if (playState == MADE_SHOT) {
    leds[ledMap[madeShotHole]] = madeShotBlinkColor;
  } else if (playState == WINNING) {
    if (currentTime - winningStageTimer < WINNING_STAGE_DELAY) {
      leds[ledMap[winningLine[0]]] = winningBlinkColor;
      leds[ledMap[winningLine[1]]] = winningBlinkColor;
      leds[ledMap[winningLine[2]]] = winningBlinkColor;
    } else {
      for (int i = 0; i < 9; i++) {
        leds[i] = winningBlinkColor;
      }
    }
  } else if (playState == MISSED_SHOT) {
    for (int i = 0; i < 9; i++) {
      if (missedShotBlinkColor != NEUTRAL_COLOR) {
        leds[i] = CRGB::Black;
      }
    }
  } else if (playState == NEW_ROUND) {
    byte led1 = newRoundBlinkStage == 0 ? 8 : newRoundBlinkStage - 1;
    byte led2 = newRoundBlinkStage;
    byte led3 = newRoundBlinkStage == 8 ? 0 : newRoundBlinkStage + 1;
    leds[ledMap[led1]] = CRGB::Black;
    leds[ledMap[led2]] = CRGB::Black;
    leds[ledMap[led3]] = CRGB::Black;
  }
}

bool resolveHole() {
  int hole = -1;
  // 0 0 1 2
  // 1 3 4 5
  // 2 6 7 8
  //.  - 3 4
  if (IR_triggered_round[0] && IR_triggered_round[3])
    hole = 1;
  else if (IR_triggered_round[0] && IR_triggered_round[4])
    hole = 2;
  else if (IR_triggered_round[0])
    hole = 0;
  else if (IR_triggered_round[1] && IR_triggered_round[3])
    hole = 4;
  else if (IR_triggered_round[1] && IR_triggered_round[4])
    hole = 5;
  else if (IR_triggered_round[1])
    hole = 3;
  else if (IR_triggered_round[2] && IR_triggered_round[3])
    hole = 7;
  else if (IR_triggered_round[2] && IR_triggered_round[4])
    hole = 8;
  else if (IR_triggered_round[2])
    hole = 6;

  if (hole == -1) {
    // Serial.println("Error in resolveHole");
    return false;
  }

  // Serial.print("Hole: ");
  // Serial.print(hole);
  // Serial.print(", Scored by: ");
  // Serial.println(currentPlayer);
  board[hole] = currentPlayer;
  madeShotHole = hole;
  return true;
}

bool checkWin() {
  setWinningLine(INVALID, INVALID, INVALID);
  if (board[4]) {
    if (board[3] == board[4] && board[4] == board[5])
      setWinningLine(3, 4, 5);
    if (board[1] == board[4] && board[4] == board[7])
      setWinningLine(1, 4, 7);
    if (board[0] == board[4] && board[4] == board[8])
      setWinningLine(0, 4, 8);
    if (board[2] == board[4] && board[4] == board[6])
      setWinningLine(2, 4, 6);
  }
  if (board[0]) {
    if (board[0] == board[1] && board[1] == board[2])
      setWinningLine(0, 1, 2);
    if (board[0] == board[3] && board[3] == board[6])
      setWinningLine(0, 3, 6);
  }
  if (board[8]) {
    if (board[6] == board[7] && board[7] == board[8])
      setWinningLine(6, 7, 8);
    if (board[2] == board[5] && board[5] == board[8])
      setWinningLine(2, 5, 8);
  }
  for (int i = 0; i < 3; i++) {
    if (winningLine[i] != INVALID) {
      return true;
    }
  }
  return false;
}

void setWinningLine(byte hole1, byte hole2, byte hole3) {
  winningLine[0] = hole1;
  winningLine[1] = hole2;
  winningLine[2] = hole3;
}