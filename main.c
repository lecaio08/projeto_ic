#include <LiquidCrystal_I2C.h>

// =========================
// ===== CARACTERES RAW ====
// =========================

byte paddleTop[8] = {
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte paddleBottom[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111
};

byte ballChar[8] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B00100,
  B00000,
  B00000,
  B00000
};

byte headChar[8] = {
  B01110,
  B11111,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};

byte bodyChar[8] = {
  B00100,
  B01110,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};

byte tailChar[8] = {
  B00000,
  B00000,
  B00100,
  B01110,
  B11111,
  B01110,
  B00100,
  B00000
};

byte fruitChar[8] = {
  B00100,
  B01110,
  B11111,
  B10101,
  B11111,
  B01110,
  B00100,
  B00000
};

// =========================
// ======= BOTÕES ==========
// =========================

const int buttonLowerRight = 2;
const int buttonUpperRight = 3;
const int buttonLowerLeft  = 4;
const int buttonUpperLeft  = 5;
const int buttonSelect     = 6;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ====================================
// =========== MENU SIMPLE ============
// ====================================
int escolherJogo() {

  int opcao = 1;
  int ultima = -1;

  while (true) {

    if (digitalRead(buttonUpperLeft) == LOW) {
      opcao--; if (opcao < 1) opcao = 2;
      delay(150);
    }

    if (digitalRead(buttonUpperRight) == LOW) {
      opcao++; if (opcao > 2) opcao = 1;
      delay(150);
    }

    if (opcao != ultima) {
      lcd.clear();
      lcd.setCursor(5,0);
      if (opcao == 1) lcd.print("Pong");
      if (opcao == 2) lcd.print("Snake");

      lcd.setCursor(2,1);
      lcd.print("Press START");

      ultima = opcao;
    }

    if (digitalRead(buttonSelect) == LOW) {
      delay(200);
      return opcao;
    }
  }
}

// ====================================
// ============= PONG =================
// ====================================
void jogo1() {

  const uint8_t W = 16;
  const uint8_t H = 4;

  uint8_t graphicRam[16/4][4];

  auto gClear = [&]() { memset(graphicRam, 0, sizeof(graphicRam)); };

  auto gAdd = [&](uint8_t x, uint8_t y, uint8_t item) {
    graphicRam[x >> 2][y] |= ((uint8_t)item) << ((x & 3) * 2);
  };

  auto gFlush = [&]() {
    for (uint8_t x = 0; x < 16; x++) {
      for (uint8_t r = 0; r < 2; r++) {

        uint8_t up = (graphicRam[x>>2][r*2] >> ((x&3)*2)) & 3;
        uint8_t lo = (graphicRam[x>>2][r*2+1] >> ((x&3)*2)) & 3;

        lcd.setCursor(x, r);

        if (up == 0 && lo == 0) lcd.write(' ');
        else {
          uint8_t g = up*3 + lo - 1;
          if (g > 7) g = 7;
          lcd.write(g);
        }
      }
    }
  };

  // ==== Gera glyphs (paddles + bola) ====
  byte block3[3] = { B01110, B01110, B01110 };
  byte glyph[8][8];

  for (uint8_t i = 0; i < 8; i++) {
    memset(glyph[i], 0, 8);
    int up = (i+1)/3;
    int lo = (i+1)%3;

    if (up == 1) memcpy(&glyph[i][0], block3, 3);
    if (lo == 1) memcpy(&glyph[i][4], block3, 3);

    lcd.createChar(i, glyph[i]);
  }

  // Estado do Pong
  int leftY  = 1;
  int rightY = 1;
  int ballX  = 8;
  int ballY  = 1;
  int dx = 1;
  int dy = 1;

  const int LEFT_LIMIT  = 3;
  const int RIGHT_LIMIT = 12;

  const int P1X = LEFT_LIMIT;
  const int P2X = RIGHT_LIMIT;

  // FPS do Pong
  const unsigned long PHYS_INTERVAL = 35;
  unsigned long lastFrame = millis();

  lcd.clear();
  lcd.setCursor(3,0); lcd.print("PONG 16x4");
  lcd.setCursor(2,1); lcd.print("Press START");

  while (digitalRead(buttonSelect) != LOW);
  delay(200);

  while (true) {

    if (digitalRead(buttonSelect) == LOW) { delay(200); return; }

    // ======= INPUT sem delay ======
    if (digitalRead(buttonUpperLeft) == LOW && leftY > 0) leftY--;
    if (digitalRead(buttonLowerLeft) == LOW && leftY < 2) leftY++;

    if (digitalRead(buttonUpperRight) == LOW && rightY > 0) rightY--;
    if (digitalRead(buttonLowerRight) == LOW && rightY < 2) rightY++;

    if (millis() - lastFrame >= PHYS_INTERVAL) {
      lastFrame = millis();

      ballX += dx;
      ballY += dy;

      if (ballY < 0) { ballY = 0; dy = 1; }
      if (ballY > 3) { ballY = 3; dy = -1; }

      // Paddle esquerdo
      if (ballX == P1X+1) {
        if (ballY == leftY)     { dx = 1; dy = -1; }
        else if (ballY == leftY+1) { dx = 1; dy = 1; }
      }

      // Paddle direito
      if (ballX == P2X-1) {
        if (ballY == rightY)     { dx = -1; dy = -1; }
        else if (ballY == rightY+1) { dx = -1; dy = 1; }
      }

      // Gol
      if (ballX < LEFT_LIMIT || ballX > RIGHT_LIMIT) {
        lcd.clear();
        lcd.setCursor(4,0); lcd.print("POINT!");
        delay(500);

        ballX = 8; ballY = 1;
        dx = (random(0,2)==0)?1:-1;
        dy = (random(0,2)==0)?1:-1;
      }
    }

    // ======= render =======
    gClear();

    gAdd(P1X, leftY, 1);
    gAdd(P1X, leftY+1, 1);

    gAdd(P2X, rightY, 1);
    gAdd(P2X, rightY+1, 1);

    gAdd(ballX, ballY, 1);

    gFlush();
  }
}



// =====================================
// ============== SNAKE ================
// =====================================
void jogo2() {

  const uint8_t W = 16;
  const uint8_t H = 4;

  uint8_t graphicRam[4][4];

  auto gClear  = [&](){ memset(graphicRam, 0, sizeof(graphicRam)); };
  auto gAdd    = [&](uint8_t x, uint8_t y, uint8_t item){
    graphicRam[x>>2][y] |= item << ((x&3)*2);
  };

  auto gFlush = [&]() {
    for (uint8_t x=0; x<16; x++) {
      for (uint8_t r=0; r<2; r++) {
        uint8_t u = (graphicRam[x>>2][r*2] >> ((x&3)*2)) & 3;
        uint8_t l = (graphicRam[x>>2][r*2+1] >> ((x&3)*2)) & 3;

        lcd.setCursor(x, r);

        if (u == 0 && l == 0) lcd.write(' ');
        else lcd.write((uint8_t)(u*3 + l - 1));
      }
    }
  };

  // ===== glyphs snake =====
  byte block[3] = { B01110, B01110, B01110 };
  byte apple[3] = { B00100, B01010, B00100 };

  for (uint8_t i=0;i<8;i++){
    byte g[8];
    memset(g,0,8);

    int up = (i+1)/3;
    int lo = (i+1)%3;

    if (up==1) memcpy(&g[0], block, 3);
    if (up==2) memcpy(&g[0], apple, 3);

    if (lo==1) memcpy(&g[4], block, 3);
    if (lo==2) memcpy(&g[4], apple, 3);

    lcd.createChar(i, g);
  }

  struct Pos { uint8_t x,y; };
  Pos snake[64];
  int len;
  int dir;

  Pos food;

  auto newFood = [&]() {
    while (true) {
      bool ok=true;
      food.x = random(0,W);
      food.y = random(0,H);
      for (int i=0;i<len;i++)
        if (snake[i].x==food.x && snake[i].y==food.y) ok=false;
      if (ok) return;
    }
  };

  auto reset = [&]() {
    len = 4;
    dir = 2; // direita

    snake[0]={3,1};
    snake[1]={2,1};
    snake[2]={1,1};
    snake[3]={0,1};

    newFood();
  };

  lcd.clear();
  lcd.setCursor(3,0); lcd.print("SNAKE LCD");
  lcd.setCursor(2,1); lcd.print("Press START");

  while (digitalRead(buttonSelect)!=LOW);
  delay(200);

  reset();

  unsigned long last = millis();
  unsigned long speed = 140;

  while (true) {

    // controles
    if (digitalRead(buttonUpperLeft)==LOW)  dir = (dir+3)%4;
    if (digitalRead(buttonUpperRight)==LOW) dir = (dir+1)%4;

    if (millis()-last >= speed) {
      last=millis();

      for (int i=len;i>0;i--) snake[i]=snake[i-1];

      if (dir==0) snake[0].y--;
      if (dir==1) snake[0].x++;
      if (dir==2) snake[0].y++;
      if (dir==3) snake[0].x--;

      if (snake[0].x>=W || snake[0].y>=H) break;
      if (snake[0].x<0  || snake[0].y<0) break;

      for (int i=1;i<len;i++)
        if (snake[0].x==snake[i].x && snake[0].y==snake[i].y) break;

      if (snake[0].x==food.x && snake[0].y==food.y) {
        len++;
        if (speed>60) speed-=8;
        newFood();
      }

      gClear();
      for (int i=0;i<len;i++) gAdd(snake[i].x,snake[i].y,1);
      gAdd(food.x,food.y,2);
      gFlush();
    }
  }

  lcd.clear();
  lcd.setCursor(2,0); lcd.print("GAME OVER");
  lcd.setCursor(3,1); lcd.print("LEN:");
  lcd.print(len);
  delay(1200);
}


// =====================================
// ============== SETUP ================
// =====================================
void setup() {

  pinMode(buttonLowerRight, INPUT_PULLUP);
  pinMode(buttonUpperRight, INPUT_PULLUP);
  pinMode(buttonLowerLeft,  INPUT_PULLUP);
  pinMode(buttonUpperLeft,  INPUT_PULLUP);
  pinMode(buttonSelect,     INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
}

// =====================================
// =============== LOOP ================
// =====================================
void loop() {

  int j = escolherJogo();

  if (j==1) jogo1();
  if (j==2) jogo2();

  delay(200);
}
