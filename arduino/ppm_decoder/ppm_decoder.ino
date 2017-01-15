// Configuration
#define ppmPin 2
#define NUM_CHANNELS 8

//For passing data between ISR and main loop
volatile uint16_t thread_channels[NUM_CHANNELS] = {0,0,0,0,0,0,0,0};
volatile bool new_data_avail = false;

//Setup input pin, ISR
void setup() {
  pinMode(ppmPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ppmPin), isr, FALLING);
}

//Gets new channel data from the ISR and prints it to the serial line.
void loop() {
  uint16_t channels_to_send[NUM_CHANNELS];
  if(new_data_avail) {
    memcpy(channels_to_send, (void*)thread_channels, NUM_CHANNELS * 2);
    new_data_avail = false;

    for(int i=0; i<NUM_CHANNELS; ++i) {
      Serial.print(channels_to_send[i]);
      Serial.print(",");
    }
    Serial.print("\r\n");
  }
}

//For use only by the ISR
uint16_t channels[NUM_CHANNELS] = {0,0,0,0,0,0,0,0};

//Get the time of the interrupt, calculate channel times and store.
void isr() {
  
  static uint16_t time_last_us = 0;
  static int chan_idx = 0;

  uint16_t time_us = micros();
  uint16_t chan_us = time_us - time_last_us;
  time_last_us = time_us;

  //Look for a start condition.
  if (chan_us > 4000) {

    //Copy the channel data over to the main loop, if its ready to consume new data
    if(!new_data_avail) {
      memcpy((void*)thread_channels, channels, NUM_CHANNELS * 2);
      new_data_avail = true;
    }

    //Reset the pointer to the start of the channels vector
    chan_idx = 0;
    return;
  }

  //Otherwise record each channel
  channels[chan_idx++] = chan_us;
  
  //Safety check
  if (chan_idx >= NUM_CHANNELS)
    chan_idx = 0;
}
