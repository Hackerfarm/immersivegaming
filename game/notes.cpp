#include "notes.h"

#include <FHT.h>
int NUM_SPELLS = 3;
int *spell_length;
int **spell_sequence;
char **spell_name;
int sound_buffer[FHT_N];

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
}
