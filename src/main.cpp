// IMPORTACIONES
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// INSTANCIAS
LiquidCrystal_I2C lcd(0x27,16,2);

// VARIABLES GLOBALES

byte set = 8, reset = 9, confirm = 10, rele = 13, echo = 3, trig = 2;
byte indice = 0;
bool modo = false;
unsigned int debounceTime = 100;

long duration;
float dactual, d1=0, d2=0;
int dactualInt, d1Int=0, d2Int=0;

bool programmed = false;

bool bandera = false;

void(* resetFunc) (void) = 0;

void pantallaPrincipal()
{
  lcd.clear();     
  lcd.setCursor(0, 0); 
  lcd.print("D1:");
  lcd.print(d1Int>9 ? d1Int : "0"+String(d1Int));
  lcd.print("CM");

  lcd.setCursor(0, 1);
  lcd.print("D2:");
  lcd.print(d2Int>9 ? d2Int : "0"+String(d2Int));
  lcd.print("CM");

  lcd.setCursor(9, 1);
  lcd.print("DA:  CM");
  
  lcd.setCursor(13,0);
  lcd.print("M:C");
}

void segundaPantalla()
{
  lcd.clear();     
  lcd.setCursor(3, 0); 
  lcd.print("DA:    CM");

  lcd.setCursor(0, 1);
  lcd.print("D1:");
  lcd.print(d1Int>9 ? d1Int : "0"+String(d1Int));
  lcd.print("CM");

  lcd.setCursor(9, 1);
  lcd.print("D2:");
  lcd.print(d2Int>9 ? d2Int : "0"+String(d2Int));
  lcd.print("CM");
}

void getDistance() 
{
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  duration = pulseIn(echo, HIGH);
  dactual = duration * 0.034 / 2;
  dactual = dactual > 99 ? 99 : dactual;
  dactualInt = int(dactual);
}

bool menu()
{
  while((indice != 0) | (!modo))
  {
    if(!digitalRead(confirm))
    {
      delay(debounceTime);
      indice = indice >= 2 ? 0 : indice+1;
    }

    lcd.setCursor(7,0); lcd.print(" ");
    lcd.setCursor(7,1); lcd.print(" ");
    lcd.setCursor(12,0); lcd.print(" ");

    switch(indice)
    {
      case 0: 
        lcd.setCursor(7,0); 
        lcd.print("<");
        if(!digitalRead(set))
        {
          d1 = dactual;
        }
        if(!digitalRead(reset))
        {
          d1 = 0;
        }
        d1Int = int(d1);
        break;
      case 1: 
        lcd.setCursor(7,1); 
        lcd.print("<"); 
        if(!digitalRead(set))
        {
          d2 = dactual;
        }
        if(!digitalRead(reset))
        {
          d2 = 0;
        }
        d2Int = int(d2);
        break;
      case 2: 
          lcd.setCursor(12,0); lcd.print(">");
          if(!digitalRead(set))
          {
            modo = true;
          }
          if(!digitalRead(reset))
          {
            modo = false;
          }
        break;
    }

    getDistance();
    lcd.setCursor(12,1);
    lcd.print(dactualInt < 9 ? "0"+String(dactualInt) : dactualInt);
    lcd.setCursor(3,0);
    lcd.print(d1Int < 9 ? "0"+String(d1Int) : d1Int);
    lcd.setCursor(3,1);
    lcd.print(d2Int < 9 ? "0"+String(d2Int) : d2Int);
    lcd.setCursor(15,0);
    lcd.print(modo ? "I":"C");
    lcd.display();
  }
  
  EEPROM.put(0,d1);
  EEPROM.put(4,d2);

  return true;
}

void setup() 
{
  Serial.begin(9600);

  pinMode(rele, OUTPUT);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(set, INPUT_PULLUP);
  pinMode(reset, INPUT_PULLUP);
  pinMode(confirm, INPUT_PULLUP);

  EEPROM.get(0, d1);
  EEPROM.get(4, d2);
  d1Int = int(d1);
  d2Int = int(d2);

  lcd.init();
  lcd.backlight();

  pantallaPrincipal();

  programmed = menu();

  segundaPantalla();
}

void loop() 
{
  if(!digitalRead(reset))
  {
    delay(debounceTime);
    resetFunc();
  }

  getDistance();
  lcd.setCursor(6,0);
  lcd.print(dactual >= 99.99 ? 99.99 : dactual);
  lcd.display();

  if(dactual > d2 || bandera)
  {
    digitalWrite(rele, HIGH);
    bandera = true;
    if(dactual < d1)
    {
      digitalWrite(rele, LOW);
      bandera = false;
    }
  }
}