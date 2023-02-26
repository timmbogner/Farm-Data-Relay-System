
#ifdef USE_OLED
#include <ESP8266_and_ESP32_OLED_driver_for_SSD1306_displays/src/SSD1306Wire.h>

String debug_buffer[5] = {"", "", "", "", ""};
SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL); // ADDRESS, SDA, SCL

void draw_OLED_header()
{
    #ifdef FDRS_GATEWAY
    display.setFont(ArialMT_Plain_10);
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "MAC: " + String(UNIT_MAC, HEX));
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(63, 0, OLED_HEADER);
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(127, 0, "TBD");
    display.display();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    #endif
    #ifdef FDRS_NODE
    display.setFont(ArialMT_Plain_10);
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "ID: " + String(READING_ID));
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(63, 0, OLED_HEADER);
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(127, 0, "GW: " + String(GTWY_MAC, HEX));
    display.display();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    #endif
}

void debug_OLED(String debug_text)
{
    draw_OLED_header();
    display.drawHorizontalLine(0, 15, 128);
    display.drawHorizontalLine(0, 16, 128);

    for (uint8_t i = 4; i > 0; i--)
    {

        debug_buffer[i] = debug_buffer[i - 1];
    }
    debug_buffer[0] = String(millis() / 1000) + " " + debug_text;
    uint8_t lineNumber = 0;
    for (uint8_t i = 0; i < 5; i++)
    {
        uint8_t ret = display.FDRS_drawStringMaxWidth(0, 17 + (lineNumber * 9), 127, debug_buffer[i]);
        lineNumber = ret + lineNumber;
        if (lineNumber > 5)
            break;
    }
    display.display();
}
void init_oled(){
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(30);
  digitalWrite(OLED_RST, HIGH);
  Wire.begin(OLED_SDA, OLED_SCL);
  display.init();
  display.flipScreenVertically();
  draw_OLED_header();


  }
#endif
