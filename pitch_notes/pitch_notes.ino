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

 // ADC configuration
 // set prescale to 16
 sbi(ADCSRA,ADPS2) ;
 sbi(ADCSRA,ADPS1) ;
 cbi(ADCSRA,ADPS0) ;

 pinMode(13, OUTPUT);
 pinMode(12, OUTPUT);
}


void detect_note(float& note, float& volume)
{
    cli();
    volume=0;
    for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
      int k = 512-analogRead(A0);
      volume += abs(k);
      k -= 0x0200;
      k <<= 6;
      fht_input[i] = k ; // put real data into bins
    }
    volume = volume/FHT_N;
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_log(); // take the output of the fht
    uint8_t maxval=0;
    int maxi=0;
    //for(int i=0;i<FHT_N/2;i++)
    for(int i=0;i<126;i++)
    {
      if(maxval<fht_log_out[i])
      {
        maxval=fht_log_out[i];
        maxi=i;
      }
    }
    sei();
    note=maxi;
    
    // Evaluate a window around the detected note
    float notef = 0;
    int accum=0;
    int win = 10;
    if((note>=win) &&(note<127-win))
    {
      for(int i=note-win;i<=note+win;i++)
      {
        int v= fht_log_out[i];
        accum += v;
      }
      float avg = accum/(win*2+1);
      accum=0;
      for(int i=note-win;i<=note+win;i++)
      {
        int v= fht_log_out[i]>avg?fht_log_out[i]-avg:0;
        notef +=v*i;
        accum += v;
      }
    }
    //note = notef/accum;
}


int current_note=0;
int note_duration=0;
int sequence[10];
int seq_index=0;
int pin13_state=LOW;
int pin12_state=LOW;

void loop() {

    int note;
    float notef;
    float volume;
    detect_note(notef, volume);
    note = (int)(notef);
    //Serial.print("\n");
    
    if((note>=10) &&(note<117))
    { Serial.print(" ");
      Serial.print(notef);
      Serial.print(" ");
      Serial.print(note);
      Serial.print(" ");
      Serial.print(volume);
      Serial.println(" ");
    }
    if(note<10)
    {
      // then it is not a note
      current_note=0;
      note_duration=0;
      return;
    }
    
    if(abs(current_note-note)<=1)
    {
      if(current_note==0)
        current_note=note;
      note_duration+=1;
    }
    else
    {
      current_note=note;
      note_duration=1;
    }

    if(note_duration==4)
    {
      sequence[seq_index]=current_note;
      seq_index=(seq_index+1)%10;
      if((abs(sequence[(10+seq_index-4)%10]-24)<=1) &&
         (abs(sequence[(10+seq_index-3)%10]-27)<=1) &&
         (abs(sequence[(10+seq_index-2)%10]-30)<=1) &&
         (abs(sequence[(10+seq_index-1)%10]-24)<=1))
         {
           Serial.println("SPELL!");
           if(pin13_state==LOW)
             pin13_state=HIGH;
           else
             pin13_state=LOW;
           digitalWrite(13, pin13_state);
         }
      if((abs(sequence[(10+seq_index-4)%10]-20)<=1) &&
         (abs(sequence[(10+seq_index-3)%10]-18)<=1) &&
         (abs(sequence[(10+seq_index-2)%10]-20)<=1) &&
         (abs(sequence[(10+seq_index-1)%10]-18)<=1))
         {
           Serial.println("SPELL!");
           if(pin12_state==LOW)
             pin12_state=HIGH;
           else
             pin12_state=LOW;
           digitalWrite(12, pin12_state);
         }
        for(int j=0;j<10;j++)
        {
          Serial.print(sequence[j]);
          if(j==(10+seq_index-1)%10)
            Serial.print("< ");
          else
            Serial.print("  ");
        }
        Serial.println("");
    }
    
     //Serial.print(current_note);
     //Serial.print("\t");
     //Serial.println(note_duration);
}
