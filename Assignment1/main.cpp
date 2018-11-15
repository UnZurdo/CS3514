#include "mbed.h"
#include "buzzer.h"
#include <time.h>
#include "TextLCD.h"
#include <pthread.h>

#define ROUND_TIME 5 //minimum time between each round 5s
#define BUZZER_TIME 3 //minimum time for the buzzer 3s
#define UNDEFINED 14000 // value used to represent an undefined time difference
#define PIN_BUZZER D8
#define PIN_START D9
#define PIN_P1 D10
#define PIN_P2 D11
#define PINS_LCD D2,D3,D4,D5,D6,D7

// Pins for LCD display D2, D3, D4, D5, D6, D7
TextLCD lcd(PINS_LCD); //rs, e, d4, d5, d6, d7

// Output interface to serial port connected to computer
Serial pc(USBTX, USBRX);

Beep buzzer(PIN_BUZZER);
DigitalIn startButton(PIN_START); //start button
InterruptIn p1Button(PIN_P1); //player1 button
InterruptIn p0Button(PIN_P2); //player2 button

PortIn p(PortA, 0x00000300);   // NOT used in this implementation

// Global variables
double timeP1, timeP0, sumP1, sumP0, t1, t2;
int winsP1,winsP0;

// Global timer for the game
Timer t;

void playerInterrupt(){
    t2 = t.read_ms();
    int pins = p.read();
    double diff = t2 - t1;
    //pc.printf("Pins: %d, Player1: %d, Player2: %d \n", pins, p1Button, p0Button);
    
    // If player 1 presses the button for the first time in this round
    if(p1Button && timeP1 == UNDEFINED) {
        if (t1 == UNDEFINED) {
            pc.printf("Player1 => DISQUALIFIED\n", diff);

        } else {
            pc.printf("Player1 => Time diff: %.0f ms\n", diff);
            timeP1 = diff;  
        }
    } 
    // If player 2 presses the button for the first time in this round
    else if (p0Button && timeP0 == UNDEFINED){
        if (t1 == UNDEFINED) {
            pc.printf("Player0 => DISQUALIFIED\n", diff);

        } else {
            pc.printf("Player0 => Time diff: %.0f ms\n", diff);
            timeP0 = diff;  
        }
    }
}

void updateGame(){
    // In case of DISQUALIFIED; the player will be penalize, 
    // adding the maximun reaction time possible
    sumP1 += timeP1;
    sumP0 += timeP0;
    
    if (timeP1 <= timeP0 ){
        winsP1++;
    } else {
        winsP0++;
    }
    timeP1 = UNDEFINED;
    timeP0 = UNDEFINED;

}

int main() {
      /* initialize random seed: */
    srand (time(NULL)); 
        
    int round = 1;
    // Set PullDown modes for each of the input buttons
    startButton.mode(PullDown);
    p1Button.mode(PullDown);
    p0Button.mode(PullDown);
    
    // Associate subrutine to interrupts triggered by players
    p1Button.rise(&playerInterrupt);
    p0Button.rise(&playerInterrupt);

    while(1) {
        // Game starts
        t.start();
        if(startButton == 1){
            // Reset variables and clean LCD display
            lcd.cls();
            round = 1;
            timeP1 = UNDEFINED;
            timeP0 = UNDEFINED;
            sumP1 = 0;
            sumP0 = 0;
            winsP1 = 0;
            winsP0 = 0;
            
            while(round <= 3){             
                // Time for current round
                int delay = rand() % 10 + BUZZER_TIME;
                
                // Clear and print into display
                lcd.cls();
                lcd.printf("Round %d STARTS\n", round);

                
                // Players shouldn't press the button before buzzer sounds ( while t1 == UNDEFINED )
                // if they do so, they will be penalize
                t1 = UNDEFINED;
                wait(delay);

                // Buzzer sounds for 1 second at a given frequency
                buzzer.beep(261,1);

                // set actual timer for this round
                t.reset();
                t1 = t.read_ms(); 

                // Clear and print into display
                lcd.cls();
                // if both players are disqualified, no one wins
                if (timeP1 == UNDEFINED && timeP0 == UNDEFINED){
                    lcd.printf("both players \n DISQUALIFIED", round);

                } 
                // One of the players isn't disqualified, so there is a winner
                else {
                    lcd.printf("P0:%.0f  P1:%.0f\nP%d WINS by %.0f", timeP0, timeP1, timeP1 <= timeP0, abs(timeP1 - timeP0));

               } 
                // wait until ROUND finishes
                wait(ROUND_TIME);
                
                // update state for the next ROUND
                ++round;
                updateGame();

            }
            // reset rounds for new GAME
            lcd.printf("//END// P%d WINS\nP0: %.0f, P1: %.0f", winsP1 < winsP0, sumP0/3.0, sumP1/3.0);

        }
    }
}

