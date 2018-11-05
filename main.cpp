#include "mbed.h"
#include "buzzer.h"
#include <time.h>
#include "TextLCD.h"

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
PwmOut mypwm(PWM_OUT);
Serial pc(USBTX, USBRX);

Beep buzzer(PIN_BUZZER);
DigitalIn startButton(PIN_START); //start button
InterruptIn p1Button(PIN_P1); //player1 button
InterruptIn p2Button(PIN_P2); //player2 button

// PortIn p(PortC, 0x00000300);   // p8-p9

double timeP1, timeP0, sumP1, sumP0, winsP1, winsP0;
//time_t t1,t2;
double t1,t2;
// Global timer for the game
Timer t;

void playerInterrupt(){
    t2 = t.read_ms();
    int pins = p.read();
    double diff = t2 - t1;
    //pc.printf("Pins: %d, Player1: %d, Player2: %d \n", pins, p1Button, p2Button);
    
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
    else if (p2Button && timeP0 == UNDEFINED){
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
    p2Button.mode(PullDown);
    
    // Associate subrutine to interrupts triggered by players
    p1Button.rise(&playerInterrupt);
    p2Button.rise(&playerInterrupt);

    while(1) {
        // Game starts
        t.start();
        if(startButton == 1){
            // Reset variables
            round = 1;
            timeP1 = UNDEFINED;
            timeP0 = UNDEFINED;
            sumP1 = 0;
            sumP0 = 0;
            winsP1 = 0;
            winsP0 = 0;
            
            pc.printf("-- NEW GAME --\n");

            while(round <= 3){
                
                int delay = rand() % 10 + BUZZER_TIME;
                
                pc.printf("Round %d, delay: %d\n", round, delay);
                
                // Players shouldn't press the button before buzzer sounds ( while t1 == UNDEFINED )
                // if they do so, they will be penalize
                t1 = UNDEFINED;
                wait(ROUND_TIME);

                pc.printf("Buzzer is ON!\n");
                buzzer.beep(261,1);

                // set actual timer for this round
                t.reset();
                t1 = t.read_ms(); 

                // wait until ROUND finishes
                wait(delay);

                // if both players are disqualified, no one wins
                if (timeP1 == UNDEFINED && timeP0 == UNDEFINED){
                     pc.printf("-- End of Round %d, both players DISQUALIFIED\n", round);

                } 
                // One of the players isn't disqualified, so there is a winner
                else {
                    pc.printf("-- End of Round %d, Player %d WINS! by %.0f ms--\n", round, timeP1 <= timeP0, abs(timeP1 - timeP0));
               } 
                
                // update state for the next ROUND
                ++round;
                updateGame();

            }
            // reset rounds for new GAME
            pc.printf("-- END OF GAME, Player %d WINS the GAME --\n reactionTineP0: %.0f, reactionTineP1: %.0f", winsP1 < winsP0, sumP0/3.0, sumP1/3.0);

        }
    }
}

