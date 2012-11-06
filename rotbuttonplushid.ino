#include <MIDI.h>

const byte BUTTON = 10;
const byte BLUE = 12;
const byte RED = 11;
const byte GREEN = 13;
const byte encPinA = 2;
const byte encPinB = 3;
const byte ledPin = 12;
const byte r_out = 6;
const byte g_out = 4;
const byte b_out = 5;
//const byte mem_in = 1;
//const int TIMER_1_TOP = 125;
//const int TIMER_1_B = 75;
//const byte strobe_pin = A0;
const byte pot_pins[] = {A15, A14, A13};
const byte sw_in = A2;
const byte X = 0;
const byte Y = 1;
const byte Z = 2;
const byte accels_in[] = {A5, A6, A7};
const byte num_accels = 2;




volatile byte red_pos = 0;
volatile byte green_pos = 0;
volatile byte blue_pos = 0;
volatile byte lastPos = 0;
volatile byte newB;
volatile byte oldA;
volatile byte* current_pos = &blue_pos;

/*int mem_pos;
int mem_posA;
int mem_posB;
int mem_posC;
byte mem_state;
byte mem_last_state;*/
int pot_pos[] = {0,0,0};
int last_pot_pos[] = {0,0,0};
byte sw_state, last_sw;

int which = 0;

byte accels[3];
byte last_accels[3];
byte lp_accels[3];

const int led_out[] = {
  23,25,27,24,22,30,49,48,53,51};
int led_state[] = {
  0,0,0,0,0,0,0,0,0, 0};
const int num_outs = 10;
int red_index = -1;
int blue_index = -1;
int green_index = -1;

void setup()
{
  pinMode(BUTTON, INPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT); 

  for (int i = 0; i < num_outs; i++)
  {
    pinMode(led_out[i], OUTPUT);
  } 

 // DDRH |= (1 << 1);
  //PORTH &= ~(1 << 1);

  pinMode(encPinA, INPUT_PULLUP);
  pinMode(encPinB, INPUT_PULLUP);

  pinMode(r_out, OUTPUT);
  pinMode(g_out, OUTPUT);
  pinMode(b_out, OUTPUT);

  //pinMode(mem_in, INPUT);
  
 // pinMode(strobe_pin, OUTPUT);

  // Set up timer
  /*TCCR1B = 0; // arduino hogs timers
  TCCR1B |= (1 << WGM12); // Set CTC mode
  OCR1A = TIMER_1_TOP;
 // OCR1B = TIMER_1_B;
  TCCR1B |= (1 << CS12) | (1 << CS10); // timer prescale of 1024
  TIMSK1 |= (1 << OCIE1A);// | (1 << OCIE1B); // enable interrupts*/

  attachInterrupt(0, doEncoderA, CHANGE);
  attachInterrupt(1, doEncoderB, CHANGE);

  MIDI.begin(MIDI_CHANNEL_OMNI);

}

void loop()
{
  static int lastState = LOW;
  int state = digitalRead(BUTTON);
  sw_state = digitalRead(sw_in);
  
  for (int i = 0; i < 3; i++)
  {
     pot_pos[i] = analogRead(pot_pins[i]);
  }

  //PORTH ^= (1 << 1);
  if (state != lastState)
  {
    // Send new button state
    MIDI.sendNoteOn(1, state*127, 1);
    if (state == HIGH) 
    {
      which = (which+1)%3;
    }
    lastState = state;
  }
  switch (which)
  {
  case 0:
    if (current_pos != &blue_pos)
    {
      digitalWrite(BLUE, LOW);
      digitalWrite(RED, HIGH);
      digitalWrite(GREEN, HIGH);
      current_pos = &blue_pos;
    }
    //checkEnc(blue_index);
    blue_index = (int)(blue_pos/12.7);
    fill_write(led_state, blue_index, num_outs);
    //PORTC = blue_pos;
    break; 
  case 1:
    if (current_pos != &red_pos)
    {
      digitalWrite(BLUE, HIGH);
      digitalWrite(RED, LOW);
      digitalWrite(GREEN, HIGH);
      current_pos = &red_pos;
    }
    //checkEnc(red_index);
    red_index = (int)(red_pos/12.7);
    clear_write(led_state, red_index, num_outs);
    break; 
  case 2:
    if (current_pos != &green_pos)
    {
      digitalWrite(BLUE, HIGH);
      digitalWrite(RED, HIGH);
      digitalWrite(GREEN, LOW);
      current_pos = &green_pos;
    }
    //checkEnc(green_index);
    green_index = (int)(green_pos/12.7);
    fillup_write(led_state, green_index, num_outs);
    break; 
  }
  
 /* static int last_mem = 0;
  static int lp_mem = 0;*/
  static byte last_enc = 0;
  
  
  // send membrane pot if necessary, on cc 4
 /* lp_mem = (mem + last_mem) /2;
  if (mem_pos != last_mem)
  {
     MIDI.sendControlChange(4, lp_mem/8, 1);  
  }*/
  // send encoder if necessary, blue cc 1, green cc 3, red cc 2
  if (last_enc != *current_pos) // if last_enc doesn't equal the value of current_pos
  {
    // if current is blue
     if (current_pos == &blue_pos)
     {
       MIDI.sendControlChange(1, blue_pos, 1);
     }
     else if (current_pos == &red_pos)
     {  
       MIDI.sendControlChange(2, red_pos, 1);
     }
     else if (current_pos == &green_pos)
     {  
       MIDI.sendControlChange(3, green_pos, 1);
     }
  }
  last_enc = *current_pos;
  
  // send accelerometer values if necessary
  // x is cc 8, y cc 9, z cc 10
  for (int i = 0; i < num_accels; i++) 
  {
    // get new accels, scale to midi range (0 - 127)
      accels[i] = analogRead(accels_in[i])/8;
      
      lp_accels[i] = (accels[i] + last_accels[i]) * 0.5;
      if (abs(lp_accels[i] - last_accels[i]) > 1)
      {
         MIDI.sendControlChange(8+i, lp_accels[i], 1); 
      }
      last_accels[i] = accels[i];
  }
  
  
  
  // Send switch
  if (sw_state != last_sw)
  {
    if (sw_state == HIGH)
       MIDI.sendNoteOn(2, 127, 1);
    else
      MIDI.sendNoteOn(2, 0, 1);
  }
  
  
 // last_mem = mem_pos;
  last_sw = sw_state;
  for (int i = 0; i < 3; i++)
  {
     // send side pots, cc numbers 5, 6, 7
     if (pot_pos[i]/8 != last_pot_pos[i]/8)
       MIDI.sendControlChange(i+5, pot_pos[i]/8, 1);
     last_pot_pos[i] = pot_pos[i]; 
  }

  write_led(led_out, led_state, num_outs);

  analogWrite(r_out, pot_pos[0]/4);
  analogWrite(g_out, pot_pos[1]/4);
  analogWrite(b_out, pot_pos[2]/4);

  delay(10);
}

/*void checkEnc(int &index)
 {
 static unsigned int lastPos = 0;
 if (encoderPos > lastPos && index < num_outs-1)
 {
 index = (index+1); 
 lastPos = encoderPos;
 }
 else if (encoderPos < lastPos && index > 0)
 {
 index = abs((index-1)%num_outs); 
 lastPos = encoderPos;
 }
 }*/

void clear_write(int states[], int index, int num_states)
{
  for (int i = 0; i < num_states; i++)
  {
    (i == index)? states[i] = 1 : states[i] = 0;
  } 
}

void fill_write(int states[], int index, int num_states)
{
  for (int i = 0; i < num_states; i++)
  {
    (i <= index)? states[i] = 1 : states[i] = 0;
  } 
}

void fillup_write(int states[], int index, int num_states)
{
  for (int i = 0; i < num_states; i++)
  {
    (i >= index)? states[i] = 1 : states[i] = 0;
  } 
}

void write_led(const int led_pins[], int led_states[], int num_pins)
{
  for (int i = 0; i < num_pins; i++)
  {
    digitalWrite(led_pins[i], led_states[i]);
  } 
}

void doEncoderA() // change in A
{
  delay(2);
  doEncoder();
  oldA = digitalRead(encPinA);
  oldA &= 0x01; // just in case, keep only msb
}

void doEncoderB() // change in B
{
  delay(2);
  newB = digitalRead(encPinB);
  newB &= 0x01; // jic
  doEncoder();
}

void doEncoder() // do real work, to be called on a rise or fall on either pin
{
  /* Output for clockwise rotation:
   * phase         A   B    
   * 1             0   0 
   * 2             0   1
   * 3             1   1
   * 4             1   0
   *
   * Counter clockwise:
   * 1             1   0
   * 2             1   1
   * 3             0   1
   * 4             0   0
   *
   * This type of encoder goes from 00 to 00 each detent
   * ie each click is four counts.
   * 
   * Also note if you XOR each B value with its previous A,
   * you will get 1 for clockwise motion, 0 for anticlockwise.
   *
   * Thus if we wait for 4 consecutive movements in one direction,
   * we have completed one full click, and should have successfully
   * ignored bounce
   */

  static byte states = 0;
  byte current_val = *current_pos;

  states <<= 1; // move across
  states &= 0x1f; // discard unused bits

  byte temp = newB^oldA; // check direction of current phase change
  states |= temp; // set msb of state to value in temp

    if (states == 0x1f) { // four 1s in a row
    if (current_val >= 4)
      current_val-=4;
    states = 0x01;
  }
  else if (states == 0x10) { // four 0s in a row
    if (current_val <= 127-4)
      current_val+=4;
    states = 0x01;
  }
  if (current_val-lastPos > 1)
  {
    current_val = (current_val+lastPos)/2;
    lastPos = current_val; 
  }
  else if (current_val-lastPos < -1)
  {
    current_val = (current_val+lastPos+1)/2;
    lastPos = current_val;
  }
  if (current_val > 127)current_val = 127;
  if (current_val <0)current_val = 0;

  *current_pos = current_val;
}

/*ISR(TIMER1_COMPA_vect)
{
  cli();
  mem_posB = analogRead(mem_in);
  digitalWrite(strobe_pin, HIGH);
  mem_posA = analogRead(mem_in);
  digitalWrite(strobe_pin, LOW); 
  if (abs(mem_posA - mem_posB) < 1)
    mem_state = 1;
  else 
    mem_state = 0;
    
  if (mem_state == mem_last_state && mem_state == 1)
  {
     mem_pos = mem_posA; 
  }
  mem_last_state = mem_state;
  sei();
}*/

/*ISR(TIMER1_COMPB_vect)
{
  digitalWrite(strobe_pin, HIGH);
  mem_posB = analogRead(mem_in);
} */


