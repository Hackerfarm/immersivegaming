#ifndef NOTES_H
#define NOTES_H

#include <chibi.h>

#define LOG_OUT 1 // use the log output function
//#define LIN_OUT8 1
#define FHT_N 256 // set to 256 point fht
#define SCALE 32








//// globals spells
//
extern  int NUM_SPELLS;
extern  int *spell_length;
extern  int **spell_sequence;
extern  char **spell_name;
extern  int sound_buffer[FHT_N];
//


int note_letter_to_int(char note);
int recog_spell(int* sequence, int buffer_length, int current_ind);
void make_spell(int index, char* spellname, char* notes);
void detect_note(float& note, float& volume);

#endif // NOTES_H
