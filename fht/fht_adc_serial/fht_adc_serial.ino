/*
fht_adc_serial.pde
guest openmusiclabs.com 7.7.14
example sketch for testing the fht library.
it takes in data on ADC0 (Analog0) and processes them
with the fht. the data is sent out over the serial
port at 115.2kb.
*/

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


#define LOG_OUT 1 // use the log output function
//#define LIN_OUT8 1
#define FHT_N 256 // set to 256 point fht
#define SCALE 32

#include <FHT.h> // include the library

void setup() {
  Serial.begin(115200); // use the serial port
 // set prescale to 16
 sbi(ADCSRA,ADPS2) ;
 sbi(ADCSRA,ADPS1) ;
 cbi(ADCSRA,ADPS0) ;
}

uint16_t buf[256];

void loop() {
  while(1) { // reduces jitter
    for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
      /*while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      //k -= 0x0200; // form into a signed int
      //k <<= 6; // form into a 16b signed int*/
      int k = 512-analogRead(A0);
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fht_input[i] = k ; // put real data into bins
      buf[i] = analogRead(A0); ;
    }
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_log(); // take the output of the fht
    //fht_mag_lin8(); // take the output of the fht
    Serial.print("start");
    //Serial.write((uint8_t*)(buf), 256); // send out the data
//    Serial.write((uint8_t*)(fht_lin_out8),256); // send out the data
    Serial.write((uint8_t*)(fht_log_out),256); // send out the data
    for (byte i = 0 ; i < FHT_N/2 ; i++) {
      //Serial.print(fht_lin_out8[i]); // send out the data
      //Serial.print(fht_log_out[i]); // send out the data
      //Serial.print((uint16_t)(fht_input[i])); // send out the data
      
    }
  }
}
