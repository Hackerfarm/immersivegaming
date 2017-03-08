#define FASTADC 1

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


byte lb, hb;
int sensorValue=0;

void setup() {
 int start ;
 int i ;
 
#if FASTADC
 // set prescale to 16
 sbi(ADCSRA,ADPS2) ;
 cbi(ADCSRA,ADPS1) ;
 cbi(ADCSRA,ADPS0) ;
#endif

 Serial.begin(115200) ;
}


void loop() {
  // read A0:
  sensorValue = analogRead(A0);            
  // shift sample by 3 bits, and select higher byte  
  hb=highByte(sensorValue<<3); 
  // set 3 most significant bits and send out
  Serial.write(hb|0b11100000); 
  // select lower byte and clear 3 most significant bits
  lb=(lowByte(sensorValue))&0b00011111;
  // set bits 5 and 6 and send out
  Serial.write(lb|0b01100000);
}



