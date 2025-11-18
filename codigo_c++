#include <LiquidCrystal_I2C.h>
const int buttonUp = 3;
const int buttonLeft = 2;
const int buttonDown = 5;
const int buttonRight = 4;
const int buttonShoot = 6;
int opcaoJogo = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    pinMode(buttonUp, INPUT_PULLUP);
    pinMode(buttonLeft, INPUT_PULLUP);
    pinMode(buttonRight, INPUT_PULLUP);
    pinMode(buttonDown, INPUT_PULLUP);
    pinMode(buttonShoot, INPUT_PULLUP);
 
  Serial.begin(9600);
  lcd.init();
  delay(100);
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Aperte o Botao para selecionar um jogo!");
}

void loop() {
  if(digitalRead(buttonLeft) == LOW){
	opcaoJogo = opcaoJogo - 1;

  }
  else if(digitalRead(buttonRight) == LOW){
  opcaoJogo = opcaoJogo + 1;
  }
  if(opcaoJogo<1){
  opcaoJogo = 3;
  }
  else if(opcaoJogo>3){
  opcaoJogo = 1;
  }
  if(opcaoJogo==1){
    lcd.clear();
    lcd.print("Pong");
  }
  else if(opcaoJogo==2){
    lcd.clear();
    lcd.print("Pac-Man");
  }
  else if(opcaoJogo==3){
  lcd.clear();
    lcd.print("Jogo da Cobrinha");
  }
}	
