/*****************************************************/
/*                                                   */
/* Medida de temperatura con LM35 para el termostato */
/*                                                   */
/*                                                   */
/*****************************************************/

#define MAX_INTENTOS_MEDIDA 5  //Numero de intentos maximos de leer del Dallas
#define HDC_DIRECCION_I2C 0x40 //Direccion I2C del HDC1080
//Tipos de sensores
#define TIPO_NULO    "NULO"
#define TIPO_HDC1080 "HDC1080"
#define TIPO_DS18B20 "DS18B20"
#define TIPO_DHT22   "DHT22"
#define TIPO_GL5539  "GL5539"

#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <ClosedCube_HDC1080.h>

// Declaracion de variables de los sensores
DHT dht(DHTPIN, DHT22);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
ClosedCube_HDC1080 hdc1080;

String tipoSensorTemperatura;
String tipoSensorHumedad;
String tipoSensorLuz;
String nombres[MAX_SATELITES]; //nombre por defecto de los satelites
float tempC=-100; //se declara global 
float humedad=-1;
float luz=-1;

void inicializaSensores(void)
  {
  //recupero datos del fichero de configuracion
  if (!recuperaDatosSensores(false)) Serial.printf("error al recuperar config de los sensores.\nConfiguracion por defecto.\n");

  //Inicializo los sensores ¿TODOS?
  //Temperatura
  if(tipoSensorTemperatura==TIPO_NULO);
  else if(tipoSensorTemperatura==TIPO_HDC1080) hdc1080.begin(HDC_DIRECCION_I2C); //I2C Temperatura y Humedad HDC1080
  else if(tipoSensorTemperatura==TIPO_DS18B20) DS18B20.begin(); //Temperatura Dallas DS18B20
  //Humedad
  if(tipoSensorHumedad==TIPO_NULO);
  else if(tipoSensorHumedad==TIPO_HDC1080) hdc1080.begin(HDC_DIRECCION_I2C); //I2C Temperatura y Humedad HDC1080
  else if(tipoSensorHumedad==TIPO_DHT22  ) dht.begin();                      //Humedad DHT22
  //Luz
  //No es necesaria la inicialización
  
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
    
/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo de sensores                      */
/************************************************/
boolean recuperaDatosSensores(boolean debug)
  {
  String cad="";
  if (debug) Serial.println("Recupero configuracion de archivo...");

  //cargo el valores por defecto
  tipoSensorTemperatura="NULO";
  tipoSensorHumedad="NULO";
  tipoSensorLuz="NULO";

  if(leeFichero(SENSORES_CONFIG_FILE, cad))
    if(parseaConfiguracionSensores(cad)) 
      return true;

  //Algo salio mal, confgiguracion por defecto
  Serial.printf("No existe fichero de configuracion de Sensores o esta corrupto\n");
  cad="{\"tipoSensorTemperatura\": \"NULO\", \"tipoSensorHumedad\": \"NULO\",\"tipoSensorLuz\": \"NULO\"}";
  salvaFichero(SENSORES_CONFIG_FILE, SENSORES_CONFIG_BAK_FILE, cad);
  Serial.printf("Fichero de configuracion de Sensores creado por defecto\n");
  parseaConfiguracionSensores(cad);
    
  return false;
  }  

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de sensores                  */
/*********************************************/
boolean parseaConfiguracionSensores(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    tipoSensorTemperatura=json.get<String>("tipoSensorTemperatura");
    tipoSensorHumedad=json.get<String>("tipoSensorHumedad");
    tipoSensorLuz=json.get<String>("tipoSensorLuz");

    Serial.printf("Configuracion leida:\ntipo sensor temperatura: %s\ntipo sensor humedad: %s\ntipo sensor luz: %s\n",tipoSensorTemperatura.c_str(),tipoSensorHumedad.c_str(),tipoSensorLuz.c_str());
//************************************************************************************************
    return true;
    }
  return false;
  }

/*********************************************** Lectura de las medidas de los sensores *****************************************************/
/**************************************/
/* Lee los sensores                   */
/* y almnacena el valor leido         */
/**************************************/
void leeSensores(int8_t debug)
  { 
  //Leo los sensores  
  //Temperatura
  if(tipoSensorTemperatura==TIPO_NULO);
  else if(tipoSensorTemperatura==TIPO_HDC1080) leeTemperaturaHDC1080(); //I2C Temperatura HDC1080
  else if(tipoSensorTemperatura==TIPO_DS18B20) leeTemperaturaDS18B20(); //Temperatura Dallas DS18B20
  //Humedad
  if(tipoSensorHumedad==TIPO_NULO);
  else if(tipoSensorHumedad==TIPO_HDC1080) leeHumedadHDC1080(); //I2C Temperatura y Humedad HDC1080
  else if(tipoSensorHumedad==TIPO_DHT22  ) leeHumedadDHT22();                      //Humedad DHT22
  //Luz
  if(tipoSensorLuz==TIPO_NULO);
  else if(tipoSensorLuz==TIPO_GL5539) leeLuzGL5539(); //I2C Temperatura y Humedad HDC1080

  if(debug)Serial.printf("T: %s; H: %s, L: %s\n",getTemperaturaString().c_str(),getHumedadString().c_str(),getLuzString().c_str());
  }

/**************************************/
/* Lee los sensores de Tª y           */
/* almnacena los valores leidos       */
/**************************************/
void xleeTemperatura(void)
  { 
  leeTemperaturaDS18B20();
  leeTemperaturaHDC1080();
  }
  
/**************************************/
/* Lee los sensores de humedad y      */
/* almnacena los valores leidos       */
/**************************************/
void xleeHumedad(void)
  { 
  leeHumedadDHT22();
  leeHumedadHDC1080();
  }
  
/**************************************/
/* Lee el sensor de luz               */
/* y almnacena el valor leido         */
/**************************************/
void xleeLuz(void)
  { 
  leeLuzGL5539();
  }
  
/**************************************/
/* Lee el sensor de Tª DS18B20        */
/* y almnacena el valor leido         */
/**************************************/
void leeTemperaturaDS18B20(void)
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
/* Lee el sensor de Tª HDC1080        */
/* y almnacena el valor leido         */
/**************************************/
void leeTemperaturaHDC1080(void)
  { 
  tempC = hdc1080.readTemperature();  
  }
  
/**************************************/
/* Lee el sensor de Humedad           */
/* y almnacena el valor leido         */
/**************************************/
void leeHumedadDHT22(void)
  { 
  // Lee el valor desde el sensor
  float h;
  
  h = dht.readHumidity();  //leo el sensor
  if(!isnan(h)) humedad=h;  //si no es nan lo guardo
  }

/**************************************/
/* Lee el sensor de Humedad HDC1080   */
/* y almnacena el valor leido         */
/**************************************/
void leeHumedadHDC1080(void)
  { 
  humedad = hdc1080.readHumidity();
  }

/**************************************/
/* Lee el sensor de luz GL5539        */
/* y almnacena el valor leido         */
/**************************************/
void leeLuzGL5539(void)
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

/*******************************************************/
/*                                                     */
/* Genera el json con el estado delcontrolador,        */
/* las habitaciones y las salidas                      */
/*                                                     */
/*******************************************************/
String generaJsonEstado(void)
  {
  return generaJson();
  } 
  
//////PARA DEPURACION  
void setTemp(float f) {tempC=f;}
void setHum(float f) {humedad=f;}
void setLuz(float f) {luz=f;}
  
