# Corn-Tac-Toe 🎯

A physical tic-tac-toe game played with cornhole bags, featuring IR beam sensing, LED feedback, turn-based game logic, and animated win effects.

Corn-Tac-Toe was awarded **3rd place overall** in the Louisiana Tech Freshman Design Competition.

---

## 🎮 Overview

Corn-Tac-Toe combines the gameplay of tic-tac-toe with the physical challenge of cornhole. Players take turns tossing bags at a 3×3 board, trying to claim spaces and get three in a row.

The board uses IR break-beam sensors to detect which hole a bag passes through, then updates addressable LEDs to show player ownership, turn status, missed shots, round transitions, and winning animations.

---

## ✨ Features

- 🔴🔵 Turn-based gameplay: Red vs. Blue
- 🎯 Sensor-based input detection
- ⏱ Turn timeout handling
- 💡 Addressable LED feedback for game state
- 🏆 Animated win sequence
- ⚡ Non-blocking timing using `millis()`
- 🧠 Finite state machine for organized game flow
- 🔊 Optional sound feedback with buzzer tones

---

## 🧰 Hardware

- Arduino Uno
- WS2812B addressable LED strip
- 5 IR break-beam sensor pairs
- Vibration sensor for missed-shot / board-impact detection
- Buzzer or speaker for sound effects
- Custom 3×3 wooden Corn-Tac-Toe board
- External power supply for LEDs

---

## 🧠 How It Works

Corn-Tac-Toe uses a combination of horizontal and vertical IR beams to determine which hole a bag passes through.

Instead of placing a separate sensor inside every hole, the board uses:

- 3 horizontal row beams
- 2 vertical column beams
- Left-column inference when no vertical beam is broken

When a bag breaks the beams, the Arduino records the triggered sensors, resolves the hole location, updates the board state, and changes the LEDs to match the current game state.

---

## 💡 LED Feedback

The LEDs provide real-time feedback to players:

- **Red LEDs** represent spaces claimed by the X player
- **Blue LEDs** represent spaces claimed by the O player
- **White / blinking LEDs** indicate available spaces and current turn
- **Made shots** blink the scored hole
- **Missed shots** briefly flash the board dark
- **Winning sequences** blink the winning line, then the full board
- **Idle states** use simple animations during pauses and round transitions

---

## 🔁 Game Logic

The program is organized around a finite state machine with states such as:

- `TAKE_5`
- `PLAY`
- `SHOT_DETECT`
- `MADE_SHOT`
- `MISSED_SHOT`
- `WINNING`
- `NEW_ROUND`
- `CLEAR_BOARD`

This keeps gameplay, sensor detection, LED animation, missed-shot handling, and win behavior organized into clear state-based sections.

---

## ⏱ Non-Blocking Timing

The game uses `millis()`-based timing instead of long blocking delays. This allows the Arduino to continue monitoring sensors while LED animations, turn timers, and feedback effects are running.

Example pattern:

```cpp
currentTime = millis();

if (currentTime - timer > interval) {
    // update animation or change state
}
