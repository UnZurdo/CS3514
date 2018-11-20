#include "mbed.h"
#include "buzzer.h"
#include <time.h>

#define PIN_BUZZER D8
#define PIN_START D9
#define PIN_P1 D10
#define PIN_P2 D11
#define PIN_LED D12

// Notes and frequencies
#define MIDDLE_C 261
#define CONCERT_A 440
#define TIME_OUT 100 // end metronome after 10s

// Output interface to serial port connected to computer
Serial pc(USBTX, USBRX);

Beep buzzer(PIN_BUZZER);
DigitalIn startButton(PIN_START); //start button
InterruptIn p1Button(PIN_P1); //player1 button
InterruptIn p0Button(PIN_P2); //player2 button
DigitalOut led(PIN_LED); //led
Timeout response, end, stopWatch;

// Global variables
int beats;
bool start;
int note;
int nBeats;
int nBeatsSinceInterrupt;
int nInterruopts; // record the number of interrupts that ocurred every 10s
bool on;

// Global timer for the game
Timer t;
double t1, t2;


void finish(){
    pc.printf("-- Metronome OFF --\n");
    t2 = t.read_ms(); 
    pc.printf("t1: %f   t2: %f\n", t2,t1);

    pc.printf("Total number of beats: %d in %ds\n", nBeats, t2 - t1);
    on = false;
    response.detach();

}
void updateTone(){
    char key;
    key = pc.getc();
    pc.printf("Update TONE, character %c read\n", key);

    if(key == 'A' || key == 'a'){
       note = CONCERT_A; 
    } else if(key == 'C' || key == 'c'){
       note = MIDDLE_C;
    } else if(key == 85) {
        pc.printf("Started from Processing\n");
        start = true;
        on = true;
    } else if(key == 0) {
        finish();
    }
}
// Calculate beat rate and show via the serial monitor in each interrupt, 
// every 6s
void checkBPM(){
    //double t3 = t.read_ms(); 
    //double diff = t3 - t1; // time diff in seconds
    //double realBPM = nBeats / diff / 60.0; // to minutes
    ++nInterruopts;
    int beatsDiff = nBeats - nBeatsSinceInterrupt;
    nBeatsSinceInterrupt = nBeats;
    pc.printf("Seconds passed: %d - real BPM: %d - expected BPM: %d\n", nInterruopts * 6, beatsDiff * 10, beats);
    stopWatch.attach(&checkBPM, 6);

}

void blink(){
    led = 1;
    buzzer.beep(note,0.1);
    wait(0.08);
    led = 0;
    ++nBeats;
    
    double bpm = beats / 60.0;
    double bps = 1.0 / bpm; // beatsin 1 second
    pc.printf("bps: %f\n", bps); 
    //response.detach();
    response.attach(&blink, bps);
   
}

void updateResponse(){
    double bpm = beats / 60.0;
    double bps = 1.0 / bpm; // beatsin 1 second
    pc.printf("bps: %f\n", bps); 
}

void playerInterrupt(){
    if(p1Button) {
        pc.printf("Increment by 4, now: ", beats);
        beats += 4;
    } 
    else if (p0Button){
        pc.printf("Decrement by 4, now: ", beats);
        beats -= 4;
    }
    updateResponse();
}
int main() {
    beats = 140;
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
    
    pc.attach(&updateTone, Serial::RxIrq);
    t.start();

    while(1) {
        // Game starts
        if(startButton == 1 || start){
            on = true;
            pc.printf("-- START --\n");
            pc.printf("bpm: %d\n", beats); 
            
            t1 = t.read_ms(); 

            double bpm = beats / 60.0;
            double bps = 1.0 / bpm; // beats in 1 second 
            pc.printf("bps: %f\n", bps); 
 
            
            // Set up interrupt for every blink         
            response.attach(&blink, bps);
            // Turn off after TIME_OUT
            // end.attach(&finish, TIME_OUT);
            stopWatch.attach(&checkBPM, 6);

            while(on){
                // Reset variables and clean LCD display
                // BPM
                // Buzzer sounds for 1 second at a given frequency
                //pc.printf("Buzzer is ON!\n");
                // set actual timer for this round
            }
            //response.detach();
        }
    }
}

