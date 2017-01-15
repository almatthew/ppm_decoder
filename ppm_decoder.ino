// Configuration
#define ppmPin 2

#define NUM_CHANNELS 16
unsigned long channels[NUM_CHANNELS];
unsigned long channels_thread_copy[NUM_CHANNELS];
bool new_data_ready = false;
bool saw_first_sync = false;

void setup() {
  memset(channels, 0, NUM_CHANNELS);
  pinMode(ppmPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ppmPin), isr, FALLING);
}

void loop() {
  if(new_data_ready) {
    unsigned long channels_to_send[NUM_CHANNELS];
    memcpy(channels_to_send, channels_thread_copy, NUM_CHANNELS);
    new_data_ready = false;

    //Send data over the UART
    Serial.print(micros());
    for(int i=0; i<NUM_CHANNELS; ++i) {
      Serial.print(channels_to_send[i]);
      Serial.print(" ");
    }
    Serial.print("\n");
  }
}

void isr() {
  unsigned long time_us = micros();
  static unsigned long time_last_us = 0;
  static unsigned long *p_chan = channels;

  //Look for a start
  if(time_us - time_last_us > 4000) {

    //If we've already seen a sync byte, the last set of channels should be good to go
    if(saw_first_sync) {
      //A bit of thread-safety here, make sure we don't overwrite data
      //that hasn't been consumed by the main loop
      if(!new_data_ready) {
        memcpy(channels_thread_copy, channels, NUM_CHANNELS);
        new_data_ready = true;
      }
    }

    if(!saw_first_sync)
      saw_first_sync = true;
    
    //Reset the pointer to the start of the channels vector
    p_chan = channels;
    time_last_us = time_us;

    return;
  }

  //If we haven't seen a sync byte, just wait until we do
  if(!saw_first_sync)
    return;

  //Otherwise record each channel
  *p_chan++ = (time_us - time_last_us);
  time_last_us = time_us;

  //Safety check
  if(p_chan > (channels + NUM_CHANNELS - 1))
    p_chan = channels;
}
