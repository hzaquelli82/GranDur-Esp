#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"
#include <UbidotsMicroESP8266.h>

#define TOKEN "****"
#define G_0 "***" //ID Granulometría oculto
#define D_0 "***" //ID Durabilidad oculto
#define G_1 "***" //ID Granulometría visible
#define D_1 "***" //ID Durabilidad visible
 
// Create the lcd object address 0x3F and 16 columns x 2 rows 
LiquidCrystal_I2C lcd (0x27, 16,2);
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 13;
int boton_uno = 14;
int boton_dos = 16;
int pasos = 0;

HX711 scale;

Ubidots client(TOKEN);
 
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
  scale.set_scale(1831.7);
  scale.tare();
  
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
  bool salto = false;
  int envase = 0; //almacena el peso del envase vacío
  int muestra = 0; //almacena el peso de la muestra
  int granu = 0; //almacena el peso restante del tamizado
  float granu_porc; // porcentaje granulometria
  int durabilidad = 0; //almacena el peso restante luego del procedimiento de durabilidad
  float durabilidad_porc; // porcentaje durabilidad
  
  //Menú Inicial escondido
  if (estado_uno == LOW && estado_dos == LOW)
    {
      lcd.clear();
      lcd.print("Configurar WiFi");
      WiFiManager wifiManager;
      wifiManager.startConfigPortal("GRANDUR_WIFI","Mtzt1585"); 
    }
  else //Inicia procedimientos
    {
     // Para pedir inicio
     do
      {
        delay(250);
        //Serial.print(estado_uno);
        lcd.clear();
        lcd.print("Para comenzar");
        lcd.setCursor(0, 1);
        lcd.print("presione 1");
        estado_uno = digitalRead(boton_uno);        
      }while (estado_uno == HIGH);
     pasos++;
     Serial.print(pasos);
     delay(1000);

     // Para guardar tara de envase
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
     delay(1000);

     // Para guardar peso de muestra total
     do
      {
        delay(250); 
        lectura = scale.get_units();
        estado_uno = digitalRead(boton_uno);
        int m = lectura - envase;
        lcd.clear();
        lcd.print("Llenar muestra");
        lcd.setCursor(0, 1);
        lcd.print(m);
        lcd.setCursor(15,1);
        lcd.print("g");
        if (lectura > envase)
        {
          salto = true;
        }
      }while ((salto == false) || (estado_uno == HIGH));
     muestra = lectura;
     Serial.println(envase);
     Serial.println(muestra);
     delay(1000);

     // Pesar muestra tamizada para granulometría
     do
      {
        delay(250); 
        lectura = scale.get_units();
        estado_uno = digitalRead(boton_uno);
        int g = lectura - envase;
        lcd.clear();
        lcd.print("Muestra Tamizada");
        lcd.setCursor(0, 1);
        lcd.print(g);
        lcd.setCursor(15,1);
        lcd.print("g");
        if ((lectura > envase) && (lectura < muestra))
        {
          salto = true;
        }
      }while ((salto == false) || (estado_uno == HIGH));
     granu = lectura;
     Serial.println(envase);
     Serial.println(muestra);
     Serial.println(granu);
     granu_porc = porcentajeCalc(envase, muestra, granu);
     Serial.println(granu_porc);
     lcd.clear();
     lcd.print("Granulometria");
     lcd.setCursor(0, 1);
     lcd.print(granu_porc);
     lcd.setCursor(15, 1);
     lcd.print("%");
     client.add(G_0,granu_porc);
     client.sendAll(false); //enviar datos de manera oculta
     delay(5000);

     // Pesar muestra tamizada para durabilidad
     do
      {
        delay(250); 
        lectura = scale.get_units();
        estado_uno = digitalRead(boton_uno);
        int d = lectura - envase;
        lcd.clear();
        lcd.print("Muestra Tester");
        lcd.setCursor(0, 1);
        lcd.print(d);
        lcd.setCursor(15,1);
        lcd.print("g");
        if ((lectura > envase) && (lectura < muestra))
        {
          salto = true;
        }
      }while ((salto == false) || (estado_uno == HIGH));
     durabilidad = lectura;
     Serial.println(envase);
     Serial.println(muestra);
     Serial.println(granu);
     Serial.println(durabilidad);
     durabilidad_porc = porcentajeCalc(envase, granu, durabilidad);
     Serial.println(durabilidad_porc);
     lcd.clear();
     lcd.print("Durabilidad");
     lcd.setCursor(0, 1);
     lcd.print(durabilidad_porc);
     lcd.setCursor(15, 1);
     lcd.print("%");
     client.add(D_0,durabilidad_porc);
     client.sendAll(false);
     delay(5000);

    }
  
  // Muestra resultados y espera respuesta para enviar datos
  do
  {
    delay (250);
    estado_uno = digitalRead (boton_uno);
    estado_dos = digitalRead (boton_dos);
    lcd.clear();
    lcd.print("G=");
    lcd.setCursor(2, 0);
    lcd.print(granu_porc);
    lcd.setCursor(8, 0);
    lcd.print("D=");
    lcd.setCursor(10,0);
    lcd.print(durabilidad_porc);
    lcd.setCursor(0,1);
    lcd.print("Desea Enviar?");
  }while (estado_uno==HIGH && estado_dos==HIGH);
  if (estado_uno==0)
  {
    client.add(G_1,granu_porc);
    client.add(D_1,durabilidad_porc);
    client.sendAll(false);
    delay(3000);
    lcd.clear();
    lcd.print("Datos Enviados");
  }
  else
  {
    lcd.clear();
    lcd.print("Datos");
    lcd.setCursor(0,1);
    lcd.print("NO Enviados");
  }
  Serial.println("FIN");
  delay(5000);
}

float porcentajeCalc(int a,int b,int c)
{
  float x; //crea x
  float m; // crea m para calcular peso de muestra
  float g; // crea g para calcular peso de muestra tamizada
  m=b-a; // calcula el peso de la muestra
  g=c-a; // calcula el peso de la muestra tamizada
  x=(g/m)*100; // calcula el porcentaje
  return x; // devuelve el porcentaje calculado
}
