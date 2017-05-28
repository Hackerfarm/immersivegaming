// defines for setting and clearing register bits (for FTH)
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


#include <chibi.h>
#include <Wire.h>
#include <stdint.h>


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



void setup() {
    Serial.begin(115200); // use the serial port
    Serial.println("starting");

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
    int type;       // 0:note, 1:spell
    float pitch;
    float volume;
    int duration;
} game_packet_t;


void loop() {
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
        if (len)
        {
            game_packet_t packet;
            packet = *((game_packet_t*)(radio_buf));
            if(packet.type==0)
            {
                sprintf(sbuf, "note addr=%d \t sstrength=%d \t note=%d \t volume=%d",
                        (int)(src_addr), (int)rssi, (int)packet.pitch, (int)packet.volume);
                Serial.println(sbuf);
            }
            else
            {
                sprintf(sbuf, "spell addr=%d \t sstrength=%d \t spell=%d",
                        (int)(src_addr), (int)rssi, (int)packet.duration);
                Serial.println(sbuf);
            }

        }
    }
}

