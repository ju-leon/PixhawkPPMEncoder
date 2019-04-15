
//CONFIG
#define INPUT_PIN 2  //Interrupt supporting PINs only
#define OUTPUT_PIN 8
#define NUM_CHANNELS 6

//TUNIG PARAMS
#define CHANNEL_WIDTH 15 //Maximum amount of channels PPM supports, should not be changed
#define ON_STATE 1  //set polarity of the pulses: 1 is positive, 0 is negative
#define DEFAULT_SERVO_VALUE 1500  //set the default servo value
#define PPM_FRAME_LENGTH 22500  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PPM_PULSE_LENGTH 300  //set the pulse length

unsigned long int last_peak;
int pulse_buffer[CHANNEL_WIDTH], channel_buffer[CHANNEL_WIDTH], current_channel;
int out_channels[NUM_CHANNELS];

// Setup PPM OUT Registers
void ppm_out_registers() {
    cli();
    TCCR1A = 0; // set entire TCCR1 register to 0
    TCCR1B = 0;
    OCR1A = 100;  // compare match register, change this
    TCCR1B |= (1 << WGM12);  // turn on CTC mode
    TCCR1B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
    TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
    sei();
}

void setup() {
    Serial.begin(9600);

    // Setup interrupt
    pinMode(INPUT_PIN, INPUT_PULLUP);
    current_channel = 0;
    attachInterrupt(digitalPinToInterrupt(INPUT_PIN), read_me, FALLING);

    //Setup PPM Write
    for(int i = 0; i < NUM_CHANNELS; i++){
        out_channels[i] = DEFAULT_SERVO_VALUE;
    }
    pinMode(OUTPUT_PIN, OUTPUT);
    digitalWrite(OUTPUT_PIN, !ON_STATE); 
    
    ppm_out_registers();
}

void read_me() {

    //Calculate peak time
    unsigned long int time_falling = micros();
    unsigned long int pulse_time = time_falling - last_peak; 
    last_peak = time_falling;

    // Sture current poulse into array
    pulse_buffer[current_channel++] = pulse_time;

    //Once all channels have been read, write back to channel buffer
    if (current_channel == CHANNEL_WIDTH) {
        for (int j = 0; j < CHANNEL_WIDTH; j++) { 
            channel_buffer[j] = pulse_buffer[j]; 
        }
        current_channel = 0;
    }
}

int *read_rc() {
    int offset = 0;

    //Calculate channel offset
    for (int k = CHANNEL_WIDTH - 1; k >= 0; k--) {
        if (channel_buffer[k] > 10000) { 
          offset = k; 
        }
    }

    static int ch[NUM_CHANNELS];

    // Write the channels into array
    for (int i = 0; i < NUM_CHANNELS; i++) {
        ch[i] = (channel_buffer[i + 1 + offset]);
    }

    return ch;
}

void loop() {
    int *ch = read_rc();

    for(int i=0; i<NUM_CHANNELS;i++) {
      Serial.print(ch[i]);
      Serial.print(", ");
      out_channels[i] =  ch[i];
    }
    Serial.print("\n");
    delay(10);
}


ISR(TIMER1_COMPA_vect){
  static boolean state = true;
  
  TCNT1 = 0;
  
  if(state) {  //start pulse
    digitalWrite(OUTPUT_PIN, ON_STATE);
    OCR1A = PPM_PULSE_LENGTH * 2;
    state = false;
  }
  else{  //end pulse and calculate when to start the next pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;
  
    digitalWrite(OUTPUT_PIN, !ON_STATE);
    state = true;

    if(cur_chan_numb >= NUM_CHANNELS){
      cur_chan_numb = 0;
      calc_rest = calc_rest + PPM_PULSE_LENGTH;
      OCR1A = (PPM_FRAME_LENGTH - calc_rest) * 2;
      calc_rest = 0;
    }
    else{
      OCR1A = (out_channels[cur_chan_numb] - PPM_PULSE_LENGTH) * 2;
      calc_rest = calc_rest + out_channels[cur_chan_numb];
      cur_chan_numb++;
    }     
  }
}

