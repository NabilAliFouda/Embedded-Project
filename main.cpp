// بسم الله نبدأ //

#include "mbed.h"

// Define shield connections (NUCLEO-F401RE pins)
DigitalOut latch(PB_5);    // D4 -> LCHCLK (Latch)
DigitalOut clk(PA_8);      // D7 -> SFTCLK (Clock)
DigitalOut data(PA_9);     // D8 -> SDI (Data)
DigitalIn  B1(PA_1);  // S1 -> A1 (Reset button, active low)
DigitalIn  B3(PB_0);  // S3 -> A3 (Mode button, active low)
AnalogIn   pot(PA_0);      // Potentiometer -> A0

// 7-seg encoding for digits 0-9 (active-low segments, common anode)
const uint8_t SEGMENT_MAP[10] = { 
    0b11000000, 0b11111001, 0b10100010, 0b1011000, 0b10011001, 
    0b10010010, 0b10000010, 0b11111000, 0b10000000, 0b10010000 
};
// Digit select bytes for 4 digits (active low to enable one digit)
const uint8_t SEGMENT_SELECT[4] = { 0b1110001, 0b11110010, 0b11110010, 0b11111000 };

// Shared variables for time
volatile int total_seconds = 0;
volatile bool updateDisplay = true;
 
// Ticker to increment time every 1 second
Ticker second_tick;
void tick() {
    total_seconds++;
    if(total_seconds >= 6000) { // wrap at 99:59 for safety
        total_seconds = 0;
    }
}

// Function to output two bytes to the shift registers (segments + digit)
void outputToDisplay(uint8_t segments, uint8_t digitSelect) {
    latch = 0;
    // Shift out 8 bits of segment data (MSB first)
    for(int i = 7; i >= 0; --i) {
        data = (segments >> i) & 0x1;
        clk = 0;
        clk = 1;
    }
    // Shift out 8 bits of digit select data
    for(int i = 7; i >= 0; --i) {
        data = (digitSelect >> i) & 0x1;
        clk = 0;
        clk = 1;
    }
    latch = 1;
}

// ISR ticker for display multiplex (called e.g. every 2 ms)
Ticker refresh_tick;
volatile int currDigit = 0;
void refreshISR() {
    updateDisplay = true;  // flag main loop to update (or do work here if non-RTOS)
}
 
int main() {
    // Initializations
    B1.mode(PullUp);
    B3.mode(PullUp);
    second_tick.attach(&tick, 1.0);            // tick every 1 second
    refresh_tick.attach(&refreshISR, 0.001);   // 2 ms refresh interrupt

    // Pre-calculate blank pattern for safety
    const uint8_t BLANK = 0xFF; // (all segments off)
 
    bool modeVoltage = false;
    int prev_b1 = 1, prev_b3 = 1;
 
    while (true) {
        // Check S1 (reset button)
        int b1 = B1.read();
        if(b1 == 0 && prev_b1 == 1) {  // falling edge
            total_seconds = 0;  // reset time
        }
        prev_b1 = b1;
 
        // Check S3 (mode switch)
        int b3 = B3.read();
        // If pressed, enter voltage display mode; if released, go back to time
        modeVoltage = (b3 == 0);
        prev_b3 = b3;
 
        // When refresh tick triggers, update the display
      if(updateDisplay) {
    updateDisplay = false;

    uint8_t segByte = 0xFF, selByte = 0xFF;

    if (!modeVoltage) {
        // MM:SS mode
        int seconds = total_seconds % 60;
        int minutes = total_seconds / 60;

        switch(currDigit) {
            case 0: segByte = SEGMENT_MAP[minutes / 10]; selByte = SEGMENT_SELECT[0]; break;
            case 1: segByte = SEGMENT_MAP[minutes % 10] & 0x7F; selByte = SEGMENT_SELECT[1]; break;
            case 2: segByte = SEGMENT_MAP[seconds / 10]; selByte = SEGMENT_SELECT[2]; break;
            case 3: segByte = SEGMENT_MAP[seconds % 10]; selByte = SEGMENT_SELECT[3]; break;
        }
    } else {
        // Voltage mode
        float volts = pot.read() * 3.3f;
        int millivolts = (int)(volts * 1000.0f);
        if(millivolts > 9999) millivolts = 9999;
        int intPart = millivolts / 1000;
        int fracPart =millivolts % 1000;

        switch(currDigit) {
            case 0: segByte = SEGMENT_MAP[intPart] &0x7F; selByte = SEGMENT_SELECT[0]; break;
            case 1: segByte = SEGMENT_MAP[fracPart / 100]; selByte = SEGMENT_SELECT[1]; break;
            case 2: segByte = SEGMENT_MAP[(fracPart % 100)/10]; selByte = SEGMENT_SELECT[2]; break;
            case 3: segByte = SEGMENT_MAP[(fracPart % 1000)%10]; selByte = SEGMENT_SELECT[3]; break;
        }
    }

    outputToDisplay(segByte, selByte);
    currDigit = (currDigit + 1) % 4;  // Cycle to next digit
}
    // (In a low-power or RTOS scenario, consider sleeping the thread here briefly)
    }
}
