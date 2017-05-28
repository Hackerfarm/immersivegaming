// defines for setting and clearing register bits (for FTH)
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


#include "notes.h"
#include <chibi.h>
#include <Wire.h>
#include <stdint.h>

#define LED_PIN            6
#define MAX_LED_VALUE      64
#define NUM_LEDS           8

//// globals radio
//
int hgmPin = 22;

#define RX_BUFSIZE 1000
unsigned char radio_buf[RX_BUFSIZE];
//


void setup() {
    Serial.begin(115200); // use the serial port
    Serial.println("starting");

    // ADC configuration
    // set prescale to 16
    sbi(ADCSRA,ADPS2) ;
    sbi(ADCSRA,ADPS1) ;
    cbi(ADCSRA,ADPS0) ;



    //// Initialize radio
    // set up high gain mode pin
    pinMode(hgmPin, OUTPUT);
    digitalWrite(hgmPin, LOW);
    // Initialize the chibi command line and set the speed to 57600 bps
    chibiCmdInit(115200);
    // Initialize the chibi wireless stack
    chibiInit();
    // high gain mode
    digitalWrite(hgmPin, HIGH);

    Serial.println("setup done");

}



char sbuf[100];
typedef struct {
    float pitch;
    float volume;
    int duration;
} game_packet_t;

int ignore_note=0;
int ignore_timeout=0;
float ignore_volume;
int volume_total=0;
int note_duration=0;
int current_note=0;
int cur_sequence[10];
int cur_sequence_volume[10];
int seq_index=0;

void loop() {
    int note;
    float notef;
    float volume;
    detect_note(notef, volume);
    note = (int)(notef);
    
    if(note<10)
    {
        // then it is not a note
        current_note=0;
        note_duration=0;
        return;
    }

    if(ignore_timeout>0)
    {
        ignore_timeout--;
    }
    
    if(abs(current_note-note)<=1)
    {
        if(current_note==0)
            current_note=note;
        note_duration+=1;
        if(note_duration<=4)
            volume_total+=volume;
        sprintf(sbuf, "note=%d volume=%d duration=%d", note, (int)volume_total, note_duration);
        game_packet_t packet;
        packet.pitch = note;
        packet.volume = volume_total;
        packet.duration = note_duration;
        Serial.println(sbuf);
        chibiTx(BROADCAST_ADDR, (unsigned char*)(&packet), sizeof(packet));
    }
    else
    {
        current_note=note;
        note_duration=1;
        volume_total=0;
    }

    if((note_duration==4))
    {
        {


            cur_sequence[seq_index]=current_note;
            cur_sequence_volume[seq_index]=volume_total/4.0f;
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
                }
                if(spell==1)
                {
                }
            }
            seq_index=(seq_index+1)%10;
        }
    }

    // Radio receive
    if (chibiDataRcvd() == true)
    {
        int rssi, src_addr, len;
        len = chibiGetData(radio_buf);
        if (len == 0) {
            Serial.println("Null packet received");
            return;
        }

        // retrieve the data and the signal strength
        rssi = chibiGetRSSI();
        src_addr = chibiGetSrcAddr();
        Serial.print(src_addr);
        Serial.print("  Signal strength: ");
        Serial.print(rssi);
        Serial.print("\t");
        if (len)
        {
            game_packet_t packet;
            packet = *((game_packet_t*)(radio_buf));
            Serial.println((char*)(radio_buf));
            sprintf(sbuf, "Decoded note=%d volume=%d", (int)packet.pitch, (int)packet.volume);
            Serial.println(sbuf);

            ignore_note = packet.pitch;
            ignore_timeout = 8;
            ignore_volume = packet.volume/packet.duration;
            if((cur_sequence[(seq_index-1)%10]==ignore_note)&&
               (ignore_volume>cur_sequence_volume[(seq_index-1)%10]))
            {
                cur_sequence_volume[(seq_index-1)%10]=0;
                seq_index=(seq_index-1)%10;
            }
        }
    }
}

