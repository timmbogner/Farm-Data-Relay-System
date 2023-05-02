

#include <ESP8266_and_ESP32_OLED_driver_for_SSD1306_displays/src/SSD1306Wire.h>
#define DISPLAY_PAGES 4

String debug_buffer[5] = {"", "", "", "", ""};
SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL); // ADDRESS, SDA, SCL
unsigned long displayEvent = 0;
uint8_t displayPage = 0;

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
    display.drawHorizontalLine(0, 15, 128);
    display.drawHorizontalLine(0, 16, 128);
}

void drawDebugPage() {
    draw_OLED_header();
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

void debug_OLED(String debug_text)
{
    displayEvent = millis()/1000; // Display Event is tracked in units of seconds
    displayPage = 0;
    display.clear();

    for (uint8_t i = 4; i > 0; i--)
    {

        debug_buffer[i] = debug_buffer[i - 1];
    }
    debug_buffer[0] = String(millis() / 1000) + " " + debug_text;
    drawDebugPage();
}

void drawBlankPage() {
    display.clear();
    display.display();
}

void drawStatusPage() {
    // draw_OLED_header();
    // display.FDRS_drawStringMaxWidth(0, 17, 127, "Status Page 1 " + String(millis()/1000));
    // display.display();
}

void drawPage2() {
    // draw_OLED_header();
    // display.FDRS_drawStringMaxWidth(0, 17, 127, "Page 2 " + String(millis()/1000));
    // display.display();
}

void drawPage3() {
    // draw_OLED_header();
    // display.FDRS_drawStringMaxWidth(0, 17, 127, "Page 3 " + String(millis()/1000));
    // display.display();
}

// write display content to display buffer
// nextpage = true -> flip 1 page
// When debug info comes in then switch to debug page
// after 60 seconds switch to blank page to save screen
void drawPageOLED(bool nextpage) {

    if((millis()/1000 - displayEvent) > OLED_PAGE_SECS && nextpage) {
        displayPage = (displayPage >= DISPLAY_PAGES) ? 0 : (displayPage + 1);
        displayEvent = millis()/1000;
        display.clear();

        switch(displayPage) {

        // page 0: debug output
        // page 1: gateway/node status
        // page 2: to be defined
        // page 3: to be defined
        // page 4: blank (screen saver)

        case 0: // display debug output
            drawDebugPage();
            break;
        case 1: // gateway/node status
            // drawStatusPage();
            // break;
        case 2: // to be defined later
            // drawPage2();
            // break;
        case 3: // to be defined later
            // drawPage3();
            // break;
        case 4: // Blank page
            drawBlankPage();
            break;
        default: // Blank page
            drawBlankPage();
            break;
        }
    }
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