#include <LiquidCrystal_I2C.h>
const int buttonLowerRight= 2;
const int buttonUpperRight = 3;
const int buttonLowerLeft = 4;
const int buttonUpperLeft = 5;
const int buttonSelect = 6;
int opcaoJogo = 0;
int ultimaOpcao = -1;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    pinMode(buttonLowerLeft, INPUT_PULLUP);
    pinMode(buttonUpperRight, INPUT_PULLUP);
    pinMode(buttonLowerLeft, INPUT_PULLUP);
    pinMode(buttonUpperLeft, INPUT_PULLUP);
    pinMode(buttonSelect, INPUT_PULLUP);
 
  Serial.begin(9600);
  lcd.init();
  delay(100);
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
}

void loop() {
  if(digitalRead(buttonUpperLeft) == LOW){
    opcaoJogo--;
    delay(200);
  }
  else if(digitalRead(buttonUpperRight) == LOW){
    opcaoJogo++;
    delay(200);
  }

  if(opcaoJogo < 1) opcaoJogo = 2;
  if(opcaoJogo > 2) opcaoJogo = 1;

  if (opcaoJogo != ultimaOpcao) {
    lcd.clear();  
    lcd.setCursor(0,0);

    if(opcaoJogo == 1){ 
      lcd.print("Pong: 2 Players");
      lcd.setCursor(0, 1);
lcd.print("Press Start");
    }
    else if(opcaoJogo == 2) {
      
      lcd.print("Snake: 1 Player");
     lcd.setCursor(0, 1);
lcd.print("Press Start");
    }

    ultimaOpcao = opcaoJogo;  
  }
}
