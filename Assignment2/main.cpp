#include "mbed.h"
#include "buzzer.h"
#include <time.h>
#include "TextLCD.h"

#define PIN_BUZZER D8
#define PIN_START D9
#define PIN_P1 D10
#define PIN_P2 D11
#define PIN_LED D12
#define PINS_LCD D2,D3,D4,D5,D6,D7

// Notes and frequencies
#define MIDDLE_C 261
#define CONCERT_A 440
#define TIME_OUT 100 // end metronome after 10s

// Output interface to serial port connected to computer
Serial pc(USBTX, USBRX);
TextLCD lcd(PINS_LCD); //rs, e, d4, d5, d6, d7

Beep buzzer(PIN_BUZZER);
InterruptIn startButton(PIN_START); //start button
InterruptIn p1Button(PIN_P1); //player1 button
InterruptIn p0Button(PIN_P2); //player2 button
DigitalOut led(PIN_LED); //led
Ticker response, end, stopWatch;

// Global variables
int beats, note, nBeats, nBeatsSinceInterrupt, nInterruopts;
bool start;
bool on;

// Global timer for the game
Timer t;
double t1, t2;


void finish(){
    pc.printf("-- Metronome OFF --\n");
    t2 = t.read_ms();
    //pc.printf("t1: %f   t2: %f\n", t2,t1);
    pc.printf("Total number of beats: %d in %.2fs\n", nBeats, (t2 - t1) / 1000);
    on = false;
    start = false;
    response.detach();
    stopWatch.detach();
    
}

void updateResponse(){
    double bps = beats / 60.0;
    double period;
    if(bps > 1.0) period = 1.0 / bps; // beats in 1 second
    else period = bps;
    pc.printf("Beats: %d - Period: %fs\n", beats, period);
    lcd.cls();
    lcd.printf("Beats: %d\nPeriod: %.3fs\n", beats, period);
    
}

// Handle interrupts coming from the serial port
void updateTone(){
    char key;
    // Get value
    key = pc.getc();
    // Case CONCERT_A button
    if(key == 2){
        note = CONCERT_A;
    }
    // Case MIDDLE_C button
    else if(key == 3){
        note = MIDDLE_C;
    }
    // Case start button pressed
    else if(key == 1) {
        start = true;
        on = true;
    }
    // Case finish button pressed
    else if(key == 0) {
        finish();
    }
    // Rest of the cases for values betwwen 40 and 208 are values related to the beat rate
    else if(key >= 40 && key <= 208){
        // Assign new frequency (BPM)
        beats = key;
        updateResponse();
    }
}
// Calculate beat rate and show via the serial monitor in each interrupt,
// every 6s
void checkBPM(){
    ++nInterruopts;
    int beatsDiff = nBeats - nBeatsSinceInterrupt;
    nBeatsSinceInterrupt = nBeats;
    pc.printf("Seconds passed: %d - real BPM: %d - expected BPM: %d\n", nInterruopts * 6, beatsDiff * 10, beats);
    stopWatch.attach(&checkBPM, 6);
    
}

void blink(){
    double bps = beats / 60.0;
    double period;
    if(bps > 1.0) period = 1.0 / bps; // beats in 1 second
    else period = bps;
    pc.printf("bps: %f\n", bps);
    response.detach();
    response.attach(&blink, period);
    
    // LED flash
    led = 1;
    // Sound BUZZER
    buzzer.beep(note,0.1);
    // Minimul time to allow LED to be visible
    wait(0.1);
    led = 0;
    ++nBeats;
}

void playerInterrupt(){
    if(p1Button) {
        pc.printf("Increment by 4, now: ", beats);
        // Edge case, don't allow higher BPM's
        if(beats <= 204) beats += 4;
        updateResponse();
    }
    else if (p0Button){
        pc.printf("Decrement by 4, now: ", beats);
        // Edge case, don't allow lower BPM's
        if(beats >= 44) beats -= 4;
        updateResponse();
    } else if (startButton){
        start = true;
    }
    
}
int main() {
    // Set initial state
    beats = 40;
    nBeats = 0;
    start = false;
    on = false;
    note = 440;
    nInterruopts = 0;
    nBeatsSinceInterrupt = 0;
    
    // Set PullDown modes for each of the input buttons
    startButton.mode(PullDown);
    p1Button.mode(PullDown);
    p0Button.mode(PullDown);
    
    // Associate subrutine to interrupts triggered by players
    p1Button.rise(&playerInterrupt);
    p0Button.rise(&playerInterrupt);
    startButton.rise(&playerInterrupt);
    pc.attach(&updateTone, Serial::RxIrq);
    t.start();
    
    while(1) {
        // Game starts
        if(start){
            on = true;
            pc.printf("-- START --\n");
            t1 = t.read_ms();
            double bps = beats / 60.0;
            double period;
            if(bps > 1.0) period = 1.0 / bps; // beats in 1 second
            else period = bps;
            pc.printf("bps: %f\n", bps);
            
            lcd.cls();
            lcd.printf("Beats: %d\nPeriod: %.3fs\n", beats, period);
            
            // Set up interrupt for every blink
            response.attach(&blink, bps);
            // Turn off after TIME_OUT
            stopWatch.attach(&checkBPM, 6);
            
            while(on){
                wait(0.5);
            }
        }
    }
}

