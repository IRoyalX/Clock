#include <mega8.h>
#include <delay.h>
int seg[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90},                // COMMON ANODE
    time[3] = {0, 0, 12}, set = 0,
    blk = 0;
//------------DISPLAY ON SEGMENTS-----------//-----------------------------------------------------------------------------------
void display (void){                        // declearing function
    int i;                                  // defining an index
    for(i = 0;i < 6;i++){                   // 6 times loop for 6 digit
     PORTC = 0x00;                          // switch off segments to prepare next digit
     if(i%2 == 0){                          // if i is even ( first digit )
      PORTD = seg[time[i/2]%10];            // sending the first digit amount to output
      if(i && (blk || set > 0))PORTD.7 = 0; // i != 0 ( divider ( DOT ) on first digit ) and blk ( toggle each 0.5sec ), set ( counting mod ) binking dividers
     }                                      //
     else                                   // if i is odd ( second digits )
      PORTD = seg[time[i/2]/10];            // sending the second digit amount to output
     PORTC = ((1 ^ (blk && set > 0 && i/2 == set)) << i);                     // shift 1 to left i times to enable next segment
     delay_ms(1);                           // 1ms delay to switching off
    }                                       //
}                                           //
//-------------TIMER1 FUNCTION--------------//-----------------------------------------------------------------------------------
interrupt [TIM1_OVF] void counter (void){   // defining TIM1 function
    TCCR1B = 0x00;                          // TIM1 clock off
    TCNT1 = 0xC2F7;                         // TIM1 bits amount
                                            //
    if(set < 1){                            // counting mod parts ( disable on settings )
        time[0]++;                          // increase second in each interrupt ( interrupt delay is 1s )
        if(time[0] >= 60){                  // second overflow
            time[0] = 0, time[1]++;         // second reset and incresing minute
            if(time[1] >= 60){              // minute overflow
                time[1] = 0, time[2]++;     // minute reset and increasing hour
                if(time[2] >= 24)           // hour overflow
                    time[2] = 0;            // hour reset
            }                               //
        }                                   //
                                            //
        if(PINB.0 == 0){                    // if set button pressed
            set--;                          // count down to measuring delay ( to make button press for 3sec )
            if(set <= -3){                  // if press delay was more than 3sec
                set = 2, time[0] = 0;       // go to setting & reset seconds
                while(PINB.0 == 0)display();// wait untill button release
            }                               //
        }                                   //
        else set = 0;                       // if button released before ( 3sec ), reset delay counter
    }                                       //
    TCCR1B = 0x03;                          // start timer with prescaler of 64
}                                           //
//--------------OCR COMPARE 0.5s------------//-----------------------------------------------------------------------------------
interrupt [TIM1_COMPA] void blink (void){   // Compare matching to make 0.5s delay
    OCR1A ^= 0x1E84;                        // set compare amount from start point + 7812 and xor with 7812 to make another compare amount on FFFF
    blk = !blk;                             // toggle blk each 0.5s
}                                           //
//--------------CONTROLING PORTS------------//-----------------------------------------------------------------------------------
void main(void){                            //
    PORTB = 0x07;                           // PB = 0
    PORTC = 0x00;                           // PC = 0
    PORTD = 0x00;                           // PIN pullup
                                            //
    DDRB = 0xf8;                            // make input for buttons
    DDRC = 0x3f;                            // segments uncommon ports
    DDRD = 0xff;                            // segments common ports
//-------------TIMER 1 (1S DELAY)-----------//-----------------------------------------------------------------------------------
    TCNT1 = 0xC2F7;                         // TIM1 bits amount
    TCCR1A = 0x00;                          // TIM1 in NORMAL mod
    TCCR1B = 0x03;                          // start TIM1 with prescaler of 64
    OCR1A = 0xE17B;                         // compare amount start point + 7812
    TIMSK = 0x14;                           // Interrupt MASK TIM 1 on OVERFLOW
                                            //
    #asm ("sei");                           // activating globall interrupt
//--------------Rest of Program-------------//-----------------------------------------------------------------------------------
    while (1){                              // infinite program loop
        display();                          // calling function to display on segments
        if(set > 0){                        // if we are in setting mod
            if(PINB.0 == 0){
                set--;
                if(!set)TCNT1 = 0xC2F7;
            }
            else if(PINB.1 == 0)
                time[set]++;
            else if(PINB.2 == 0)
                time[set]--;
            switch(set){
             case 2:
              if(time[set] > 23)
               time[set] = 0;
              else if(time[set] < 0)
               time[set] = 23;
              break;
             case 1:
              if(time[set] > 59)
               time[set] = 0;
              else if(time[set] < 0)
               time[set] = 59;
              break;
            }
            while(PINB ^ 0x07)display();
        }
    }                                       //
}                                           //
//------------------------------------------//-----------------------------------------------------------------------------------