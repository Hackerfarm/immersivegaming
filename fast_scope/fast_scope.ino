

byte lb, hb;
int sensorValue=0;

void setup() {
  Serial.begin(115200); // use the serial port
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
}

void loop() {
  while(1) { // reduces jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 256 ; i++) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      /*k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int*/
      sensorValue=k;
      Serial.println((int)(512-k));
      /*hb=highByte(sensorValue<<3); 
      // set 3 most significant bits and send out
      Serial.write(hb|0b11100000); 
      // select lower byte and clear 3 most significant bits
      lb=(lowByte(sensorValue))&0b00011111;
      // set bits 5 and 6 and send out
      Serial.write(lb|0b01100000);*/

    }
  }  
}
