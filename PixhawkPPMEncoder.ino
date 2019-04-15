#define INPUT_PIN 2  //Interrupt supporting PINs only
#define NUM_CHANNELS 6

#define CHANNEL_WIDTH 15 //Maximum amount of channels PPM supports, should not be changed

unsigned long int time_falling, last_peak, pulse_time;
int pulse_buffer[CHANNEL_WIDTH], channel_buffer[CHANNEL_WIDTH], current_channel;

void setup() {
    Serial.begin(9600);

    // Setup interrupt
    pinMode(INPUT_PIN, INPUT_PULLUP);
    current_channel = 0;
    attachInterrupt(digitalPinToInterrupt(INPUT_PIN), read_me, FALLING);
}

void read_me() {

    //Calculate peak time
    time_falling = micros();
    pulse_time = time_falling - last_peak; 
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
    }
    Serial.print("\n");
    delay(100);
}

