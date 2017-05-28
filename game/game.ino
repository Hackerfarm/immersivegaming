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
#include <Adafruit_NeoPixel.h>


#define LED_PIN            6
#define MAX_LED_VALUE      64
#define NUM_LEDS           8

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(8, LED_PIN, NEO_GRB + NEO_KHZ800);

//// globals radio
//
int hgmPin = 22;
int ledPin = 18;
int vbatPin = 31;
int vsolPin = 29;
int led_state = 1;

#define NODE_ID 300
#define RX_BUFSIZE 1000
unsigned char radio_buf[RX_BUFSIZE];
//



//// globals game
//
int player_hp=6;
int player_max_hp=8;
//





void show_hp(int hp, int maxhp=NUM_LEDS)
{
    uint32_t green = pixels.Color(0,MAX_LED_VALUE,0);
    uint32_t red   = pixels.Color(MAX_LED_VALUE,0,0);
    uint32_t off   = pixels.Color(0,0,0);

    for(int i=0;i<hp;i++)
    {
        pixels.setPixelColor(i, green);
    }
    for(int i=hp;i<maxhp;i++)
    {
        pixels.setPixelColor(i, red);
    }
    for(int i=maxhp;i<NUM_LEDS;i++)
    {
        pixels.setPixelColor(i, off);
    }
    pixels.show();
}

void led_edit(uint32_t l0, uint32_t l1, uint32_t l2, uint32_t l3, uint32_t l4, uint32_t l5, uint32_t l6, uint32_t l7)
{
    pixels.setPixelColor(0, l0);
    pixels.setPixelColor(1, l1);
    pixels.setPixelColor(2, l2);
    pixels.setPixelColor(3, l3);
    pixels.setPixelColor(4, l4);
    pixels.setPixelColor(5, l5);
    pixels.setPixelColor(6, l6);
    pixels.setPixelColor(7, l7);
    pixels.show();
}


void setup() {
    pixels.begin(); // NeoPixel init
    Serial.begin(115200); // use the serial port
    Serial.println("starting");

    // ADC configuration
    // set prescale to 16
    sbi(ADCSRA,ADPS2) ;
    sbi(ADCSRA,ADPS1) ;
    cbi(ADCSRA,ADPS0) ;


    //// Spells initialization
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

    show_hp(4,8);
}



char sbuf[100];
typedef struct {
    float pitch;
    float volume;
    int duration;
} note_packet_t;

int ignore_note=0;
int ignore_timeout=0;
float ignore_volume;
int volume_total=0;
int note_duration=0;
int current_note=0;
int cur_sequence[10];
int cur_sequence_volume[10];
int seq_index=0;

int blink_state=0;
int blink_counter=0;
const uint32_t COLOR_BLACK = 0;
const uint32_t COLOR_RED =  pixels.Color(MAX_LED_VALUE, 0, 0);
const uint32_t COLOR_GREEN = pixels.Color(0, MAX_LED_VALUE, 0);
const uint32_t COLOR_BLUE = pixels.Color(0,0,MAX_LED_VALUE);

const int BLINK_WARNING_RED = 1;
const int BLINK_WARNING_GREEN = 2;

void loop() {
    if(blink_state!=0)
    {
        if(blink_state==BLINK_WARNING_RED)
        {
            blink_counter--;
            if(blink_counter%10<5)
            {
                led_edit(COLOR_BLACK, COLOR_RED,
                         COLOR_BLACK, COLOR_RED,
                         COLOR_BLACK, COLOR_RED,
                         COLOR_BLACK, COLOR_RED);
            }
            else
            {
                led_edit(COLOR_RED, COLOR_BLACK,
                         COLOR_RED, COLOR_BLACK,
                         COLOR_RED, COLOR_BLACK,
                         COLOR_RED, COLOR_BLACK);
            }
            if(blink_counter<0)
            {
                blink_state=0;
            }
        }
        if(blink_state==BLINK_WARNING_GREEN)
        {
            blink_counter--;
            if(blink_counter%10<5)
            {
                led_edit(COLOR_BLACK, COLOR_GREEN,
                         COLOR_BLACK, COLOR_GREEN,
                         COLOR_BLACK, COLOR_GREEN,
                         COLOR_BLACK, COLOR_GREEN);
            }
            else
            {
                led_edit(COLOR_GREEN, COLOR_BLACK,
                         COLOR_GREEN, COLOR_BLACK,
                         COLOR_GREEN, COLOR_BLACK,
                         COLOR_GREEN, COLOR_BLACK);
            }
            if(blink_counter<0)
            {
                blink_state=0;
            }
        }
    }
    else
    {
        show_hp(4,8);
    }



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
    }
    else
    {
        current_note=note;
        note_duration=1;
        volume_total=0;
    }

    if((note_duration==4))
    {
        if((ignore_note!=current_note)||(ignore_timeout==0)||(ignore_volume<volume_total/4.0f))
        {
            sprintf(sbuf, "note=%d volume=%d", note, (int)volume_total);
            note_packet_t packet;
            packet.pitch = note;
            packet.volume = volume_total;
            packet.duration = note_duration;
            Serial.println(sbuf);
            chibiTx(BROADCAST_ADDR, (unsigned char*)(&packet), sizeof(packet));


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
                    blink_counter = 200;
                    blink_state = BLINK_WARNING_RED;
                }
                if(spell==1)
                {
                    blink_counter = 200;
                    blink_state = BLINK_WARNING_GREEN;
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
            note_packet_t packet;
            packet = *((note_packet_t*)(radio_buf));
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

