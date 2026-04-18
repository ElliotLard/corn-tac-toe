# Corn Tac Toe 🎯

A physical tic-tac-toe game played with cornhole bags, featuring LED feedback, turn-based logic, and animated win effects.

## 🎮 Overview

Corn Tac Toe is a hybrid of cornhole and tic-tac-toe. Players take turns tossing bags onto a 3x3 board, trying to claim spaces and get three in a row.

The board uses sensors to detect hits and LED strips to display game state in real time.

---

## ✨ Features

- 🔴🔵 Turn-based gameplay (Red vs Blue)
- ⏱ Turn timeout handling
- 💡 LED board with:
  - Player-indicating blinking
  - Turn-switch animation
  - Multi-phase win animation:
    - Winning tiles flash
    - Full-board celebration
    - Final highlight state
- 🎯 Sensor-based input detection (no buttons required)
- ⚡ Fully non-blocking (millis-based timing)

---

## 🧰 Hardware

- Arduino Uno
- WS2812 LED strips (3 strips × 3 LEDs)
- 9 sensors (mechanical switches)
- Vibration sensor (for miss detection)
- Speaker (for sound effects)

---

## 🧠 How It Works

- Each board position has a sensor
- When a bag lands, the corresponding tile is claimed
- The board updates LEDs instantly
- Turns alternate automatically
- A win triggers a custom animation sequence

---

## 🎨 Game Logic Highlights
- Efficient win detection using center/corner anchors
- Edge-triggered input handling (prevents multi-trigger spam)
- State-based animation system:
- Normal play
- Turn switch animation
- Win animation

---

## 🔮 Future Improvements
- 🔊 Sound effects (turn + win)
- 🧠 Smarter miss detection
- 🎨 Additional animations
- 📱 Score tracking / display
- 🛠 Adjustable game modes

## 📜 License

- This project is licensed under the Apache License.

## 🙌 Team
- Elliot Lard
- Baz McIntyre
- Landon Aucoin
- James Cowan
