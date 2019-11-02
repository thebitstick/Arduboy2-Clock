/*
   Clock.ino
   @TheBitStick
   Pocket Watch for Arduboy
   Uses Arduboy2, EEPROM, and 8x8DotJPFont libraries
*/

#include <Arduboy2.h>
#include <EEPROM.h>
#include <misaki_font.h>

Arduboy2 arduboy;

#define FRAME_RATE 15
#define MAX_HOUR 23 // maximum hours in a day
#define MAX_MINUTE 59 // maximum minutes in an hour
#define MAX_SECOND 59 // maximum seconds in a minute
#define CLOCK_HEIGHT HEIGHT / 2 // horizontal position of clock
#define CLOCK_WIDTH WIDTH / 4 // vertical position of clock
#define TIME_UPDATE 150 // perfect time for syncing event updates
#define REPEAT_DELAY 1000 // a second
#define ENABLE_EEPROM true // enable use of EEPROM to save hour:minute

PROGMEM const byte minute[] = { 0xf0, 0x48, 0x4a, 0xf1, 0x0f, 0xf0, 0x59, 0x61, 0x06, 0x46, 0x65, 0x79, 0x95, }; // うえ/した：いっぷん
PROGMEM const byte hour[] = { 0xf0, 0x81, 0x50, 0xf1, 0x0f, 0xf0, 0x74, 0x62, 0x8c, 0x06, 0x46, 0x63, 0x5a, 0x4d, 0x95, }; // みぎ/ひだり：いちじかん
PROGMEM const byte alarm[] = { 0xf1, 0x21, 0xf0, 0x06, 0x97, 0xde, 0xee, 0xd5, }; // A：アラーム
PROGMEM const byte freeze[] = { 0xf1, 0x22, 0xf0, 0x06, 0xca, 0xdf, 0xee, 0xaf, }; // B：フリーズ
PROGMEM const byte info[] = { 0xf0, 0xd2, 0xa6, 0xb8, 0xbd, 0x6a, 0x53, 0x46, }; // ポケットとけい

byte CLOCK_POSITION = CLOCK_WIDTH;
byte HOUR = 0;
byte MINUTE = 0;
byte SECOND = 0;
byte ALARM_HOUR = 0;
byte ALARM_MINUTE = 0;
bool ALARM_CHANGED = false;

// from emutyworks/8x8DotJPFont @ Github, file: 8x8DotJPFont/JPLang8x8Text/JPLang8x8Text.ino
// takes array of unsigned 8-bit integers that represent characters in special format
// to draw katakana and hiragana glyphs in Japanese
// edited to the bare essentials and edited with understandable variables
void drawText(byte x0, byte y0, byte *bitmap, byte size) {
  byte currentByte;
  byte byteType;

  for (byte i = 0; i < size; i++) {
    currentByte = pgm_read_byte_near(bitmap + i);

    switch (currentByte) {
      case MISAKI_FONT_F0_PAGE:
        byteType = MISAKI_FONT_F0_PAGE;
        continue;
      case MISAKI_FONT_F1_PAGE:
        byteType = MISAKI_FONT_F1_PAGE;
        continue;
    }

    switch (byteType) {
      case MISAKI_FONT_F0_PAGE:
        arduboy.drawBitmap(x0, y0,  misaki_font_f0[currentByte], MISAKI_FONT_F0_W, 8);
        x0 = x0 + 8;
        break;
      case MISAKI_FONT_F1_PAGE:
        arduboy.drawBitmap(x0, y0,  misaki_font_f1[currentByte], MISAKI_FONT_F1_W, 8);
        x0 = x0 + 4;
        break;
    }
  }
}

// saves identifier 'E', hour, then minute to EEPROM
void saveTime() {
  uint16_t address = EEPROM_STORAGE_SPACE_START;
  EEPROM.update(address++, 'E'); // E
  EEPROM.update(address++, HOUR);
  EEPROM.update(address++, MINUTE);
}

// loads hour and minute after ensuring identifier 'E'
void loadTime() {
  uint16_t address = EEPROM_STORAGE_SPACE_START;
  if (EEPROM.read(address++) == 'E') { // E
    HOUR = EEPROM.read(address++); // lets hope a byte is available
    MINUTE = EEPROM.read(address++); // lets hope a byte is available
  } else {
    saveTime();
  }
}

// updates the time accordingly
// increments seconds, then minutes, then hours
// and ensures not to go over MAX'es
void updateTime() {
  SECOND++;
  if (SECOND > MAX_SECOND) {
    SECOND = 0;
    MINUTE++;
    if (ENABLE_EEPROM) saveTime();
  }
  if (MINUTE > MAX_MINUTE) {
    MINUTE = 0;
    HOUR++;
  }
  if (HOUR > MAX_HOUR) HOUR = 0;
}

// sets cursor for updateScreen for clock
void moveCursor() {
  CLOCK_POSITION += 6; // best size for readability, 5 sometimes cuts off letters
  arduboy.setCursor(CLOCK_POSITION, CLOCK_HEIGHT);
}

// updates clock on screen
void updateScreen(byte _HOUR = HOUR, byte _MINUTE = MINUTE, byte _SECOND = SECOND) {
  arduboy.clear();
  CLOCK_POSITION = WIDTH / 4;
  moveCursor();

  // if the hour is less than 10, make space for a zero before the number
  if (_HOUR < 10) {
    arduboy.print(0);
    moveCursor();
  }
  arduboy.print(_HOUR);
  if (_HOUR >= 10) {
    moveCursor();
    moveCursor();
  }
  else {
    moveCursor();
  }

  arduboy.print(':');
  moveCursor();

  // if the minute is less than 10, make space for a zero before the number
  if (_MINUTE < 10) {
    arduboy.print(0);
    moveCursor();
  }
  arduboy.print(_MINUTE);
  if (_MINUTE >= 10) {
    moveCursor();
    moveCursor();
  } else {
    moveCursor();
  }

  arduboy.print(':');
  moveCursor();

  // if the second is less than 10, make space for a zero before the number
  if (_SECOND < 10) {
    arduboy.print(0);
    moveCursor();
  }
  arduboy.print(_SECOND);
}

// when updating time via buttons, reset seconds to zero
// then updateScreen with new time and delay by TIME_UPDATE
void resetSecondsOnEvent() {
  SECOND = 0;
  updateScreen();
  delay(TIME_UPDATE);
}

/*// returns true if its alarm time
  // returns false if not
  bool alarmTime() {
  if (!ALARM_CHANGED) return false;
  else return (ALARM_HOUR == HOUR && ALARM_MINUTE == MINUTE);
  }

  // MAKE SOME NOISE WOOOOOOO
  void alarm() {

  }*/

// follow me on my twitter dot com
void displayHelp() {
  arduboy.clear();
  drawText(CLOCK_WIDTH + 6, CLOCK_HEIGHT - 6, info, sizeof(info));
  arduboy.setCursor(CLOCK_WIDTH - 2, CLOCK_HEIGHT + 3);
  arduboy.print("@TheBitStick");
  updateTime();
  delay(REPEAT_DELAY);
}

// osd info
void cornerInfo() {
  drawText(0, 0, minute, sizeof(minute));
  drawText(0, 9, hour, sizeof(hour));
  drawText(0, HEIGHT - 7, alarm, sizeof(alarm));
  drawText(WIDTH / 1.51, HEIGHT - 7, freeze, sizeof(freeze));
}

// begins standard Arduboy cycle and sets framerate
// then loads time from EEPROM if allowed
void setup() {
  arduboy.begin();
  arduboy.setFrameRate(FRAME_RATE);
  uint16_t address = EEPROM_STORAGE_SPACE_START;
  if (ENABLE_EEPROM) loadTime();
}

// the main loop where everything happens
// event catching!
void loop() {
  if (!(arduboy.nextFrame())) return;

  // if holding Up & Left, displayHelp
  // else if holding Up & B, increment minutes
  // else if holding Down & B, decrement minutes
  if (!arduboy.pressed(A_BUTTON) && !arduboy.pressed(B_BUTTON) && arduboy.pressed(UP_BUTTON | LEFT_BUTTON))
    displayHelp();
  else if (!arduboy.pressed(A_BUTTON) && arduboy.pressed(B_BUTTON) && arduboy.pressed(UP_BUTTON)) {
    MINUTE++;
    resetSecondsOnEvent();
  }
  else if (!arduboy.pressed(A_BUTTON) && arduboy.pressed(B_BUTTON) && arduboy.pressed(DOWN_BUTTON)) {
    if (MINUTE > 0) {
      MINUTE--;
      resetSecondsOnEvent();
    }
  }

  // if holding Left & B, decrement hours
  // else if holding Right & B, increment hours
  if (!arduboy.pressed(A_BUTTON) && arduboy.pressed(B_BUTTON) && arduboy.pressed(LEFT_BUTTON)) {
    if (HOUR > 0) {
      HOUR--;
      resetSecondsOnEvent();
    }
  }
  else if (!arduboy.pressed(A_BUTTON) && arduboy.pressed(B_BUTTON) && arduboy.pressed(RIGHT_BUTTON)) {
    HOUR++;
    resetSecondsOnEvent();
  }

  // if holding Up & Left, displayHelp
  // else if holding A, then alarm stuff
  // if holding time modifiers, do similar tasks as above
  // else if NOT holding B, update the time
  if (!arduboy.pressed(A_BUTTON) && !arduboy.pressed(B_BUTTON) && arduboy.pressed(UP_BUTTON | LEFT_BUTTON))
    displayHelp();
  else if (arduboy.pressed(A_BUTTON) && arduboy.pressed(B_BUTTON)) {
    if (arduboy.pressed(UP_BUTTON)) {
      ALARM_MINUTE++;
      if (!ALARM_CHANGED) ALARM_CHANGED = true;
    }
    else if (arduboy.pressed(DOWN_BUTTON)) if (ALARM_MINUTE > 0) {
        ALARM_MINUTE--;
        if (!ALARM_CHANGED) ALARM_CHANGED = true;
      }
      else if (arduboy.pressed(LEFT_BUTTON)) if (ALARM_HOUR > 0) {
          ALARM_HOUR--;
          if (!ALARM_CHANGED) ALARM_CHANGED = true;
        }
        else if (arduboy.pressed(RIGHT_BUTTON)) {
          ALARM_HOUR++;
          if (!ALARM_CHANGED) ALARM_CHANGED = true;
        }
  }
  else if (arduboy.pressed(A_BUTTON))
    updateScreen(ALARM_HOUR, ALARM_MINUTE, 0);
  else if (!arduboy.pressed(B_BUTTON)) {
    updateScreen();
    updateTime();
    delay(REPEAT_DELAY);
  }

  cornerInfo();

  arduboy.display(); // display everything in the current loop
}
