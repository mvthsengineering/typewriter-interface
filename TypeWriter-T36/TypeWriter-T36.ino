/*
   CN1                                      CN2
   12 NA                                    12 D12 INPUT
   11 NA                                    11 D11 INPUT
   10 NA                                    10 D10 INPUT
   09 NA                                    09 D09 INPUT
   08 D3 OUTPUT START PULSE                 08 D08 INPUT
   07 NA                                    07 NA
   06 NA                                    06 NA
   05 NA                                    05 NA  OUTPUT SIGNAL FOR CAPSLOCK (ON PULSE 4)
   04 ?  INPUT FOR SHIFT/LOCK               04 D04 INPUT
   03 NA                                    03 D13 INPUT
   02 NA                                    02 NA  OUTPUT SIGNAL FOR SHIFT (ON PULSE 3)
   01 GND                                   01 NA

   D12 --> CN2 12
   D11 --> CN2 11
   D10 --> CN2 10
   D9  --> CN2 9
   D8  --> CN2 8
   D7  --> CN2 4
   D6  --> CN2 3   j, x, .,
   D12 --> CN1 4   SHIFT INPUT
   D3  --> CN1 8   START PULSE
   GND --> CN1 1   GND

*/

/* NOTES: Make SHIFT_PULSE a variable. It can be set to an number outside the range to turn off
  Test removing all detach with subsequate attatch.
*/

#define C12     12
#define C11     11
#define C10     10
#define C09     9
#define C08     8
#define C04     7
#define C03     6

#define ZERO_PULSE_DELAY  400        // Length of initial pulse, hardcoded (see below)
#define PRESS_COUNT       5          // Number of clocks for each press
#define RISE_TOTAL        10         // Total number of rising pulses (or timing sequences) after the first falling pulse
#define SHIFT_ON          3          // Correct pulse for shift (3rd of sequence)
#define SHIFT_OFF         13         // Dummy pulse count, outside range and cannot be reached

//Column of array that cooresponds to pulse and pin
#define CONTROL_PIN       0
#define CONTROL_PULSE     1

// States
#define PRESS_KEY   0
//#define WAIT        1
#define ZERO_PULSE  3
#define SERIAL      4

//Special pins
#define SHIFT_LOCK_WRITE  5 // Pin used to set shift or shift_lock
#define START_SIGNAL      3 // Pin used to read clock pulse from typewriter

volatile uint8_t pulse_counter;     // Which of the ten (0-9) pulses is present
volatile uint8_t press_counter;     // For how many pulse sets has the key been pressed

// Pulse and pin combination determine which key press is being set
volatile uint8_t control_pulse;   // Which of the nine pulses are we looking for
volatile uint8_t control_pin;     // Which of eight input pins seven on CN2 and one on CN1 are we setting

// Both of these variables are set as volatile since they are used in the interrupt
volatile uint8_t state = SERIAL;    // This sets the state of the code.
volatile uint8_t shift_status = SHIFT_OFF;

uint8_t controlPins[7] = {C03, C04, C08, C09, C10, C11, C12}; // Set of pins controlling inputs on CN2

// For testing
//char text[24] = {'B', 'i', 'g', ' ', 'b', 'a', 'd', ' ', 'd', 'o', 'g', 's', '.', 13};
char ascii_key = 'R';               // Variable for holding ascii characters
char ascii_key_last;                // For dealing with double types i.e. oo, bb
//char char_count = 0;

// Special keyboard characters that require a shift.
const char odds_char[] = "!@#$%&*()_+:\"?";
const char keys_char[] = "abcdefghijklmnopqrstuvwxyz.,-=;'1234567890";
const char caps_char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char ctrs_char[] = "\r, \b, \n";

//bool shift_key = false;  // Set if the character uses the shift key

void start_pulse_int();
void count_pulse_int();
void end_pulse_int();
void zero_pulse_int();

uint8_t letter[128][2] = {
  {0, 0}, //0 NULL
  {0, 0}, //1 NULL
  {0, 0}, //2 NULL
  {0, 0}, //3 NULL
  {0, 0}, //4 NULL
  {0, 0}, //5 NULL
  {0, 0}, //6 NULL
  {0, 0}, //7 NULL
  {C11, 3}, //8 BACK SPACE
  {C09, 8}, //9 TAB

  {0, 0}, //10 LF
  {0, 0}, //11 NULL
  {0, 0}, //12 NULL
  {C09, 4}, //13 CARRIAGE RETURN
  {C11, 1}, //14 MARGIN RELEASE
  {C12, 7}, //15 LEFT MARGIN
  {0, 0}, //16 NULL
  {0, 0}, //17 NULL
  {0, 0}, //18 NULL
  {0, 0}, //19 NULL

  {0, 0}, //20 NULL
  {0, 0}, //21 NULL
  {0, 0}, //22 NULL
  {0, 0}, //23 NULL
  {0, 0}, //24 NULL
  {0, 0}, //25 NULL
  {0, 0}, //26 NULL
  {0, 0}, //27 NULL
  {0, 0}, //28 NULL
  {0, 0}, //29 NULL

  {0, 0}, //30 NULL
  {0, 0}, //31 NULL
  {C03, 6}, //32 SPACE
  {C12, 1}, //33 !
  {C04, 3}, //34 "
  {C11, 8}, //35 #
  {C12, 8}, //36 $
  {C12, 9}, //37 %
  {C12, 0}, //38 &
  {C04, 3}, //39 '

  {C11, 2}, //40 (
  {C11, 6}, //41 )
  {C12, 2}, //42 *
  {C12, 3}, //43 +
  {C04, 6}, //44 ,
  {C12, 6}, //45 -
  {C03, 2}, //46 .
  {C03, 4}, //47 /
  {C11, 6}, //48 0
  {C12, 1}, //49 1

  {C11, 7}, //50 2
  {C11, 8}, //51 3
  {C12, 8}, //52 4
  {C12, 9}, //53 5
  {C11, 0}, //54 6
  {C12, 0}, //55 7
  {C12, 2}, //56 8
  {C11, 2}, //57 9
  {C04, 4}, //58 :
  {C04, 4}, //59 ;

  {0, 0}, //60 <
  {C12, 3}, //61 =
  {0, 0}, //62 >
  {C03, 4}, //63 ?
  {C11, 7}, //64 @
  {0, 0}, //65 NULL
  {0, 0}, //66 NULL
  {0, 0}, //67 NULL
  {0, 0}, //68 NULL
  {0, 0}, //69 NULL

  {0, 0}, //70 NULL
  {0, 0}, //71 NULL
  {0, 0}, //72 NULL
  {0, 0}, //73 NULL
  {0, 0}, //74 NULL
  {0, 0}, //75 NULL
  {0, 0}, //76 NULL
  {0, 0}, //77 NULL
  {0, 0}, //78 NULL
  {0, 0}, //79 NULL

  {0, 0}, //80 NULL
  {0, 0}, //81 NULL
  {0, 0}, //82 NULL
  {0, 0}, //83 NULL
  {0, 0}, //84 NULL
  {0, 0}, //85 NULL
  {0, 0}, //86 NULL
  {0, 0}, //87 NULL
  {0, 0}, //88 NULL
  {0, 0}, //89 NULL

  {0, 0}, //90 NULL
  {0, 0}, //91 [
  {0, 0}, //92 NULL
  {0, 0}, //93 NULL
  {0, 0}, //94 NULL
  {C12, 6}, //95 _
  {0, 0}, //96 NULL
  {C04, 8},   //97 a
  {C04, 5},   //98 b
  {C03, 7},   //99 c

  {C04, 7},   //100 d
  {C09, 1},   //101 e
  {C09, 7},   //102 f
  {C09, 2},   //103 g
  {C04, 2},   //104 h
  {C09, 9},   //105 i
  {C03, 0},   //106 j
  {C09, 6},   //107 k
  {C09, 3},   //108 l
  {C04, 9},   //109 m

  {C03, 9},   //110 n
  {C10, 9},   //111 o
  {C10, 2},   //112 p
  {C10, 8},   //113 q
  {C10, 7},   //114 r
  {C04, 1},   //115 s
  {C10, 0},   //116 t
  {C11, 9},   //117 u
  {C04, 0},   //118 v
  {C10, 1},   //119 w

  {C03, 1},   //120 x
  {C09, 0},   //121 y
  {C03, 8},   //122 z
  {0, 0},  //123 NULL
  {0, 0},  //124 NULL
  {0, 0},  //125 NULL
  {0, 0},  //126 NULL

};

// Find the start pulse on the leading (CN1 pin 8) signal line.
// There will be a total of ten pulses (timing sequences) 0-9
void start_pulse_int() {
  delayMicroseconds(50);                                      // This delay waits to make sure we are starting on the correct start pulse
  if (digitalRead(START_SIGNAL) == LOW) {                     // We have found the start pulse!
    pulse_counter = 1;                                        // Set the pulse counter to one since this counts as the first pulse
    if (press_counter++ < PRESS_COUNT) {                      // Check how many pulse sets have passed
      // Clear the pulse count
      if (control_pulse == 0) {                               // The zero pulse is a special case and must be handled w/o interrupts
        digitalWrite(control_pin, LOW);
        detachInterrupt(digitalPinToInterrupt(START_SIGNAL));
        state = ZERO_PULSE;
      } else {
        detachInterrupt(digitalPinToInterrupt(START_SIGNAL));     // If not zero than start counting pulses in the count int routine
        attachInterrupt(digitalPinToInterrupt(START_SIGNAL), count_pulse_int, RISING);
      }
    } else {                                                  // We have completed the specified number of pulse sets, return and wait
      press_counter = 0;
      detachInterrupt(digitalPinToInterrupt(START_SIGNAL));
      //state = WAIT; // For non serial applications
      state = SERIAL; // For serial applications
    }
  }
}

// Count each pulse and check for control pulse
void count_pulse_int() {
  if (pulse_counter >= RISE_TOTAL) {                          // Are we at the end of a pulse set (10 total pulses)
    detachInterrupt(digitalPinToInterrupt(START_SIGNAL));     // Return to waiting for start pulse
    attachInterrupt(digitalPinToInterrupt(START_SIGNAL), start_pulse_int, FALLING);
    
  } else if (pulse_counter == control_pulse) {                // Is the pulse we are looking for? 
    digitalWrite(control_pin, LOW);                           // Pull CN2 line low and wait for end of pulse
    detachInterrupt(digitalPinToInterrupt(START_SIGNAL));
    attachInterrupt(digitalPinToInterrupt(START_SIGNAL), end_pulse_int, FALLING);
  }
  if (pulse_counter == shift_status) {                        // Do we need to add a shift pulse?
    digitalWrite(SHIFT_LOCK_WRITE, LOW);                      // Pull the shift line CN1-4 low and wait for end of pulse
    detachInterrupt(digitalPinToInterrupt(START_SIGNAL));
    attachInterrupt(digitalPinToInterrupt(START_SIGNAL), end_pulse_int, FALLING);
  }
  pulse_counter++;
}

// Find end of pulse and go back to counting pulses
void end_pulse_int() {
  digitalWrite(control_pin, HIGH);
  digitalWrite(SHIFT_LOCK_WRITE, HIGH);   //SHIFT_NEW
  detachInterrupt(digitalPinToInterrupt(START_SIGNAL));
  attachInterrupt(digitalPinToInterrupt(START_SIGNAL), count_pulse_int, RISING);
}

void setup() {

  // Serial.begin(115200);
  Serial.begin(9600);

  // Set CN2 drive pins as OUTPUT
  for (int x = 0; x < sizeof(controlPins); x++) {
    pinMode(controlPins[x], OUTPUT);
    digitalWrite(controlPins[x], HIGH);
  }

  // Set read control for the start signal
  pinMode(START_SIGNAL, INPUT_PULLUP);

  // Set the interrupt
  // attachInterrupt(digitalPinToInterrupt(START_SIGNAL), start_pulse_int, FALLING);
}


void loop() {

  switch (state) {
    case SERIAL:
      if (Serial.available() > 0) {
        ascii_key = Serial.read();

        // Check to see which sort of character is being sent.
        char *odds = strchr(odds_char, ascii_key);
        char *keys = strchr(keys_char, ascii_key);
        char *caps = strchr(caps_char, ascii_key);
        char *ctrs = strchr(ctrs_char, ascii_key);

        if (caps != NULL) {                       // We have a capital letter
          shift_status = SHIFT_ON;
          pinMode(SHIFT_LOCK_WRITE, OUTPUT);
          ascii_key += 32;
        } else if (odds != NULL) {               // We have a special character accessed with a shift
          shift_status = SHIFT_ON;
          pinMode(SHIFT_LOCK_WRITE, OUTPUT);
        } else if (keys != NULL) {              // We have a key not accessed by shift
          shift_status = SHIFT_OFF;
          pinMode(SHIFT_LOCK_WRITE, INPUT);  
        } else if (ctrs != NULL) {              // We have a control character i.e. RETURN
          shift_status = SHIFT_OFF;
          pinMode(SHIFT_LOCK_WRITE, INPUT);  // This removes the pin from the circuit. So it does not interferre with the return key.
        }
        /*
                if (isupper(ascii_key)) {
                  shift_status = SHIFT_ON;
                  pinMode(SHIFT_LOCK_WRITE, OUTPUT);
                  ascii_key += 32;
                } else {
                  shift_status = SHIFT_OFF;
                  pinMode(SHIFT_LOCK_WRITE, INPUT);  // This removes the pin from the circuit. So it does not interferre with the return key.;
                }
        */
        // Set the correct control pin on the CN2 and the correct pulse to wait for
        control_pin = letter[ascii_key][CONTROL_PIN];
        control_pulse = letter[ascii_key][CONTROL_PULSE];

        // Duplicate keys must be slowed down due to some engineered double type prevention in the typewriter.
        ascii_key_last = ascii_key;
        if (ascii_key_last == ascii_key) {
          delay(100);
        }

        // Go to key state and turn interrupt
        state = PRESS_KEY;
        attachInterrupt(digitalPinToInterrupt(START_SIGNAL), start_pulse_int, FALLING);
      }
      break;

    // Do nothing until triggered by start pulse
    case PRESS_KEY:
      break;

    // The zero pulse is a special case and must be handled with a hard delay as opposed from the other pu
    case ZERO_PULSE:
      delayMicroseconds(ZERO_PULSE_DELAY);
      digitalWrite(control_pin, HIGH);
      state = PRESS_KEY;
      attachInterrupt(digitalPinToInterrupt(START_SIGNAL), count_pulse_int, RISING);

      break;
  }
}
