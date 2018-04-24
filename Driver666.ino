
#include "HX711.h"   //TO DO: Tryb ustawiania tensometru, funckja display
#include <Bridge.h>
#include <YunClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Ustawienie adresu ukladu na 0x27

#define calibration_factor 200.0 //This value is obtained using the SparkFun_HX711_Calibration sketch

#define DOUT  3
#define CLK  2

#define calib_step 500 //krok kalibracji
#define zakl 10 //zakl/kalb
#define buzz 8
#define kom 9
HX711 scale(DOUT, CLK);


float a; //pierwszy odczyt do porównania
float b; // drugi
int calib_time; //kalibracja w trybie kalibracji
int fast_calib_count; //kalibracja w locie
int calib_count;
int noise_flag=0;
float calib_out;
int adhoc_out; //kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
float tg; //tangens nachylenia
float device_weight; //waga worka i rurki
float drip_weight = 400; //waga startowa PŁYNU - to podaje pielęgniarka
float start_weight; //rzeczywista waga startowa
float fill_perc;

void lcdPrint(int timetoEnd){
  
  lcd.clear();
  lcd.setCursor(0,0); // Ustawienie kursora w pozycji 0,0 (pierwszy wiersz, pierwsza kolumna)
  lcd.print("TimetoEnd");
  lcd.setCursor(0,1); //Ustawienie kursora w pozycji 0,0 (drugi wiersz, pierwsza kolumnaa
  lcd.print(timetoEnd);
  //lcd.print(timetoEnd/60);
  //lcd.print(" min ");
  //lcd.print(timetoEnd % 60);
  //lcd.print(" sec");
}


void sendingData(int czas, int liczba){
  digitalWrite(kom,HIGH);
  YunClient client;
  client.connect("192.168.1.101",6666);
  
  client.println(liczba);

  client.println(czas);
  client.stop();
  digitalWrite(kom,LOW);
}

void lcdBlinking(int repeats){
  int i = 0;
  while( i < repeats){
    lcd.backlight(); // zalaczenie podwietlenia 
    delay(200);
    lcd.noBacklight();
    delay(200);
    i++;
  }
}



void setup() {
  pinMode(zakl, OUTPUT); //pin do diody sygnaliującej zaklocenia
  pinMode(buzz, OUTPUT); //buzzer
  pinMode(kom, OUTPUT);  //komunikacja
//  pinMode(kalib, OUTPUT); //kalibracja
 
  calib_count=20;
  fast_calib_count=2;
  calib_time=0;
  tg=0;
  Bridge.begin();
  //Serial.begin(9600);
  
  

  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare();	//Assuming there is no weight on the scale at start up, reset the scale to 0

 for(int i=0; i<1; i++){
digitalWrite(buzz, HIGH);
delay(500);
digitalWrite(buzz, LOW);
delay(500);
}
//lcd.begin(16,2);
//Serial.println("Setup finished");
     

//KALIBRACJA//   //Zebranie '20' par punktow, utworzenie '20' wsp kier prostych i ich usrednienie - trwa to 10 sek

  
    digitalWrite(zakl, HIGH);
  // Serial.println("Jeden");
    start_weight = scale.get_units();
    //Serial.println("Dwa");
    device_weight = start_weight - drip_weight;
    //Serial.println("Trzy");
    calib_time=calib_step;
    //Serial.println("Cztery");
    delay(calib_step);
    //Serial.println("Piec");
    for (int i=0; i < calib_count; i++)
    {
      tg=(tg+(scale.get_units()/calib_time))/(i+1);
      calib_time=calib_time + (i+1)*calib_time;         //Czy i+1 jest ok?
     // Serial.println("Szesc");
      delay(calib_step);
    }
    //Serial.println("siedem");
  digitalWrite(zakl, LOW);
  calib_out=device_weight/tg;
  
//Serial.println("po kalibracji");

 //KONIEC KALIBRACJI// 

}

void loop() {


  //WYZNACZANIE FUNKCJI//             czas = waga / tg
 
if (noise_flag==0)
{ 
for (int i=0; i < fast_calib_count; i++)
    {
      tg=(tg+(scale.get_units()/calib_time))/(i+1);
      calib_time=calib_time + (i+1)*calib_time;         //Czy i+1 jest ok?
      delay(calib_step);
    }
 adhoc_out=device_weight/tg;
 fill_perc = scale.get_units()/start_weight;
 //WYPISZ DO EKRANU
 //Serial.println("Przed sendingiem");
 //Serial.print("Czas do konca ");
 //Serial.println(adhoc_out);
 sendingData(adhoc_out, fill_perc);
 //Serial.println("po sendingu");
 lcdPrint(adhoc_out);
 //Serial.println("wypisano do ekranu");
}

 delay(1000);           //ponowne sprawdzenie zakłóceń po 1 sek

//Serial.println("przed kontrola");
 //KONTROLA ZAKLOCEN//
 noise_flag=0;
 a=scale.get_units();
 //Serial.print("a= ");
 //Serial.println(a);
 delay(500);
 b=scale.get_units();
// Serial.print("b= ");
 //Serial.println(b);
 digitalWrite(zakl, LOW);
 if (a>(b+10.0) or b>(a+5.0))
    {
      //Serial.println("zaklocenia");
    digitalWrite(zakl, HIGH);
    noise_flag = 1;
    //WYPISZ DO EKRANU WYNIK KALIBRACJI
    lcdPrint(calib_out);
    }
  // Serial.println("bez zaklocen");
 //KONIEC KONTROLI//

}
