#define INPUT_PIN 2  //Interrupt supporting PINs only


unsigned long int a, b, c;
int x[15], ch1[15], i;

void setup() {
    Serial.begin(9600);

    // Setup interrupt
    pinMode(INPUT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INPUT_PIN), read_me, FALLING);
}

void read_me() {
//this code reads value from RC reciever from PPM pin (Pin 2 or 3)
//this code gives channel values from 0-1000 values 
// -: ABHILASH :- //
    a = micros(); //store time value a when pin value falling
    c = a - b; //calculating time inbetween two peaks
    b = a; // 
    x[i] = c; //storing 15 value in array
    i = i + 1;
    if (i == 15) {
        for (int j = 0; j < 15; j++) { ch1[j] = x[j]; }
        i = 0;
    }
}//copy store all values from temporary array another array after 15 reading 

int *read_rc() {
    int i, j, k = 0;
    
    for (k = 14; k > -1; k--) {
        if (ch1[k] > 10000) { 
          j = k; 
        }
    }

    static int ch[7];
    
    for (i = 0; i <= 6; i++) {
        ch[i] = (ch1[i + 1 + j]);
    }

    return ch;
} 

void loop() {
    int *ch = read_rc();

    for(int i=0; i<7;i++) {
      Serial.print(ch[i]);
      Serial.print(", ");
    }
    Serial.print("\n");
    delay(100);
}

