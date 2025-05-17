# Embedded-Project
This is the Repositry for the embedded systems project
# 🕒 4-Digit 7-Segment Display Timer & Voltmeter (mbed - NUCLEO-F401RE)

## 📋 Overview

This project demonstrates how to use a 4-digit 7-segment display with a NUCLEO-F401RE development board and mbed OS. It has two operating modes:

1. **Timer Mode (MM:SS)** – counts up from `00:00` to `99:59`
2. **Voltage Mode** – reads an analog voltage (via a potentiometer) and displays it as a 4-digit millivolt value (e.g., `3.30V` → `3300`)

---

## 🔌 Hardware Connections

| Component         | NUCLEO Pin | Function        |
|------------------|------------|-----------------|
| Latch (LCHCLK)   | PB_5       | `DigitalOut`    |
| Clock (SFTCLK)   | PA_8       | `DigitalOut`    |
| Data (SDI)       | PA_9       | `DigitalOut`    |
| Reset Button (S1)| PA_1       | `DigitalIn`     |
| Mode Button (S3) | PB_0       | `DigitalIn`     |
| Potentiometer    | PA_0       | `AnalogIn`      |

---

## 🧠 Core Concepts

### 🕓 1. Timer Mode (MM:SS Format)
- Time is tracked using a `Ticker` interrupt that fires every 1 second.
- The display updates across 4 digits:
  - Digits 0–1: Minutes (with a colon-like decimal on digit 1)
  - Digits 2–3: Seconds

### 🔋 2. Voltage Mode (0.00V - 3.30V)
- Voltage is read using an ADC input connected to a potentiometer.
- Converted from analog to millivolts and displayed over 4 digits.
  - E.g., 3.3V → `3300`

### 🔁 3. Multiplexed Display Control
- Uses shift registers to drive the 7-segment display.
- A fast `Ticker` (1ms) triggers display refreshes, updating one digit at a time.
- The digits are cycled quickly to simulate a static image.

---

## 🧱 Components Used

- **NUCLEO-F401RE Board**
- **4-digit 7-segment display with shift register**
- **Potentiometer (connected to analog pin)**
- **Two push buttons (for reset and mode switching)**

---

## ⌨️ Controls

| Button | Action                            |
|--------|-----------------------------------|
| S1     | Reset time to 00:00 (only in timer mode) |
| S3     | Hold to switch to voltage mode. Release to return to timer mode. |

---

## 🔧 How It Works (Code Breakdown)

### Timekeeping
```cpp
Ticker second_tick;
second_tick.attach(&tick, 1.0);
```
Increments `total_seconds` every second.

### Display Refresh
```cpp
Ticker refresh_tick;
refresh_tick.attach(&refreshISR, 0.001);
```
Triggers refresh every 1ms for multiplexing.

### Segment Output
```cpp
void outputToDisplay(uint8_t segments, uint8_t digitSelect)
```
Uses SPI-like bit-banging to send segment and digit control data to the shift registers.

---

## 🧪 Notes

- Segment encoding is **active-low** (common anode display).
- Uses `& 0x7F` in digit 1 or voltage digit 0 to simulate a **decimal point**.
- Voltage is limited to 0–3.3V due to the MCU's ADC range.
- `total_seconds` wraps at 6000 seconds (99:59) to avoid overflow.

