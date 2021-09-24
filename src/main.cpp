#include <Arduino.h>

// #include <CapacitiveSensor.h>

#include <Ethernet.h>

#include <Artnet.h>

#include <FastLED.h>

// wdt

#include <avr/wdt.h>

// FastLED
#define NUM_LEDS 144
CRGB leds[NUM_LEDS];
#define PIN_LED_DATA 3
bool doShow = false;

#define BUTTON_PIN 7
#define LED_PIN 6
#define ADDR_1 A1
#define ADDR_2 A2
#define ADDR_3 A3
#define ADDR_4 A4

uint8_t ledValue = 0;

Artnet artnet;
uint8_t universeReceive = 1; // 0 - 15
uint8_t universeSend = 2;

int addr;
const int ipOffset = 10;
// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
uint8_t mac[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB};
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip;

// CapacitiveSensor cs_6_7 = CapacitiveSensor(6, 7); // 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired
// long senseData = 0;
byte data[1];

unsigned long lastSend = 0;

void callback(const uint8_t *data, const uint16_t size)
{
    wdt_reset(); // reset wdt only if artnet is active.
    if (size >= NUM_LEDS * 3)
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
        if (size > NUM_LEDS * 3)
        {
            ledValue = data[NUM_LEDS * 3];
        }
        doShow = true;
        // Serial.println("-");
    }
}

void fastLedTest()
{
    // for (size_t pixel = 0; pixel < NUM_LEDS; ++pixel)
    // {
    //     leds[pixel].r = (pixel % 3 == 0) * 255;
    //     leds[pixel].g = (pixel % 3 == 1) * 255;
    //     leds[pixel].b = (pixel % 3 == 2) * 255;
    // }
    leds[0].r = 255;
    leds[0].g = 255;
    leds[0].b = 255;
    FastLED.show();
    // delay(500);
}

// software reset

void Reset_A(void) { asm volatile("jmp 0 \n"); }

void setup()
{
    wdt_enable(WDTO_4S);
    pinMode(ADDR_1, INPUT_PULLUP);
    pinMode(ADDR_2, INPUT_PULLUP);
    pinMode(ADDR_3, INPUT_PULLUP);
    pinMode(ADDR_4, INPUT_PULLUP);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    analogWrite(LED_PIN, 0);
    FastLED.addLeds<WS2812, PIN_LED_DATA, GRB>(leds, NUM_LEDS);
    // fastLedTest();
    addr = (digitalRead(ADDR_1) & 0x1) | ((digitalRead(ADDR_2) & 0x1) << 1) | ((digitalRead(ADDR_3) & 0x1) << 2) | ((digitalRead(ADDR_4) & 0x1) << 3);
    data[0] = ipOffset + addr;
    ip = IPAddress(192, 168, 0, ipOffset + addr);
    mac[5] = ipOffset + addr;
    Ethernet.begin(mac, ip);
    artnet.begin(); // waiting for Art-Net in default port
    // artnet.begin(net, subnet); // optionally you can set net and subnet here
    artnet.subscribe(0, callback);

    // cs_6_7.set_CS_AutocaL_Millis(0xFFFFFFFF); // turn off autocalibrate
    Serial.begin(115200);
    Serial.println(F("Starting"));
    Serial.println(ipOffset + addr);
}

void loop()
{
    // wdt_reset();
    if (millis() > 300000) //300000
    { // every 5min
        Serial.println("RESET ...");
        Reset_A();
    }
    if (doShow)
    {
        // Serial.println("*");
        FastLED.show();
        doShow = false;
    }
    analogWrite(LED_PIN, ledValue);
    artnet.parse();
    unsigned long now = millis();
    if (now > lastSend + 20)
    {
        lastSend = now;
        // senseData = cs_6_7.capacitiveSensorRaw(30);
        // Serial.print("\t T: ");
        // Serial.print(senseData);
        // Serial.print("\t");

        data[1] = digitalRead(7);
        // data[3] = senseData & 0xFF;
        // data[2] = (senseData >> 8) & 0xFF;
        // data[1] = (senseData >> 16) & 0xFF;
        // data[0] = (senseData >> 24) & 0xFF;
        // Serial.print(data[1]);
        // Serial.println("\t");
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

    artnet.streaming_data(data, 2);
    artnet.streaming("192.168.0.175", universeSend); // automatically send set data in 40fps (15bit universe)
    // artnet.streaming("127.0.0.1", net, subnet, univ); // or you can set net and subnet here
}