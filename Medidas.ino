/*****************************************************/
/*                                                   */
/* Medida de temperatura con LM35 para el termostato */
/*                                                   */
/*                                                   */
/*****************************************************/

#define MAX_INTENTOS_MEDIDA 5//Numero de intentos maximos de leer del Dallas

#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

// Declaracion de variables de los sensores
DHT dht(DHTPIN, DHT22);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

String nombres[MAX_SATELITES]; //nombre por defecto de los satelites
float tempC; //se declara global 
float humedad;
float luz;

void inicializaSensores(void)
  {

  DS18B20.begin();
  dht.begin();  
  
  //preconfiguracion de fabrica de las habitaciones
  //0 Salon
  nombres[0]="Salon";
  //1 Despacho
  nombres[1]="Despacho";  
  //2 Dormitorio Ppal
  nombres[2]="DormitorioPpal";  
  //3 Jorge
  nombres[3]="Jorge";  
  //4 Sara
  nombres[4]="Sara";  
  //5 Diego
  nombres[5]="Diego";  
  //6 Buhardilla
  nombres[6]="Buhardilla";  
  //7 Bodega
  nombres[7]="Bodega";  
  //8 Lavanderia
  nombres[8]="Lavanderia"; 
  //9 Salon
  nombres[9]="Exterior";
  //10 Despacho
  nombres[10]="Despacho2";  
  //11 Dormitorio Ppal
  nombres[11]="DormitorioPpal2";  
  //12 Jorge
  nombres[12]="Jorge2";  
  //13 Sara
  nombres[13]="Sara2";  
  //14 Diego
  nombres[14]="Diego2";  
  //15 Buhardilla
  nombres[15]="Buhardilla2";  

  //Variables medidas  
  leeSensores(debugGlobal);
  }
    
/**************************************/
/* Lee los sensores                   */
/* y almnacena el valor leido         */
/**************************************/
void leeSensores(int8_t debug)
  { 
  leeTemperatura();// Lee el valor desde el sensor
  leeHumedad();
  leeLuz();

  if(debug)Serial.printf("T: %s; H: %s, L: %s\n",getTemperaturaString().c_str(),getHumedadString().c_str(),getLuzString().c_str());
  }

/**************************************/
/* Lee el sensor de Tª y almnacena    */
/* el valor leido                     */
/**************************************/
void leeTemperatura(void)
  { 
  int8_t i=0;//hago como mucho MAX_INTENTOS_MEDIDA
  
  do 
    {
    DS18B20.requestTemperatures(); 
    tempC = DS18B20.getTempCByIndex(0);
    if(tempC != 85.0 && tempC != (-127.0)) break;
    delay(100);
    } while (i++<MAX_INTENTOS_MEDIDA);
  }
  
/**************************************/
/* Lee el sensor de Humedad           */
/* y almnacena el valor leido         */
/**************************************/
void leeHumedad(void)
  { 
  // Lee el valor desde el sensor
  float h;
  
  h = dht.readHumidity();  //leo el sensor
  if(!isnan(h)) humedad=h;  //si no es nan lo guardo
  }
/**************************************/
/* Lee el sensor de luz               */
/* y almnacena el valor leido         */
/**************************************/
void leeLuz(void)
  { 
  // Lee el valor desde el sensor
  luz=(analogRead(LDR_PIN)*100/1024);//valor entre 0 y 100. 100 luz intensa 0 oscuridad
  }
  
/**************************************/
/* Publica el valor de la Tª medida   */
/**************************************/
float getTemperatura(void)  //encapsula el acceso a la temperatura
  {
  return tempC;  
  }
/********************************************/
/* Publica el valor de la humedada medida   */
/********************************************/
float getHumedad(void)  //encapsula el acceso a la humedad
  {
  return humedad;  
  }
/**************************************/
/* Publica el valor de la luz medida  */
/**************************************/
float getLuz(void)  //encapsula el acceso a la luz
  {
  return luz;  
  }


/**************************************/
/* Publica el valor de la Tª medida   */
/**************************************/
String getTemperaturaString(void)  //encapsula el acceso a la temperatura
  {
  char salida[7];//"-999.9
  dtostrf(tempC, 2, 1, salida);  

  return String(salida);
  }
/********************************************/
/* Publica el valor de la humedada medida   */
/********************************************/
String getHumedadString(void)  //encapsula el acceso a la humedad
  {
  char salida[7];//"-999.9
  dtostrf(humedad, 2, 1, salida);  

  return String(salida);
  }
/**************************************/
/* Publica el valor de la luz medida  */
/**************************************/
String getLuzString(void)  //encapsula el acceso a la luz
  {
  char salida[7];//"-999.9
  dtostrf(luz, 2, 1, salida);  

  return String(salida);
  }


/***************************************/
/* Genera el json con las medidas      */
/***************************************/
String generaJson(void)
  {
  String cad="";
  
  //genero el json con las medidas--> {"temperatura": 22.5,"Humedad": 63,"Luz": 12}
  cad  = "{\n\t\"Temperatura\": ";
  cad += String(getTemperatura(),1);
  cad += ",\n\t\"Humedad\": ";
  cad += String(getHumedad(),1);
  cad += ",\n\t\"Luz\": ";
  cad += String(getLuz(),1);
  cad += ",\n\t\"id\": ";
  cad += String(direccion);   
  cad += "\n}";  

  return cad;
  }

//////PARA DEPURACION  
void setTemp(float f) {tempC=f;}
void setHum(float f) {humedad=f;}
void setLuz(float f) {luz=f;}
  
