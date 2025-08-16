#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

// ===================== OLED =====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ===================== NeoPixel =================
#define NEOPIXEL_PIN 27
#define NUMPIXELS    16
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ===================== Joysticks =================
// Joystick 1 (Player 1)
#define JOY1_X   34  // ADC1
#define JOY1_Y   35  // ADC1
#define JOY1_SW  32  // digital (INPUT_PULLUP)

// Joystick 2 (Player 2)
#define JOY2_X   33  // ADC1
#define JOY2_Y   25  // ADC2 (ok in Wokwi; avoid WiFi when using ADC2)
#define JOY2_SW  26  // digital (INPUT_PULLUP)

// ===================== Buttons ==================
#define PAUSE_BTN 13   // Pause/Resume
#define RESET_BTN 4    // Reset score/game

// ===================== Buzzer (LEDC tone) =======
#define BUZZER_PIN 14
#define BUZZER_CH   0  // LEDC channel for tone

// ===================== EEPROM ===================
#define EEPROM_SIZE 64
const int HIGH_SCORE_ADDR = 0;

// ===================== Game/Menu =================
const char* games[] = {"Pong", "Snake", "Breakout", "Tetris"};
const int   GAME_COUNT = 4;

enum AppState { STATE_SPLASH, STATE_MENU, STATE_PLAYING };
AppState appState = STATE_SPLASH;

int selectedIndex = 0;
int highScore = 0;
int currentScore = 0;
bool paused = false;

// Timing
unsigned long lastFrameMs = 0;
const uint16_t FRAME_MS = 33; // ~30 FPS

// Joystick debounce-ish repeat timing
unsigned long lastNavMs = 0;
const uint16_t NAV_REPEAT_MS = 180;

// ===================== Helpers ==================
int  joy1X() { return analogRead(JOY1_X); }
int  joy1Y() { return analogRead(JOY1_Y); }
bool joy1Pressed() { return digitalRead(JOY1_SW) == LOW; }

int  joy2X() { return analogRead(JOY2_X); }
int  joy2Y() { return analogRead(JOY2_Y); }
bool joy2Pressed() { return digitalRead(JOY2_SW) == LOW; }

void toneBeep(int freq, int durationMs) {
  ledcAttachPin(BUZZER_PIN, BUZZER_CH);
  ledcWriteTone(BUZZER_CH, freq);
  delay(durationMs);
  ledcWriteTone(BUZZER_CH, 0);
}

void pixelsFill(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUMPIXELS; i++) pixels.setPixelColor(i, pixels.Color(r, g, b));
  pixels.show();
}

void rainbowAnimation(uint8_t cycles = 1, uint8_t wait = 10) {
  for (uint16_t j = 0; j < 256 * cycles; j++) {
    for (uint16_t i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color((i + j) & 255, (i * 5) & 255, (j * 2) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

// ===================== Screens ==================
void drawSplash() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 16);
  display.println("RETRO-ESP32");
  display.setTextSize(1);
  display.setCursor(20, 44);
  display.println("Press START (J1-SW)");
  display.display();

  rainbowAnimation(1, 8);
  toneBeep(1200, 150);
}

void drawMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Select Game (J1-Y / J1-SW)");

  for (int i = 0; i < GAME_COUNT; i++) {
    if (i == selectedIndex) {
      // Invert selection
      display.fillRect(0, 14 + i * 12, SCREEN_WIDTH, 12, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(4, 16 + i * 12);
      display.println(games[i]);
      display.setTextColor(SSD1306_WHITE);
    } else {
      display.setCursor(4, 16 + i * 12);
      display.println(games[i]);
    }
  }
  display.display();
}

void drawHUD() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Game: ");
  display.println(games[selectedIndex]);
  display.setCursor(0, 12);
  display.print("Score: ");
  display.println(currentScore);
  display.setCursor(0, 24);
  display.print("High:  ");
  display.println(highScore);
  display.setCursor(0, 36);
  display.print("Pause:");
  display.println(paused ? "YES" : "NO");
  display.setCursor(0, 48);
  display.println("P13=Pause  RST=G4");
  display.display();
}

// ===================== App Logic =================
void enterMenu() {
  appState = STATE_MENU;
  pixelsFill(0, 0, 32);
  drawMenu();
}

void startGame() {
  appState = STATE_PLAYING;
  currentScore = 0;
  pixelsFill(0, 0, 64); // blue tint on start
  toneBeep(900, 80);
  toneBeep(1100, 80);
}

void endGame() {
  // High score check
  if (currentScore > highScore) {
    highScore = currentScore;
    EEPROM.write(HIGH_SCORE_ADDR, highScore);
    EEPROM.commit();
    pixelsFill(255, 215, 0); // gold
    toneBeep(1400, 200);
  } else {
    pixelsFill(128, 0, 0); // red
    toneBeep(400, 300);
  }
  delay(700);
  enterMenu();
}

void handleMenuInput() {
  int y = joy1Y();
  unsigned long now = millis();

  // Navigate with J1-Y (center ~2048). Use deadband & repeat timer.
  if (now - lastNavMs > NAV_REPEAT_MS) {
    if (y < 1500) { // up
      selectedIndex = (selectedIndex - 1 + GAME_COUNT) % GAME_COUNT;
      lastNavMs = now;
      toneBeep(1200, 40);
      drawMenu();
    } else if (y > 2600) { // down
      selectedIndex = (selectedIndex + 1) % GAME_COUNT;
      lastNavMs = now;
      toneBeep(1200, 40);
      drawMenu();
    }
  }

  // Select with J1-SW
  if (joy1Pressed()) {
    toneBeep(1500, 90);
    startGame();
    delay(200); // basic debounce
  }
}

void handlePlayingInput() {
  // Pause toggle
  if (digitalRead(PAUSE_BTN) == LOW) {
    paused = !paused;
    toneBeep(paused ? 700 : 1000, 100);
    delay(200);
  }

  // Reset
  if (digitalRead(RESET_BTN) == LOW) {
    currentScore = 0;
    pixels.clear(); pixels.show();
    toneBeep(500, 150);
    delay(300);
  }
}

void gameLoopStep() {
  // This is where you plug your actual game logic.
  // For now, we simulate a score increase and simple feedback.

  // Example use of both joysticks (read but not used visually here)
  int p1x = joy1X();
  int p1y = joy1Y();
  bool p1sw = joy1Pressed();
  int p2x = joy2X();
  int p2y = joy2Y();
  bool p2sw = joy2Pressed();

  // Simulate scoring
  currentScore++;

  // Feedback every 10 points
  if (currentScore % 10 == 0) {
    pixelsFill(0, 200, 0); // green pulse
    toneBeep(1200, 60);
    pixelsFill(0, 0, 64);
  }

  // End game sample condition (just for demo)
  if (currentScore >= 200) endGame();
}

// ===================== Setup / Loop ===============
void setup() {
  Serial.begin(115200);

  // Inputs
  pinMode(JOY1_SW, INPUT_PULLUP);
  pinMode(JOY2_SW, INPUT_PULLUP);
  pinMode(PAUSE_BTN, INPUT_PULLUP);
  pinMode(RESET_BTN, INPUT_PULLUP);

  // Buzzer
  ledcSetup(BUZZER_CH, 2000, 8); // base config; freq changes per beep
  ledcAttachPin(BUZZER_PIN, BUZZER_CH);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }
  display.clearDisplay();
  display.display();

  // NeoPixel
  pixels.begin();
  pixels.setBrightness(60);
  pixels.clear();
  pixels.show();

  // EEPROM
  EEPROM.begin(EEPROM_SIZE);
  highScore = EEPROM.read(HIGH_SCORE_ADDR);

  // Splash then menu
  drawSplash();
  enterMenu();
}

void loop() {
  switch (appState) {
    case STATE_MENU:
      handleMenuInput();
      break;

    case STATE_PLAYING:
      handlePlayingInput();

      if (!paused) {
        unsigned long now = millis();
        if (now - lastFrameMs >= FRAME_MS) {
          lastFrameMs = now;
          gameLoopStep();
          drawHUD();
        }
      } else {
        drawHUD();
        delay(60);
      }
      break;

    case STATE_SPLASH:
    default:
      enterMenu();
      break;
  }
}
