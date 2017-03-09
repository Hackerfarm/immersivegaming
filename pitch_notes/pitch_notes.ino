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

const int NUM_SPELLS = 2;
int *spell_length;
int **spell_sequence;
char **spell_name;

int note_letter_to_int(char note)
{
  switch(note)
  {
    case 'F':
    return 17;
    break;
    
    case 'G':
    return 19;
    break;
    
    case 'A':
    break;
    case 'B':
    break;
    
    case 'C':
    return 23;
    break;
    
    case 'D':
    return 25;
    break;
    
    case 'E':
    return 28;
    break;
  }
}

void make_spell(int index, char* spellname, char* notes)
{
  delete spell_sequence[index];
  delete spell_name[index];
  spell_length[index] = strlen(notes);
  spell_name[index] = new char[strlen(spellname)+1];
  strcpy(spell_name[index], spellname);
  spell_sequence[index] = new int[strlen(notes)];
  for(int i=0;i<strlen(notes);i++)
    spell_sequence[index][i] = note_letter_to_int(notes[i]);
}

int recog_spell(int* sequence, int buffer_length, int current_ind)
{
  for(int i=0;i<NUM_SPELLS;i++)
  {
    bool fail = false;
    for(int j=0;j<spell_length[i];j++)
    {
      int note = sequence[(buffer_length+current_ind-spell_length[i]+j+1)%buffer_length];
      int spell_note = spell_sequence[i][j];
      if(abs(note-spell_note)>1)
      {
        fail=true;
        break;
      }
    }
    if(!fail)
      return i;
  }
  return -1;
}

void setup() {
  Serial.begin(115200); // use the serial port
  Serial.println("starting");
 // ADC configuration
 // set prescale to 16
 sbi(ADCSRA,ADPS2) ;
 sbi(ADCSRA,ADPS1) ;
 cbi(ADCSRA,ADPS0) ;

 pinMode(13, OUTPUT);
 pinMode(12, OUTPUT);
 


 // Spells initialization

 spell_length = new int[NUM_SPELLS];
 spell_sequence = new int*[NUM_SPELLS];
 spell_name = new char*[NUM_SPELLS];
 for(int i=0;i<NUM_SPELLS;i++)
 {
  spell_length[i]=0;
  spell_sequence[i] = NULL;
  spell_name[i] = NULL;
 }

 make_spell(0, "Red Led", "CDEC");
 make_spell(1, "Green Led", "FGFG");
 Serial.println("setup done");
}

int sound_buffer[FHT_N];

void detect_note(float& note, float& volume)
{
    cli();
    volume=0;
    for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
      sound_buffer[i] = analogRead(A0);
    }

    for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
      int k = 512-sound_buffer[i];
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
    
    /*// Evaluate a window around the detected note
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
    note = notef/accum;*/
}


int current_note=0;
int note_duration=0;
int cur_sequence[10];
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
    { 
      Serial.print(" ");
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
      cur_sequence[seq_index]=current_note;
      /*if((abs(cur_sequence[(10+seq_index-4)%10]-23)<=1) &&
         (abs(cur_sequence[(10+seq_index-3)%10]-25)<=1) &&
         (abs(cur_sequence[(10+seq_index-2)%10]-28)<=1) &&
         (abs(cur_sequence[(10+seq_index-1)%10]-23)<=1))
         {
           Serial.println("SPELL!");
           if(pin13_state==LOW)
             pin13_state=HIGH;
           else
             pin13_state=LOW;
           digitalWrite(13, pin13_state);
         }
      if((abs(cur_sequence[(10+seq_index-4)%10]-19)<=1) &&
         (abs(cur_sequence[(10+seq_index-3)%10]-17)<=1) &&
         (abs(cur_sequence[(10+seq_index-2)%10]-19)<=1) &&
         (abs(cur_sequence[(10+seq_index-1)%10]-17)<=1))
         {
           Serial.println("SPELL!");
           if(pin12_state==LOW)
             pin12_state=HIGH;
           else
             pin12_state=LOW;
           digitalWrite(12, pin12_state);
         }*/
        for(int j=0;j<10;j++)
        {
          Serial.print(cur_sequence[j]);
          if(j==(10+seq_index)%10)
            Serial.print("< ");
          else
            Serial.print("  ");
        }
        Serial.println("");
        int spell = recog_spell(cur_sequence, 10, seq_index);
        if(spell!=-1){
          Serial.println("SPELL!");
          Serial.println(spell_name[spell]);

          if(spell==0)
          {
            if(pin13_state==LOW)
             pin13_state=HIGH;
           else
             pin13_state=LOW;
           digitalWrite(13, pin13_state);
          }
          if(spell==1)
          {
            if(pin12_state==LOW)
             pin12_state=HIGH;
           else
             pin12_state=LOW;
           digitalWrite(12, pin12_state);
          }
        }
        seq_index=(seq_index+1)%10;
    }
    
     //Serial.print(current_note);
     //Serial.print("\t");
     //Serial.println(note_duration);
}
