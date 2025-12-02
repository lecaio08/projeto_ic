#include <LiquidCrystal_I2C.h>

// setando a pinagem dos 4 botões para controle e 1 botão pro select
const int buttonLowerRight = 2;
const int buttonUpperRight = 3;
const int buttonLowerLeft  = 4;
const int buttonUpperLeft  = 5;
const int buttonSelect     = 6;

// configuração do painel de LCD baseada no tinkercad. provavelmente será necessário modificar.
LiquidCrystal_I2C lcd(0x27, 16, 2);

/* 
   - definindo a matriz p/ armazenar a representação gráfica antes de enviar pro LCD. 
   - partindo a largura de 16 em 4 blocos de 4 colunas por byte. 
   - cada elemento graphicRam[a][b] é um uint8 q armazena quatro valores de 2 bits (um por coluna dentro do grupo) para uma linha b. 
   - ou seja: cada byte guarda 4 * 2bits = 8bits = estados para 4 colunas naquela linha. 
*/
uint8_t graphicRamPong[16/4][4];

/*
    - as funções são chamadas pra auxiliar o jogo PONG.
    - a gClearPong() limpa tudo que estiver no painel de LCD antes do jogo começar. ref.: https://man7.org/linux/man-pages/man3/memset.3.html
    - funcionamento da gAddPong():
      - x >> 2: operação bitwise de deslocar 2 bits para a direita, o mesmo que dividir por 4, para encontrar em qual índice do array graphicRamPong a coluna x está.
      - x & 3:  "e" lógico bitwise com 3 (11 em binario), o mesmo que pegar o resto da divisão por 4, para descobrir a posição exata dentro do byte (0, 1, 2 ou 3).
      - * 2:    multiplica por 2 pq cada "pixel" ocupa 2 bits de informação.
      - <<:     move o valor do item para a posição correta.
      - |=:     "Ou" lógico bitwsie, ddiciona o novo pixel sem apagar os que já existem naquele byte.
    - funcionamento da gFlushPong():
      - o proposito dessa função é renderizar o buffer de vídeo virtual (graphicRamPong) pro LCD, traduzindo 16x4 (virtual) -> 16x2 (físico).
      - o LCD possui apenas 2 linhas físicas, p/ simular 4 linhas de jogo, esta função agrupa duas linhas virtuais em uma única célula física (caractere 5x8).
      - a linha física 0 exibe: linha virtual 0 (metade superior) + linha virtual 1 (metade inferior).
      - a linha física 1 exibe: linha virtual 2 (metade superior) + linha virtual 3 (metade inferior).
      - iteração e varredura:
        - a função percorre as 16 colunas do display (eixo X) e as 2 linhas físicas (eixo Y físico).
        - para cada coordenada (x, r), ela precisa determinar qual caractere desenhar.
      - etapa 1: extração de dados (bitwise unpacking)
        - como a memória (graphicRamPong) usa compactação de bits (4 colunas por byte), os dados não podem ser lidos diretamente.
        - é necessário isolar os 2 bits da "metade de cima" (up) e os 2 bits da "metade de baixo" (lo).
        - cálculo da metade superior (up):
          - indexação do byte: `x >> 2` (divisão por 4) seleciona qual byte do array contém a coluna atual.
          - seleção da linha virtual: `r * 2` seleciona a linha virtual par (0 ou 2) correspondente à linha física atual.
          - alinhamento de bits: `>> ((x & 3) * 2)` usa o resto da divisão (`x & 3`) para saber a posição dentro do byte e desloca os bits para a direita. multiplica-se por 2 porque cada pixel ocupa 2 bits.
          - mascaramento: `& 3` (binário 00000011) zera todos os bits exceto os dois últimos, extraindo o valor limpo.
        - cálculo da metade inferior (lo):
          - utiliza a mesma lógica de bitwise shift e mask acima.
          - a diferença é o acesso à memória: usa `r * 2 + 1` para pegar a linha virtual ímpar (a parte de baixo da célula física).
      - etapa 2: lógica de renderização e linearização
        - posiciona o cursor do LCD na coordenada física (x, r).
        - caso vazio (limpeza):
          - se `up == 0` e `lo == 0`, desenha-se um caractere de espaço (' ').
          - isso é crucial para apagar "rastros" de objetos que se moveram.
        - caso preenchido (cálculo do caractere):
          - se houver pixels em `up` ou `lo`, é necessário descobrir qual dos 8 caracteres customizados (0-7) representa essa combinação visual.
          - origem da matemática:
            - no setup(), os caracteres foram gerados iterando um índice 'i' e usando aritmética de base 3.
            - `up` era o quociente de `(i + 1) / 3`.
            - `lo` era o resto de `(i + 1) % 3`.
          - aplicação da inversa (a fórmula):
            - para renderizar, fazemos o caminho inverso: transformamos os componentes (up, lo) de volta num índice linear.
            - fórmula da divisão euclidiana inversa: `dividendo = (divisor * quociente) + resto`.
            - aplicando ao contexto: `(indice + 1) = (3 * up) + lo`.
            - isolando o índice: `indice = (up * 3) + lo - 1`.
        - segurança (clamping):
          - aplica-se uma verificação final para garantir que o índice calculado não exceda 7.
          - isso previne acesso a endereços de memória inválidos no controlador do LCD.
*/
void gClearPong() {
  memset(graphicRamPong, 0, sizeof(graphicRamPong));
}

void gAddPong(uint8_t x, uint8_t y, uint8_t item) {
  graphicRamPong[x >> 2][y] |= ((uint8_t)item) << ((x & 3) * 2);
}

void gFlushPong() {
  for (uint8_t x = 0; x < 16; x++) {
    for (uint8_t r = 0; r < 2; r++) {
      uint8_t up = (graphicRamPong[x>>2][r*2] >> ((x&3)*2)) & 3; // pq usar uint8? fixar um inteiro de apenas 1 byte. economizar memoria.
      uint8_t lo = (graphicRamPong[x>>2][r*2+1] >> ((x&3)*2)) & 3;

      lcd.setCursor(x, r);

      if (up == 0 && lo == 0) lcd.write(' ');
      else {
        uint8_t g = up*3 + lo - 1;
        if (g > 7) g = 7;
        lcd.write(g);
      }
    }
  }
}

uint8_t graphicRamSnake[4][4];

void gClearSnake() {
  memset(graphicRamSnake, 0, sizeof(graphicRamSnake));
}

void gAddSnake(uint8_t x, uint8_t y, uint8_t item) {
  graphicRamSnake[x>>2][y] |= item << ((x&3)*2);
}

void gFlushSnake() {
  for (uint8_t x=0; x<16; x++) {
    for (uint8_t r=0; r<2; r++) {
      uint8_t u = (graphicRamSnake[x>>2][r*2] >> ((x&3)*2)) & 3;
      uint8_t l = (graphicRamSnake[x>>2][r*2+1] >> ((x&3)*2)) & 3;

      lcd.setCursor(x, r);

      if (u == 0 && l == 0) lcd.write(' ');
      else lcd.write((uint8_t)(u*3 + l - 1));
    }
  }
}

int escolherJogo() {

  int opcao  = 1;
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
      if (opcao == 1) lcd.print("PONG");
      if (opcao == 2) lcd.print("SNAKE");

      lcd.setCursor(2,1);
      lcd.print("PRESS START");

      ultima = opcao;
    }

    if (digitalRead(buttonSelect) == LOW) {
      delay(200);
      return opcao;
    }
  }
}

void pong_game() {

  const uint8_t W = 16;
  const uint8_t H = 4;

  // glyphs
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

  const unsigned long PHYS_INTERVAL = 35;
  unsigned long lastFrame = millis();

  lcd.clear();
  lcd.setCursor(3,0); lcd.print("PONG 16x4");
  lcd.setCursor(2,1); lcd.print("Press START");

  while (digitalRead(buttonSelect) != LOW);
  delay(200);

  while (true) {

    if (digitalRead(buttonSelect) == LOW) { delay(200); return; }

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

      if (ballX == P1X+1) {
        if (ballY == leftY)     { dx = 1; dy = -1; }
        else if (ballY == leftY+1) { dx = 1; dy = 1; }
      }

      if (ballX == P2X-1) {
        if (ballY == rightY)     { dx = -1; dy = -1; }
        else if (ballY == rightY+1) { dx = -1; dy = 1; }
      }

      if (ballX < LEFT_LIMIT || ballX > RIGHT_LIMIT) {
        lcd.clear();
        lcd.setCursor(4,0); lcd.print("POINT!");
        delay(500);

        ballX = 8; ballY = 1;
        dx = (random(0,2)==0)?1:-1;
        dy = (random(0,2)==0)?1:-1;
      }
    }

    gClearPong();
    gAddPong(P1X, leftY, 1);
    gAddPong(P1X, leftY+1, 1);

    gAddPong(P2X, rightY, 1);
    gAddPong(P2X, rightY+1, 1);

    gAddPong(ballX, ballY, 1);

    gFlushPong();
  }
}

struct Pos { uint8_t x,y; };
Pos snake[64];
int lenSnake;
int dirSnake;
Pos foodSnake;

void newFoodSnake() {
  const uint8_t W = 16;
  const uint8_t H = 4;

  while (true) {
    bool ok=true;
    foodSnake.x = random(0,W);
    foodSnake.y = random(0,H);
    for (int i=0;i<lenSnake;i++)
      if (snake[i].x==foodSnake.x && snake[i].y==foodSnake.y) ok=false;
    if (ok) return;
  }
}

void resetSnake() {
  lenSnake = 4;
  dirSnake = 1;

  snake[0]={3,1};
  snake[1]={2,1};
  snake[2]={1,1};
  snake[3]={0,1};

  newFoodSnake();
}

void snake_game() {

  const uint8_t W = 16;
  const uint8_t H = 4;

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

  lcd.clear();
  lcd.setCursor(3,0); lcd.print("SNAKE LCD");
  lcd.setCursor(2,1); lcd.print("Press START");

  while (digitalRead(buttonSelect)!=LOW);
  delay(200);

  resetSnake();

  unsigned long last = millis();
  unsigned long speed = 200;

  while (true) {

    if (digitalRead(buttonUpperLeft)==LOW)  dirSnake = (dirSnake+3)%4;
    if (digitalRead(buttonUpperRight)==LOW) dirSnake = (dirSnake+1)%4;

    if (millis()-last >= speed) {
      last=millis();

      for (int i=lenSnake;i>0;i--) snake[i]=snake[i-1];

      if (dirSnake==0) snake[0].y--;
      if (dirSnake==1) snake[0].x++;
      if (dirSnake==2) snake[0].y++;
      if (dirSnake==3) snake[0].x--;

      if (snake[0].x>=W || snake[0].y>=H) break;
      if (snake[0].x<0  || snake[0].y<0) break;

      for (int i=1;i<lenSnake;i++)
        if (snake[0].x==snake[i].x && snake[0].y==snake[i].y) break;

      if (snake[0].x==foodSnake.x && snake[0].y==foodSnake.y) {
        lenSnake++;
        if (speed>60) speed-=8;
        newFoodSnake();
      }

      gClearSnake();
      for (int i=0;i<lenSnake;i++) gAddSnake(snake[i].x,snake[i].y,1);
      gAddSnake(foodSnake.x,foodSnake.y,2);
      gFlushSnake();
    }
  }

  lcd.clear();
  lcd.setCursor(2,0); lcd.print("GAME OVER");
  lcd.setCursor(3,1); lcd.print("LEN:");
  lcd.print(lenSnake);
  delay(1200);
}

void setup() {

  pinMode(buttonLowerRight, INPUT_PULLUP);
  pinMode(buttonUpperRight, INPUT_PULLUP);
  pinMode(buttonLowerLeft,  INPUT_PULLUP);
  pinMode(buttonUpperLeft,  INPUT_PULLUP);
  pinMode(buttonSelect,     INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

}

void loop() {

  int j = escolherJogo();

  if (j==1) pong_game();
  if (j==2) snake_game();

  delay(200);

}
