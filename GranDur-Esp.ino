#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"
#include <UbidotsMicroESP8266.h>

#define TOKEN "xxxxx"
#define G_0 "xxxx" //ID Granulometría oculto
#define D_0 "xxxxx" //ID Durabilidad oculto
#define G_1 "xxxxxx" //ID Granulometría visible
#define D_1 "xxxxxx" //ID Durabilidad visible
 
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
  lcd.print("Esperando WiFi");
  
   // Creamos una instancia de la clase WiFiManager
  WiFiManager wifiManager;
  // Descomentar para resetear configuración
  //wifiManager.resetSettings();
  // Cremos AP y portal cautivo
  wifiManager.setConfigPortalTimeout(60);
  wifiManager.autoConnect("GRANDUR-WIFI","Mtzt1585");

  // print the Message on the LCD.
  lcd.clear(); 
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
  int lectura = scale. get_units(10);
  int estado_uno = digitalRead (boton_uno);
  int estado_dos = digitalRead (boton_dos);
  bool salto = false;
  int envase1 = 0; //almacena el peso del envase vacío 1
  int muestra1 = 0; //almacena el peso de la muestra 1
  int granu1 = 0; //almacena el peso restante del tamizado 1
  float granu_porc1; // porcentaje granulometria 1
  int durabilidad1 = 0; //almacena el peso restante luego del procedimiento de durabilidad 1
  float durabilidad_porc1; // porcentaje durabilidad 1
  int envase2 = 0; //almacena el peso del envase vacío 2
  int muestra2 = 0; //almacena el peso de la muestra 2
  int granu2 = 0; //almacena el peso restante del tamizado 2
  float granu_porc2; // porcentaje granulometria 2
  int durabilidad2 = 0; //almacena el peso restante luego del procedimiento de durabilidad 2
  float durabilidad_porc2; // porcentaje durabilidad 2
  
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
        lcd.print("presione boton");
        estado_uno = digitalRead(boton_uno);        
      }while (estado_uno == HIGH);
     pasos++;
     Serial.print(pasos);
     delay(1000);

     // Para guardar tara de envase 1
     do
      {
        delay(250); 
        lectura = scale.get_units(10);
        estado_uno = digitalRead(boton_uno);
        lcd.clear();
        lcd.print("Envase Vacio 1");
        lcd.setCursor(0, 1);
        lcd.print(lectura);
        lcd.setCursor(15,1);
        lcd.print("g");
      }while ((lectura == 0) || (estado_uno == HIGH));
     envase1 = lectura;
     //Serial.println(envase1);
     delay(1000);

     // Para guardar tara de envase 2
     do
      {
        delay(250); 
        lectura = scale.get_units(10);
        estado_uno = digitalRead(boton_uno);
        lcd.clear();
        lcd.print("Envase Vacio 2");
        lcd.setCursor(0, 1);
        lcd.print(lectura);
        lcd.setCursor(15,1);
        lcd.print("g");
      }while ((lectura == 0) || (estado_uno == HIGH));
     envase2 = lectura;
     //Serial.println(envase1);
     delay(1000);

     // Para guardar peso de muestra total 1
     do
      {
        delay(250); 
        lectura = scale.get_units(10);
        estado_uno = digitalRead(boton_uno);
        int m = lectura - envase1;
        lcd.clear();
        lcd.print("Muestra 1");
        lcd.setCursor(0, 1);
        lcd.print(m);
        lcd.setCursor(15,1);
        lcd.print("g");
        if (lectura > envase1)
        {
          salto = true;
        }
      }while ((salto == false) || (estado_uno == HIGH));
     muestra1 = lectura;
    // Serial.println(envase);
    // Serial.println(muestra);
     delay(1000);

     // Para guardar peso de muestra total 2
     do
      {
        delay(250); 
        lectura = scale.get_units(10);
        estado_uno = digitalRead(boton_uno);
        int m = lectura - envase2;
        lcd.clear();
        lcd.print("Muestra 2");
        lcd.setCursor(0, 1);
        lcd.print(m);
        lcd.setCursor(15,1);
        lcd.print("g");
        if (lectura > envase2)
        {
          salto = true;
        }
      }while ((salto == false) || (estado_uno == HIGH));
     muestra2 = lectura;
    // Serial.println(envase);
    // Serial.println(muestra);
     delay(1000);

     // Pesar muestra tamizada para granulometría 1
     do
      {
        delay(250); 
        lectura = scale.get_units(10);
        estado_uno = digitalRead(boton_uno);
        int g = lectura - envase1;
        lcd.clear();
        lcd.print("Muestra Tamiz. 1");
        lcd.setCursor(0, 1);
        lcd.print(g);
        lcd.setCursor(15,1);
        lcd.print("g");
        if ((lectura > envase1) && (lectura < muestra1))
        {
          salto = true;
        }
      }while ((salto == false) || (estado_uno == HIGH));
     granu1 = lectura;
     /*Serial.println(envase);
     Serial.println(muestra);
     Serial.println(granu);*/
     granu_porc1 = porcentajeCalc(envase1, muestra1, granu1);
     //Serial.println(granu_porc);
     lcd.clear();
     lcd.print("Granulometria");
     lcd.setCursor(0, 1);
     lcd.print(granu_porc1);
     lcd.setCursor(15, 1);
     lcd.print("%");
     client.add(G_0,granu_porc1);
     client.sendAll(false); //enviar datos de manera oculta
     delay(5000);

     // Pesar muestra tamizada para granulometría 2
     do
      {
        delay(250); 
        lectura = scale.get_units(10);
        estado_uno = digitalRead(boton_uno);
        int g = lectura - envase2;
        lcd.clear();
        lcd.print("Muestra Tamiz. 2");
        lcd.setCursor(0, 1);
        lcd.print(g);
        lcd.setCursor(15,1);
        lcd.print("g");
        if ((lectura > envase2) && (lectura < muestra2))
        {
          salto = true;
        }
      }while ((salto == false) || (estado_uno == HIGH));
     granu2 = lectura;
     /*Serial.println(envase);
     Serial.println(muestra);
     Serial.println(granu);*/
     granu_porc2 = porcentajeCalc(envase2, muestra2, granu2);
     //Serial.println(granu_porc);
     lcd.clear();
     lcd.print("Granulometria");
     lcd.setCursor(0, 1);
     lcd.print(granu_porc2);
     lcd.setCursor(15, 1);
     lcd.print("%");
     client.add(G_0,granu_porc2);
     client.sendAll(false); //enviar datos de manera oculta
     delay(5000);

     // Pesar muestra tamizada para durabilidad 1
     do
      {
        delay(250); 
        lectura = scale.get_units(10);
        estado_uno = digitalRead(boton_uno);
        int d = lectura - envase1;
        lcd.clear();
        lcd.print("Muestra Tester1");
        lcd.setCursor(0, 1);
        lcd.print(d);
        lcd.setCursor(15,1);
        lcd.print("g");
        if ((lectura > envase1) && (lectura < muestra1))
        {
          salto = true;
        }
      }while ((salto == false) || (estado_uno == HIGH));
     durabilidad1 = lectura;
     Serial.println(envase1);
     Serial.println(muestra1);
     Serial.println(granu1);
     Serial.println(durabilidad1);
     durabilidad_porc1 = porcentajeCalc(envase1, granu1, durabilidad1);
     Serial.println(granu_porc1);
     Serial.println(durabilidad_porc1);
     lcd.clear();
     lcd.print("Durabilidad 1");
     lcd.setCursor(0, 1);
     lcd.print(durabilidad_porc1);
     lcd.setCursor(15, 1);
     lcd.print("%");
     client.add(D_0,durabilidad_porc1);
     client.sendAll(false);
     delay(5000);

     // Pesar muestra tamizada para durabilidad 2
     do
      {
        delay(250); 
        lectura = scale.get_units(10);
        estado_uno = digitalRead(boton_uno);
        int d = lectura - envase2;
        lcd.clear();
        lcd.print("Muestra Tester2");
        lcd.setCursor(0, 1);
        lcd.print(d);
        lcd.setCursor(15,1);
        lcd.print("g");
        if ((lectura > envase2) && (lectura < muestra2))
        {
          salto = true;
        }
      }while ((salto == false) || (estado_uno == HIGH));
     durabilidad2 = lectura;
     Serial.println(envase2);
     Serial.println(muestra2);
     Serial.println(granu2);
     Serial.println(durabilidad2);
     durabilidad_porc2 = porcentajeCalc(envase2, granu2, durabilidad2);
     Serial.println(granu_porc2);
     Serial.println(durabilidad_porc2);
     lcd.clear();
     lcd.print("Durabilidad 2");
     lcd.setCursor(0, 1);
     lcd.print(durabilidad_porc2);
     lcd.setCursor(15, 1);
     lcd.print("%");
     client.add(D_0,durabilidad_porc2);
     client.sendAll(false);
     delay(5000);

    }
  
  // Muestra resultados y espera respuesta para enviar datos 1
  do
  {
    delay (250);
    estado_uno = digitalRead (boton_uno);
    estado_dos = digitalRead (boton_dos);
    lcd.clear();
    lcd.print("G=");
    lcd.setCursor(2, 0);
    lcd.print(granu_porc1);
    lcd.setCursor(8, 0);
    lcd.print("D=");
    lcd.setCursor(10,0);
    lcd.print(durabilidad_porc1);
    lcd.setCursor(0,1);
    lcd.print("Desea Enviar 1?");
  }while (estado_uno==HIGH && estado_dos==HIGH);
  if (estado_uno==0)
  {
    client.add(G_1,granu_porc1);
    client.add(D_1,durabilidad_porc1);
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

  // Muestra resultados y espera respuesta para enviar datos 2
  do
  {
    delay (250);
    estado_uno = digitalRead (boton_uno);
    estado_dos = digitalRead (boton_dos);
    lcd.clear();
    lcd.print("G=");
    lcd.setCursor(2, 0);
    lcd.print(granu_porc2);
    lcd.setCursor(8, 0);
    lcd.print("D=");
    lcd.setCursor(10,0);
    lcd.print(durabilidad_porc2);
    lcd.setCursor(0,1);
    lcd.print("Desea Enviar 2?");
  }while (estado_uno==HIGH && estado_dos==HIGH);
  if (estado_uno==0)
  {
    client.add(G_1,granu_porc2);
    client.add(D_1,durabilidad_porc2);
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
