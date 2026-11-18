
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ===== 1. DISPLAY CONFIGURATION =====

// MAX7219 Matrix
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define DATA_PIN 11
#define CS_PIN   10
#define CLK_PIN  9

MD_MAX72XX matrix(
  HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES
);

// OLED 128x64
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET
);

const int DISPLAY_WIDTH = 8;
const int DISPLAY_HEIGHT = 32;

// ===== OLED TEXT POSITIONS =====
// Y = vertical position (0 = top, text is 16px tall, max useful value is 48)
// X_OFFSET = shift from the centered position (+ moves right, - moves left)

// Game screen: "Level1:" and "Stack"
const int LEVEL_Y = 16;
const int LEVEL_X_OFFSET = 0;

const int STACK_Y = 40;
const int STACK_X_OFFSET = 0;

// Game over screen: "GAME OVER" and "Score"
const int GAMEOVER_Y = 16;
const int GAMEOVER_X_OFFSET = 0;

const int SCORE_Y = 40;
const int SCORE_X_OFFSET = 0;

// ===== 2. ORIENTATION SETTINGS =====

const bool FLIP_INSIDE_MODULE = true;
const bool FLIP_MODULE_ORDER = false;
const bool FLIP_X = false;

// ===== 3. JOYSTICK CONFIGURATION =====

const int BUTTON_PIN = 2;  // SW
const int VRX_PIN = A0;    // VRX
const int VRY_PIN = A1;    // VRY

// ===== 4. GAME SETTINGS =====

const int INITIAL_WIDTH = 4;
const int BLOCK_HEIGHT = 2;

const unsigned long INITIAL_SPEED = 250;
const unsigned long SPEED_STEP = 35;
const unsigned long MIN_SPEED = 65;

const int MAX_LEVELS = DISPLAY_HEIGHT / BLOCK_HEIGHT;

// ===== 5. GAME VARIABLES =====

int towerLeft[MAX_LEVELS];
int towerWidth[MAX_LEVELS];

int level = 0;
int movingX = 0;
int movingDirection = 1;
int movingWidth = INITIAL_WIDTH;

bool gameOver = false;

unsigned long lastMoveTime = 0;
unsigned long gameStartTime = 0;

// Function declarations
void finishGame();
void drawGame();
void startGame();
void updateGameDisplay();

// ===== 6. LIGHT ONE PIXEL =====

void setPixel(int y, int x) {

  if (y < 0 || y >= DISPLAY_HEIGHT ||
      x < 0 || x >= DISPLAY_WIDTH) {
    return;
  }

  int module = y / 8;
  int inside = y % 8;

  if (FLIP_INSIDE_MODULE) {
    inside = 7 - inside;
  }

  if (FLIP_MODULE_ORDER) {
    module = MAX_DEVICES - 1 - module;
  }

  int libCol = module * 8 + inside;
  int libRow = FLIP_X ? (7 - x) : x;

  matrix.setPoint(libRow, libCol, true);
}

// ===== 7. DRAW A RECTANGLE =====

void drawRectangle(int x, int bottomY, int width) {

  for (int y = bottomY; y < bottomY + BLOCK_HEIGHT; y++) {

    for (int col = x; col < x + width; col++) {

      setPixel(y, col);

    }
  }
}

// ===== 8. DRAW THE WHOLE GAME =====

void drawGame() {

  matrix.clear();

  for (int i = 0; i < level; i++) {

    drawRectangle(
      towerLeft[i],
      BLOCK_HEIGHT * i,
      towerWidth[i]
    );

  }

  if (!gameOver && level < MAX_LEVELS) {

    drawRectangle(
      movingX,
      BLOCK_HEIGHT * level,
      movingWidth
    );

  }
}

// ===== 9. OLED GAME SCREEN =====

// Draw one line of size-2 text centered horizontally at height y,
// then shifted left/right by xOffset.
// Uses a plain char array (not String) to save the Uno's limited RAM.
void drawCenteredText(const char* text, int y, int xOffset) {

  int textWidth = strlen(text) * 12;
  int xPos = (SCREEN_WIDTH - textWidth) / 2 + xOffset;

  if (xPos < 0) {
    xPos = 0;
  }

  display.setCursor(xPos, y);
  display.print(text);
}

void updateGameDisplay() {

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  char levelText[12];
  snprintf(levelText, sizeof(levelText), "Level%d:", level + 1);

  drawCenteredText(levelText, LEVEL_Y, LEVEL_X_OFFSET);
  drawCenteredText("Stack", STACK_Y, STACK_X_OFFSET);

  display.display();
}

// ===== 10. START THE GAME =====

void startGame() {

  level = 0;
  movingX = 0;
  movingDirection = 1;
  movingWidth = INITIAL_WIDTH;

  gameOver = false;

  lastMoveTime = millis();
  gameStartTime = millis();

  drawGame();
  updateGameDisplay();

  Serial.println("BLOCK STACKER");
  Serial.println("Game started!");
}

// ===== 11. CALCULATE MOVEMENT SPEED =====

unsigned long getMoveInterval() {

  unsigned long speedUp = level * SPEED_STEP;

  if (speedUp >= INITIAL_SPEED - MIN_SPEED) {
    return MIN_SPEED;
  }

  return INITIAL_SPEED - speedUp;
}

// ===== 12. MOVE THE BLOCK AUTOMATICALLY =====

void moveBlock() {

  movingX = movingX + movingDirection;

  if (movingX <= 0) {

    movingX = 0;
    movingDirection = 1;

  }

  int rightLimit = DISPLAY_WIDTH - movingWidth;

  if (movingX >= rightLimit) {

    movingX = rightLimit;
    movingDirection = -1;

  }
}

// ===== 13. PLACE THE BLOCK =====

void placeBlock() {

  if (level == 0) {

    towerLeft[level] = movingX;
    towerWidth[level] = movingWidth;

  }

  else {

    int previousLeft = towerLeft[level - 1];
    int previousRight = previousLeft + towerWidth[level - 1];

    int currentLeft = movingX;
    int currentRight = movingX + movingWidth;

    int overlapLeft =
      (currentLeft > previousLeft) ? currentLeft : previousLeft;

    int overlapRight =
      (currentRight < previousRight) ? currentRight : previousRight;

    int overlapWidth = overlapRight - overlapLeft;

    if (overlapWidth <= 0) {

      finishGame();
      return;

    }

    towerLeft[level] = overlapLeft;
    towerWidth[level] = overlapWidth;

    movingWidth = overlapWidth;

  }

  level++;

  if (level >= MAX_LEVELS) {

    finishGame();
    return;

  }

  movingX = 0;
  movingDirection = 1;

  lastMoveTime = millis();

  drawGame();
  updateGameDisplay();

  Serial.print("Blocks placed: ");
  Serial.println(level);
}

// ===== 14. END THE GAME AND CALCULATE SCORE =====

void finishGame() {

  gameOver = true;

  long totalWidth = 0;

  for (int i = 0; i < level; i++) {
    totalWidth += towerWidth[i];
  }

  int score = 0;

  if (level > 0) {

    score = totalWidth * 100L /
            (INITIAL_WIDTH * MAX_LEVELS);

  }

  drawGame();

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  char scoreText[14];
  snprintf(scoreText, sizeof(scoreText), "Score:%d%%", score);

  // GAME OVER on top, score below
  drawCenteredText("GAME OVER", GAMEOVER_Y, GAMEOVER_X_OFFSET);
  drawCenteredText(scoreText, SCORE_Y, SCORE_X_OFFSET);

  display.display();

  Serial.println("===== GAME OVER =====");

  Serial.print("Score: ");
  Serial.print(score);
  Serial.println("%");
}

// ===== 15. SETUP =====

void setup() {

  Serial.begin(9600);

  // Joystick button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize MAX7219
  matrix.begin();
  matrix.control(MD_MAX72XX::INTENSITY, 0);
  matrix.clear();

  // Initialize I2C on Arduino Uno
  // SDA = A4, SCL = A5
  Wire.begin();

  // Initialize OLED
  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        SCREEN_ADDRESS
      )) {

    Serial.println(F("SSD1306 allocation failed"));

    for (;;) {
    }

  }

  display.setRotation(2);

  startGame();
}

// ===== 16. MAIN LOOP =====

void loop() {

  static int lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;

  unsigned long debounceDelay = 50;

  int currentButtonState = digitalRead(BUTTON_PIN);

  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    static int stableButtonState = HIGH;

    if (currentButtonState != stableButtonState) {

      stableButtonState = currentButtonState;

      if (stableButtonState == LOW) {

        if (gameOver) {
          startGame();
        }

        else {
          placeBlock();
        }

      }
    }
  }

  lastButtonState = currentButtonState;

  if (gameOver) {
    return;
  }

  // Automatic block movement
  unsigned long now = millis();

  if (now - lastMoveTime >= getMoveInterval()) {

    lastMoveTime = now;

    moveBlock();
    drawGame();

  }
}