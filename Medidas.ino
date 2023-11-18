/*****************************************************/
/*                                                   */
/* Medida de temperatura con LM35 para el termostato */
/*                                                   */
/*                                                   */
/*****************************************************/

#define MAX_INTENTOS_MEDIDA    5  //Numero de intentos maximos de leer del Dallas
#define HDC_DIRECCION_I2C    0x40 //Direccion I2C del HDC1080
#define BME280_DIRECCION_I2C 0x76 //Direccion I2C del BME280
#define BMP280_DIRECCION_I2C 0x76 //Direccion I2C del BMP280
#define SEALEVELPRESSURE_HPA 1024 //Presion a nivel del mar
#define BH1750_FONDO_ESCALA  3800 //Fondo de escala del sensor BH1750
#define SECO                  250 //Lectura del sensor en vacio
#define MOJADO                750 //Lectura del sensor en vaso de agua
#define NUM_MEDIDAS            10 //Numero de medidas a promediar

//Tipos de sensores
#define TIPO_NULO    "NULO"
#define TIPO_DS18B20 "DS18B20"  //Temperatura
#define TIPO_HDC1080 "HDC1080"  //Temperatura, Humedad
#define TIPO_DHT22   "DHT22"    //Temperatura, Humedad
#define TIPO_BME280  "BME280"   //Temperatura, Humedad y presion
#define TIPO_BMP280  "BMP280"   //Temperatura y presion
#define TIPO_GL5539  "GL5539"   //Luz
#define TIPO_BH1750  "BH1750"   //Luz
#define TIPO_SOILMOISTURECAPACITIVEV2 "SOILMOISTUREV2" //Humedad del suelo   

//Valor de dato no leido
#define NO_LEIDO      -100.0

#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <ClosedCube_HDC1080.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_Sensor.h>
#include <BH1750.h>

// Declaracion de variables de los sensores
DHT dht(DHTPIN, DHT22);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
ClosedCube_HDC1080 hdc1080;
Adafruit_BME280 bme280; // I2C Tº, humedad y presion
Adafruit_BMP280 bmp280; // I2C Tº y presion
BH1750 bh1750; //I2C direccion por defecto 0x23


String tipoSensorTemperatura;
String tipoSensorHumedad;
String tipoSensorLuz;
String tipoSensorPresion;
//String tipoSensorAltitud; //No es necesario, porque si hay es porque hay presion
String tipoSensorTemperaturaSuelo;
String tipoSensorHumedadSuelo;

String nombres[MAX_SATELITES]; //nombre por defecto de los satelites
float tempC=NO_LEIDO; //se declara global 
float tempProm[NUM_MEDIDAS];
int8_t indiceMedida;
float humedad=NO_LEIDO;
float luz=NO_LEIDO;
float presion=NO_LEIDO;
float altitud=NO_LEIDO;
float temperaturaSuelo=NO_LEIDO;
float humedadSuelo=NO_LEIDO;

void inicializaSensores(void)
  {
  //Valores pòr defecto
  tipoSensorTemperatura=TIPO_NULO;
  tipoSensorHumedad=TIPO_NULO;
  tipoSensorLuz=TIPO_NULO;
  tipoSensorPresion=TIPO_NULO;
  tipoSensorTemperaturaSuelo=TIPO_NULO;
  tipoSensorHumedadSuelo=TIPO_NULO;

  //Lo inicializo a NO_LEIDO y el indice a -1. En la primera lectira lo 
  //identifica (indiceMedida=-1) y carga en todas las medidas la medida inicial
  for(uint8_t i=0;i<NUM_MEDIDAS;i++)tempProm[i]=NO_LEIDO;
  indiceMedida=-1;

  Wire.begin();
  ScannerI2C();
  
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
  //9 Exterior
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

  //recupero datos del fichero de configuracion
  if (!recuperaDatosSensores(false)) 
    {
    Serial.printf("error al recuperar config de los sensores.\nConfiguracion por defecto.\n");
    return;
    }

  //Inicializo los sensores ¿TODOS?
  //Temperatura
  if(tipoSensorTemperatura==TIPO_NULO);
  else if(tipoSensorHumedad==TIPO_DHT22  ) dht.begin();                           //Temperatura y Humedad DHT22
  else if(tipoSensorTemperatura==TIPO_HDC1080) hdc1080.begin(HDC_DIRECCION_I2C);  //I2C Temperatura y Humedad HDC1080
  else if(tipoSensorTemperatura==TIPO_DS18B20) DS18B20.begin();                   //Temperatura Dallas DS18B20
  else if(tipoSensorTemperatura==TIPO_BME280){
    if (!bme280.begin(BME280_DIRECCION_I2C)) Serial.println("\n\nError al inicializar BME280\n\n\n"); //Temperatura bme280
    }
  else if(tipoSensorTemperatura==TIPO_BMP280){
    if (!bmp280.begin(BMP280_DIRECCION_I2C)) Serial.println("\n\nError al inicializar BMP280\n\n\n"); //Temperatura bmp280
    }
  
  //Humedad
  if(tipoSensorHumedad==TIPO_NULO);
  else if(tipoSensorHumedad==TIPO_HDC1080) hdc1080.begin(HDC_DIRECCION_I2C); //I2C Temperatura y Humedad HDC1080
  else if(tipoSensorHumedad==TIPO_DHT22  ) dht.begin();                      //Humedad DHT22
  else if(tipoSensorHumedad==TIPO_BME280) bme280.begin(BME280_DIRECCION_I2C); //Humedad bme280

  //Luz
  //No es necesaria la inicialización
  if(tipoSensorLuz==TIPO_NULO);
  else if(tipoSensorLuz==TIPO_GL5539); //LDR, no se inicializa. Lectura analogica
  else if(tipoSensorLuz==TIPO_BH1750) bh1750.begin(BH1750::CONTINUOUS_LOW_RES_MODE ); //I2C luz bh1750

  //Presion
  if(tipoSensorPresion==TIPO_NULO);
  else if(tipoSensorPresion==TIPO_BME280) bme280.begin(BME280_DIRECCION_I2C); //Humedad bme280
  else if(tipoSensorPresion==TIPO_BMP280) bmp280.begin(BMP280_DIRECCION_I2C); //Humedad bme280
    
  //Altitud
  //No es necesaria la inicialización, si hay es porque hay presion
  
  //Temperatura del suelo
  if(tipoSensorTemperaturaSuelo==TIPO_NULO);
  else if(tipoSensorTemperaturaSuelo==TIPO_DS18B20) DS18B20.begin();          //Temperatura Dallas DS18B20  

  //Humedad del suelo
  if(tipoSensorPresion==TIPO_NULO);
  else if(tipoSensorPresion==TIPO_SOILMOISTURECAPACITIVEV2) //Sensor capacitivo V2 no requiere inicializacion
  
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

  /* REDUNDANTE
  //cargo el valores por defecto
  tipoSensorTemperatura="NULO";
  tipoSensorHumedad="NULO";
  tipoSensorLuz="NULO";
  tipoSensorPresion="NULO";
  //tipoSensorAltitud="NULO";
  */
  
  if(!leeFichero(SENSORES_CONFIG_FILE, cad))
    {
    //Algo salio mal, confgiguracion por defecto
    Serial.printf("No existe fichero de configuracion de Sensores o esta corrupto\n");
    return false;
    }
    
  return parseaConfiguracionSensores(cad);    
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
    tipoSensorPresion=json.get<String>("tipoSensorPresion");
    tipoSensorPresion=json.get<String>("tipoSensorPresion");
    tipoSensorPresion=json.get<String>("tipoSensorPresion");
    tipoSensorTemperaturaSuelo=json.get<String>("tipoSensorTemperaturaSuelo");
    tipoSensorHumedadSuelo=json.get<String>("tipoSensorHumedadSuelo");

    Serial.printf("Configuracion leida:\ntipo sensor temperatura: %s\ntipo sensor humedad: %s\ntipo sensor luz: %s\ntipo sensor presion: %s\ntipo sensor tenperatura del suelo: %s\ntipo sensor humedad del suelo: %s\n",tipoSensorTemperatura.c_str(),tipoSensorHumedad.c_str(),tipoSensorLuz.c_str(),tipoSensorPresion.c_str(),tipoSensorTemperaturaSuelo.c_str(),tipoSensorHumedadSuelo.c_str());
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
  else if(tipoSensorHumedad==TIPO_DHT22  ) leeTemperaturaDHT22();       //Humedad DHT22
  else if(tipoSensorTemperatura==TIPO_HDC1080) leeTemperaturaHDC1080(); //I2C Temperatura HDC1080
  else if(tipoSensorTemperatura==TIPO_DS18B20) leeTemperaturaDS18B20(); //Temperatura Dallas DS18B20
  else if(tipoSensorTemperatura==TIPO_BME280 ) leeTemperaturaBME280(); //Temperatura BME280
  else if(tipoSensorTemperatura==TIPO_BMP280 ) leeTemperaturaBMP280(); //Temperatura BMP280
  if(tempC!=NO_LEIDO){
    if(indiceMedida==-1){
      for(indiceMedida=0;indiceMedida<NUM_MEDIDAS;indiceMedida++) tempProm[indiceMedida]=tempC;//Si es la primera despues d earrancar, todas a tempC;
      indiceMedida=0;
    }
    tempProm[indiceMedida++]=tempC;
    indiceMedida=indiceMedida % NUM_MEDIDAS;
    }
    
  //Humedad
  if(tipoSensorHumedad==TIPO_NULO);
  else if(tipoSensorHumedad==TIPO_HDC1080) leeHumedadHDC1080(); //I2C Temperatura y Humedad HDC1080
  else if(tipoSensorHumedad==TIPO_DHT22  ) leeHumedadDHT22();   //Humedad DHT22
  else if(tipoSensorHumedad==TIPO_BME280 ) leeHumedadBME280();  //Temperatura BME280
  //Luz
  if(tipoSensorLuz==TIPO_NULO);
  else if(tipoSensorLuz==TIPO_GL5539) leeLuzGL5539(); //LDR
  else if(tipoSensorLuz==TIPO_BH1750) leeLuzBH1750(); //I2C Luz BH1750
  //Presion
  if(tipoSensorPresion==TIPO_NULO);
  else if(tipoSensorPresion==TIPO_BME280) leePresionBME280(); //I2C Temperatura Presion y Humedad 
  else if(tipoSensorPresion==TIPO_BMP280) leePresionBMP280(); //I2C Temperatura y Presion
  //Temperatura del suelo
  if(tipoSensorTemperaturaSuelo==TIPO_NULO);
  else if(tipoSensorTemperaturaSuelo==TIPO_DS18B20) leeTemperaturaDS18B20(); //Temperatura Dallas DS18B20
  //Humedad del suelo
  if(tipoSensorHumedadSuelo==TIPO_SOILMOISTURECAPACITIVEV2) leeHumedadSueloCapacitivo();//Sensor capacitivo V2

  if(debug)Serial.printf("T: %s; H: %s, L: %s, P: %s, A: %s, TS: %s, HS: %s\n",getTemperaturaString().c_str(),getHumedadString().c_str(),getLuzString().c_str(),getPresionString().c_str(),getAltitudString().c_str(),getTemperaturaSueloString().c_str(),getHumedadSueloString().c_str());
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
    if(tempC != 85.0 && tempC != (-127.0)) return;
    delay(100);
    } while (i++<MAX_INTENTOS_MEDIDA);

  if(isnan(tempC)) tempC=NO_LEIDO;
  tempC=NO_LEIDO;
  }


/**************************************/
/* Lee el sensor de Tª DTH22          */
/* y almnacena el valor leido         */
/**************************************/
void leeTemperaturaDHT22(void)
  {
  float t;
  
  tempC = dht.readTemperature();  //leo el sensor
  if(isnan(tempC)) tempC=NO_LEIDO;
  }
  
/**************************************/
/* Lee el sensor de Tª HDC1080        */
/* y almnacena el valor leido         */
/**************************************/
void leeTemperaturaHDC1080(void)
  { 
  delay(20);
  tempC = hdc1080.readTemperature();  
  
  if(isnan(tempC)) tempC=NO_LEIDO;
  if(tempC==125) tempC=NO_LEIDO;
  }
  
/**************************************/
/* Lee el sensor de Tª BME280         */
/* y almnacena el valor leido         */
/**************************************/
void leeTemperaturaBME280(void)
  { 
  tempC = bme280.readTemperature();  
  if(!isnan(tempC)) tempC=NO_LEIDO;
  }

/**************************************/
/* Lee el sensor de Tª BMP280         */
/* y almnacena el valor leido         */
/**************************************/
void leeTemperaturaBMP280(void)
  { 
  tempC = bmp280.readTemperature();  
  if(!isnan(tempC)) tempC=NO_LEIDO;
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
  // Lee el valor desde el sensor
  float h;
  
  h = hdc1080.readHumidity();
  if(!isnan(h)) humedad=h;  //si no es nan lo guardo  
  }

/**************************************/
/* Lee el sensor de Humedad BME280    */
/* y almnacena el valor leido         */
/**************************************/
void leeHumedadBME280(void)
  { 
  // Lee el valor desde el sensor
  float h;
  
  h = bme280.readHumidity();
  if(!isnan(h)) humedad=h;  //si no es nan lo guardo  
  }

/**************************************/
/* Lee el sensor de luz GL5539        */
/* y almnacena el valor leido         */
/* valor entre 0 y 100.               */
/* 100 luz intensa 0 oscuridad        */
/**************************************/
void leeLuzGL5539(void)
  { 
  // Lee el valor desde el sensor
  float l;
  
  l = (analogRead(LDR_PIN)*100/1024);//valor entre 0 y 100. 100 luz intensa 0 oscuridad
  if(!isnan(l)) luz=l;  //si no es nan lo guardo
  }
  
/*****************************************/
/* Lee el sensor de luz GY-302 - BH1750 */
/* y almnacena el valor leido           */
/* valor entre 0 y 100.                 */
/* 100 luz intensa 0 oscuridad          */
/****************************************/
void leeLuzBH1750(void)
  { 
  // Lee el valor desde el sensor
  float l;
  
  l = bh1750.readLightLevel()*100.0/BH1750_FONDO_ESCALA;//fondo de escala tomo BH1750_FONDO_ESCALA, es aun mayor pero eso es mucha luz. responde entre 0 y 100
  if(!isnan(l)){
    luz=l;  //si no es nan lo guardo
    if(luz>100) luz=100; //si es mayor topo 100
    //valor entre 0 y 100. 100 luz intensa 0 oscuridad
    }
  }

/**************************************/
/* Lee el sensor de Presion BME280    */
/* y almnacena el valor leido         */
/**************************************/
void leePresionBME280(void)
  { 
  // Lee el valor desde el sensor
  float p,a;
  
  p = bme280.readPressure()/100.0F;
  //Serial.printf("p: %f | funcion: %f | division: %f\n",p,bmp280.readPressure(),bmp280.readPressure()/100.0F);  
  if(!isnan(p)){
    presion=p;  //si no es nan lo guardo  

    a = bme280.readAltitude(SEALEVELPRESSURE_HPA);
    if(!isnan(a)) altitud=a;
    else altitud=-1;
    }
  }

/**************************************/
/* Lee el sensor de Presion BMP280    */
/* y almnacena el valor leido         */
/**************************************/
void leePresionBMP280(void)
  { 
  // Lee el valor desde el sensor
  float p,a;
  
  p = bmp280.readPressure()/100.0F;
  //Serial.printf("p: %f | funcion: %f | division: %f\n",p,bmp280.readPressure(),bmp280.readPressure()/100.0F);
  if(!isnan(p)){
    presion=p;  //si no es nan lo guardo  

    a = bmp280.readAltitude(SEALEVELPRESSURE_HPA);
    if(!isnan(a)) altitud=a;
    else altitud=-1;    
    }
  }

/**************************************/
/* Lee el sensor de Presion tª suelo  */
/* y almnacena el valor leido         */
/**************************************/
void leeTemperaturaSuelo(void)
  {   
  int8_t i=0;//hago como mucho MAX_INTENTOS_MEDIDA
  
  do 
    {
    DS18B20.requestTemperatures(); 
    temperaturaSuelo = DS18B20.getTempCByIndex(0);
    if(temperaturaSuelo != 85.0 && tempC != (-127.0)) return;
    delay(100);
    } while (i++<MAX_INTENTOS_MEDIDA);

  if(isnan(tempC)) tempC=NO_LEIDO;
  //temperaturaSuelo=NO_LEIDO;
  }
  
/**************************************/
/* Lee el sensor de humedad del suelo */
/* y almnacena el valor leido         */
/**************************************/
void leeHumedadSueloCapacitivo(void)
  { 
  int sensorVal = analogRead(PIN_SENSOR_HUMEDAD_SUELO);
  humedadSuelo=map(sensorVal, MOJADO, SECO, 100, 0); 
  }

/**************************************/
/* Publica el valor de la Tª medida   */
/**************************************/
float getTemperatura(void)  //encapsula el acceso a la temperatura
  {
  //return tempC;  
  float tempMedia;
  for(uint8_t i=0;i<NUM_MEDIDAS;i++) tempMedia+=tempProm[i];
  return tempMedia/NUM_MEDIDAS;
  }
/*Temperatura en farenheit*/  
/*  (32 °C × 9 / 5) + 32 = 89,6 °F*/
float getTemperaturaF(void)
  {
  return ((tempC * 9.0 / 5.0) + 32.0);  
  }
float getTemperaturaRocio(void) //En celsius
  {
  return (getTemperatura() - ((100.0 - getHumedad())/5.0));
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

/******************************************/
/* Publica el valor de la presion medida  */
/******************************************/
float getPresion(void)  //encapsula el acceso a la presion
  {
  return presion;  
  }

/******************************************/
/* Publica el valor de la altitud medida  */
/******************************************/
float getAltitud(void)  //encapsula el acceso a la altitud
  {
  return altitud;  
  }

/************************************************/
/* Publica el valor de la temperatura del suelo */
/************************************************/
float getTemperaturaSuelo(void)  //encapsula el acceso a la temperatura del suelo
  {
  return temperaturaSuelo;  
  }

/********************************************/
/* Publica el valor de la humedad del suelo */
/********************************************/
float getHumedadSuelo(void)  //encapsula el acceso a la humedad del suelo
  {
  return humedadSuelo;  
  }

/**************************************/
/* Publica el valor de la Tª medida   */
/**************************************/
String getTemperaturaString(void)  //encapsula el acceso a la temperatura
  {
  const uint8_t anchoSalida=7;  
  char salida[anchoSalida];//"-999.9
  dtostrf(tempC, anchoSalida-1, 1, salida);  

  return String(salida);
  }
  
/********************************************/
/* Publica el valor de la humedada medida   */
/********************************************/
String getHumedadString(void)  //encapsula el acceso a la humedad
  {
  const uint8_t anchoSalida=7;  
  char salida[anchoSalida];//"-999.9
  dtostrf(humedad, anchoSalida-1, 1, salida);  

  return String(salida);
  }
/**************************************/
/* Publica el valor de la luz medida  */
/**************************************/
String getLuzString(void)  //encapsula el acceso a la luz
  {
  const uint8_t anchoSalida=7;
  char salida[anchoSalida];//"-999.9
  dtostrf(luz, anchoSalida-1, 1, salida);  

  return String(salida);
  }

/******************************************/
/* Publica el valor de la presion medida  */
/******************************************/
String getPresionString(void)  //encapsula el acceso a la presion
  {
  const uint8_t anchoSalida=7;  
  char salida[anchoSalida];//"-999.9
  dtostrf(presion, anchoSalida-1, 1, salida);  

  return String(salida);
  }

/******************************************/
/* Publica el valor de la altitud medida  */
/******************************************/
String getAltitudString(void)  //encapsula el acceso a la altitud
  {
  const uint8_t anchoSalida=7;  
  char salida[anchoSalida];//"-999.9
  dtostrf(altitud, anchoSalida-1, 1, salida);  

  return String(salida);
  }

/********************************************************/
/* Publica el valor de la temperatura del suelo medida  */
/********************************************************/
String getTemperaturaSueloString(void)  //encapsula el acceso a la temperatura del suelo
  {
  const uint8_t anchoSalida=7;  
  char salida[anchoSalida];//"-999.9
  dtostrf(temperaturaSuelo, anchoSalida-1, 1, salida);  

  return String(salida);
  }

/****************************************************/
/* Publica el valor de la humedad del suelo medida  */
/****************************************************/
String getHumedadSueloString(void)  //encapsula el acceso a la humedad del suelo
  {
  const uint8_t anchoSalida=7;  
  char salida[anchoSalida];//"-999.9
  dtostrf(humedadSuelo, anchoSalida-1, 1, salida);  

  return String(salida);
  }
/******************************************* Fin Medidas*************************************************/

/***************************************/
/* Genera el json con las medidas      */
/***************************************/
String generaJson(void)
  {
  String cad="";
  
  //genero el json con las medidas--> {"id": 1, "temperatura": 22.5, "Humedad": 63, "Luz": 12, "Presion": 1036.2, "Altitud": 645.2}
/*  
  cad = "{\"titulo\": \"";
  cad += String(nombre_dispositivo);
  cad += "\"";
  cad += ",\"id\": ";
  cad += String(direccion);
  cad += ",\"habitacion\": ";
  cad += "\"";
  cad += String(nombres[direccion]);
  cad += "\"";  
  cad += ",\"Temperatura\": ";
  cad += String(getTemperatura(),1);
  cad += ",\"Humedad\": ";
  cad += String(getHumedad(),1);
  cad += ",\"Luz\": ";
  cad += String(getLuz(),1);
  cad += ",\"Altitud\": ";
  cad += String(getAltitud(),1);   
  cad += ",\"Presion\": ";
  cad += String(getPresion(),1);
  cad += ",\"TemepraturaSuelo\": ";
  cad += String(getTemperaturaSuelo(),1);
  cad += ",\"HumedadSuelo\": ";
  cad += String(getHumedadSuelo(),1);
  cad += "}";  
*/
  /***********************************************************************************/
  const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(6) + 2*JSON_OBJECT_SIZE(3) + 7*JSON_OBJECT_SIZE(7);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  
  JsonObject& root = jsonBuffer.createObject();
  root["titulo"] = String(nombre_dispositivo);;
  root["id"] = String(direccion);
  root["habitacion"] = String(nombres[direccion]);
  root["Temperatura"] = String(getTemperatura(),1);
  root["Humedad"] = String(getHumedad(),1);
  root["Luz"] = String(getLuz(),1);
  root["Altitud"] = String(getAltitud(),1);
  root["Presion"] = String(getPresion(),1);
  root["TemepraturaSuelo"] = String(getTemperaturaSuelo(),1);
  root["HumedadSuelo"] = String(getHumedadSuelo(),1);    

  root.printTo(cad);//root.printTo(Serial);
  /**********************************************************************************************************/  
  return cad;
  }

/****************************** PARA DEPURACION  ****************************************/
void setTemp(float f) {tempC=f;}
void setHum(float f) {humedad=f;}
void setLuz(float f) {luz=f;}
void setPresion(float f) {presion=f;}
void setAltitud(float f) {altitud=f;} 
void setTemperaturaSuelo(float f) {temperaturaSuelo=f;}  
void setHumedadSuelo(float f) {humedadSuelo=f;}  

void ScannerI2C(void)
{
  byte error, address;
  int nDevices;
 
  Serial.println("Identificando sensores...");
 
  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
 
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      Serial.print("Dispositivo I2C encontrando en la direccion 0x");
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("");
 
      nDevices++;
    }
    else if (error==4) 
    {
      Serial.print("Error desconocido en la direccion 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) Serial.printf("No se encontro ningun dispositivo I2C\n");
}
