#include <LiquidCrystal_I2C.h>

const int buttonLowerRight = 2;
const int buttonUpperRight = 3;
const int buttonLowerLeft  = 4;
const int buttonUpperLeft  = 5;
const int buttonSelect     = 6;

int opcaoJogo              = 0;
int ultimaOpcao            = -1;

LiquidCrystal_I2C lcd(0x27, 16, 2);

int escolherJogo() {

    int opcao  = 1;
    int ultima = -1;

    while (true) {

        if (digitalRead(buttonUpperLeft) == LOW) {
            opcao--;
            if (opcao < 1) opcao = 2;
            delay(200);
        }

        if (digitalRead(buttonUpperRight) == LOW) {
            opcao++;
            if (opcao > 2) opcao = 1;
            delay(200);
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
            return opcao;  // <-- devolve a escolha
        }
    }
}
void jogo1() {

    lcd.clear();
  	lcd.setCursor(1,0);
    lcd.print("Iniciou Pong!");
    delay(1000);

}

void jogo2() {

    lcd.clear();
  	lcd.setCursor(1,0);
    lcd.print("Iniciou Snake!");
    delay(1000);

}

void setup() {

    pinMode(buttonLowerLeft,  INPUT_PULLUP);
    pinMode(buttonUpperRight, INPUT_PULLUP);
    pinMode(buttonLowerLeft,  INPUT_PULLUP);
    pinMode(buttonUpperLeft,  INPUT_PULLUP);
    pinMode(buttonSelect,     INPUT_PULLUP);
 
    Serial.begin(9600);
    lcd.init();
    delay(100);
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0, 1);

}

void loop() {

    int jogo = escolherJogo();  // <-- CHAMA O MENU

    if (jogo == 1) jogo1();
    if (jogo == 2) jogo2();

    delay(2000);
    
}
