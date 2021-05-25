#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"
 
// Create the lcd object address 0x3F and 16 columns x 2 rows 
LiquidCrystal_I2C lcd (0x27, 16,2);
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 13;
int boton_uno = 14;
int boton_dos = 16;
int pasos = 0;
int envase;
int muestra;

HX711 scale;
 
void  setup () {
  // Initialize the LCD connected 
  lcd.init();
  
  // Turn on the backlight on LCD. 
  lcd. backlight ();
  
   // Creamos una instancia de la clase WiFiManager
  WiFiManager wifiManager;
  // Descomentar para resetear configuración
  //wifiManager.resetSettings();
  // Cremos AP y portal cautivo
  wifiManager.autoConnect("GRANDUR-WIFI","Mtzt1585");

  // print the Message on the LCD. 
  lcd. print ( "BALANZA GRANDUR" );
  lcd. setCursor (0, 1);
  lcd. print ("Conexion Lista");

  Serial.begin(115200);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale. set_scale(1831.7);
  scale. tare();
  
  delay (3000);
}
 
void  loop () {
  /*if (scale.is_ready()) {
    Serial.print("HX711 reading: ");
    long lectura = scale.read();
    Serial.println(lectura);
  } else {
    Serial.println("HX711 not found.");
  }*/

  scale. set_scale(1831.7);
  int lectura = scale. get_units();
  int estado_uno = digitalRead (boton_uno);
  int estado_dos = digitalRead (boton_dos);
  
  if (estado_uno == LOW && estado_dos == LOW)
    {
      lcd.clear();
      lcd.print("Configurar WiFi")
      WiFiManager wifiManager;
      wifiManager.startConfigPortal("OnDemandAP"); 
    }
  else
    {
     do
      {
        delay(250);
        Serial.print(estado_uno);
        lcd.clear();
        lcd.print("Para comenzar");
        lcd.setCursor(0, 1);
        lcd.print("presione 1");
        estado_uno = digitalRead(boton_uno);        
      }while (estado_uno == HIGH);
     pasos++;
     Serial.print(pasos);
     do
      {
        delay(250); 
        lectura = scale.get_units();
        estado_uno = digitalRead(boton_uno);
        lcd.clear();
        lcd.print("Envase Vacio");
        lcd.setCursor(0, 1);
        lcd.print(lectura);
        lcd.setCursor(15,1);
        lcd.print("g");
      }while ((lectura == 0) || (estado_uno == HIGH));
     envase = lectura;
     Serial.println(envase);
     do
      {
        delay(250); 
        lectura = scale.get_units();
        estado_uno = digitalRead(boton_uno);
        lcd.clear();
        lcd.print("Llenar muestra");
        lcd.setCursor(0, 1);
        lcd.print(lectura);
        lcd.setCursor(15,1);
        lcd.print("g");
      }while ((lectura == 0) || (estado_uno == HIGH));
     muestra = lectura;
     Serial.println(muestra);

    }
  Serial.println("FIN");  
  delay (5000);
}
