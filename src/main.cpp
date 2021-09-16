#include <Arduino.h>

#include <CapacitiveSensor.h>

#include <Ethernet.h>

#include <Artnet.h>

#include <FastLED.h>

// FastLED
#define NUM_LEDS 144
CRGB leds[NUM_LEDS];
#define PIN_LED_DATA 3
bool doShow = false;

Artnet artnet;
uint8_t universeReceive = 1; // 0 - 15
uint8_t universeSend = 2;

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
uint8_t mac[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB};
// Set the static IP address to use if the DHCP fails to assign
const IPAddress ip(192, 168, 0, 14);

CapacitiveSensor cs_6_7 = CapacitiveSensor(6, 7); // 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired
long senseData = 0;
byte data[4];
unsigned long lastSend = 0;

void callback(const uint8_t *data, const uint16_t size)
{
    // you can also use pre-defined callbacks
    // Serial.print("Received data, size = ");
    // Serial.println(size);
    for (size_t pixel = 0; pixel < NUM_LEDS; ++pixel)
        {
            size_t idx = pixel * 3;
            leds[pixel].r = data[idx + 0];
            leds[pixel].g = data[idx + 1];
            leds[pixel].b = data[idx + 2];
        }
        doShow = true;
        // Serial.println("-");
}

void fastLedTest(){
    for (size_t pixel = 0; pixel < NUM_LEDS; ++pixel)
        {
            leds[pixel].r = (pixel%3 == 0)*255;
            leds[pixel].g = (pixel%3 == 1)*255;
            leds[pixel].b = (pixel%3 == 2)*255;
        }
        FastLED.show();
        delay(500);
}

void setup()
{
    FastLED.addLeds<WS2812, PIN_LED_DATA, GRB>(leds, NUM_LEDS);
    fastLedTest();
    Ethernet.begin(mac, ip);
    artnet.begin(); // waiting for Art-Net in default port
    // artnet.begin(net, subnet); // optionally you can set net and subnet here
    artnet.subscribe(universeReceive, callback);

    cs_6_7.set_CS_AutocaL_Millis(0xFFFFFFFF); // turn off autocalibrate
    Serial.begin(115200);
    Serial.println(F("Starting"));
}

void loop()
{
    if(doShow){
        // Serial.println("*");
        FastLED.show();
        doShow = false;
    }
    artnet.parse();
    unsigned long now = millis();
    if (now > lastSend + 20)
    {
        lastSend = now;
        senseData = cs_6_7.capacitiveSensorRaw(30);
        // Serial.print("\t T: ");
        // Serial.print(senseData);
        // Serial.print("\t");
        

        data[3] = senseData & 0xFF;        
        data[2] = (senseData >> 8) & 0xFF;  
        data[1] = (senseData >> 16) & 0xFF; 
        data[0] = (senseData >> 24) & 0xFF; 
        // Serial.print(data[0], BIN);
        // Serial.print("\t");
        // Serial.print(data[1], BIN);
        // Serial.print("\t");
        // Serial.print(data[2], BIN);
        // Serial.print("\t");
        // Serial.println(data[3], BIN);
                                    //    artnet.send("192.168.0.175", 2, data, 4);
    }

    // Serial.print("ms: ");
    // Serial.print(millis() - start);        // check on performance in milliseconds
    // Serial.print("\t T: ");                    // tab character for debug windown spacing

    // Serial.println(total1);                  // print sensor output 1

    artnet.streaming_data(data, 4);
    artnet.streaming("192.168.0.175", universeSend); // automatically send set data in 40fps (15bit universe)
    // artnet.streaming("127.0.0.1", net, subnet, univ); // or you can set net and subnet here
}