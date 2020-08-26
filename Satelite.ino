/*
 * Satelite de medidas para Termostato
 *
 * Medidor distribuido
 * 
 * Un sensor de Tª LM35 en el pin XX
 * Un sensor de luminancia LDR en pin YYY
 * Servicio web levantado en puerto ZZZ
 */

/***************************** Defines *****************************/
//Defines generales
#define NOMBRE_FAMILIA "Termometro_Satelite"
#define VERSION "1.8.8 (ESP8266v2.7.4 OTA|json|MQTT|Cont. dinamicos|WebSockets)" //Actualizada version de tarjeta y corregido problema con la configuracion por defecto en el arranque
#define SEPARADOR        '|'
#define SUBSEPARADOR     '#'
#define KO               -1
#define OK                0
#define MAX_VUELTAS  UINT16_MAX// 32767 

#define PUERTO_WEBSERVER 80
#define MAX_SATELITES 16 //numero maximo de satelites de 0 a 15 controlado por los DIP Switch
#define POLLING_TIME_OUT 60000 //Milisegundos transcurridos entre dos peticiones del controlador antes de intentar registrarse

//Nombres de ficheros
#define GLOBAL_CONFIG_FILE         "/Config.json"
#define GLOBAL_CONFIG_BAK_FILE     "/Config.json.bak"
#define SENSORES_CONFIG_FILE       "/SensoresConfig.json"
#define SENSORES_CONFIG_BAK_FILE   "/SensoresConfig.json.bak"
#define WIFI_CONFIG_FILE           "/WiFiConfig.json"
#define WIFI_CONFIG_BAK_FILE       "/WiFiConfig.json.bak"
#define MQTT_CONFIG_FILE           "/MQTTConfig.json"
#define MQTT_CONFIG_BAK_FILE       "/MQTTConfig.json.bak"

//Defines de pines de los captadores
#define ONE_WIRE_BUS        D7//Pin donde esta el DS18B20
#define DHTPIN              D6//Pin de datos del DTH22
#define LDR_PIN             A0 
#define PIN_DESBORDE_TIEMPO D5

// Una vuela de loop son ANCHO_INTERVALO segundos 
#define MULTIPLICADOR_ANCHO_INTERVALO 5 //Multiplica el ancho del intervalo para mejorar el ahorro de energia
#define ANCHO_INTERVALO            1200 //Ancho en milisegundos de la rodaja de tiempo
#define FRECUENCIA_OTA                5 //cada cuantas vueltas de loop atiende las acciones
#define FRECUENCIA_LEE_SENSORES      50 //cada cuantas vueltas de loop lee los sensores
#define FRECUENCIA_SERVIDOR_WEB       1 //cada cuantas vueltas de loop atiende el servidor web
#define FRECUENCIA_ORDENES            2 //cada cuantas vueltas de loop atiende las ordenes via serie 
#define FRECUENCIA_MQTT              50 //cada cuantas vueltas de loop envia y lee del broket MQTT
#define FRECUENCIA_ENVIA_DATOS      100 //cada cuantas vueltas de loop publica el estado en el broker MQTT
#define FRECUENCIA_WIFI_WATCHDOG    100 //cada cuantas vueltas comprueba si se ha perdido la conexion WiFi

//#define LED_BUILTIN                2 //GPIO del led de la placa en los ESP32   
/***************************** Defines *****************************/

/***************************** Includes *****************************/
//Includes generales
#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <TimeLib.h>  // download from: http://www.arduino.cc/playground/Code/Time
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <WebSocketsServer.h> //Lo pongo aqui porque si lo pongo en su sitio no funciona... https://github.com/Links2004/arduinoWebSockets/issues/356
/***************************** Includes *****************************/

/*-----------------Variables comunes---------------*/
String nombre_dispositivo(NOMBRE_FAMILIA);//Nombre del dispositivo, por defecto el de la familia
int8_t direccion=0; //Direccion del modulo
uint16_t vuelta = MAX_VUELTAS-100;//0; //vueltas de loop
int debugGlobal=0; //por defecto desabilitado
uint8_t ahorroEnergia=0;//inicialmente desactivado el ahorro de energia
time_t anchoLoop= ANCHO_INTERVALO;//inicialmente desactivado el ahorro de energia

//Contadores
uint16_t multiplicadorAnchoIntervalo=5;
uint16_t anchoIntervalo=1200;
uint16_t frecuenciaOTA=5;
uint16_t frecuenciaLeeSensores=50;
uint16_t frecuenciaServidorWeb=1;
uint16_t frecuenciaOrdenes=2;
uint16_t frecuenciaMQTT=50;
uint16_t frecuenciaEnviaDatos=100;
uint16_t frecuenciaWifiWatchdog=100;

/************************* FUNCIONES PARA EL BUITIN LED ***************************/
void configuraLed(void){pinMode(LED_BUILTIN, OUTPUT);}
void enciendeLed(void){digitalWrite(LED_BUILTIN, LOW);}//En esp8266 es al reves que en esp32
void apagaLed(void){digitalWrite(LED_BUILTIN, HIGH);}//En esp8266 es al reves que en esp32
void parpadeaLed(uint8_t veces, uint16_t espera=100)
  {
  for(uint8_t i=0;i<2*veces;i++)
    {
    delay(espera/2);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }
/***********************************************************************************/  

void setup()
  {
  Serial.begin(115200);
  configuraLed();
  enciendeLed();
    
  Serial.printf("\n\n\n");
  Serial.printf("*************** %s ***************\n",NOMBRE_FAMILIA);
  Serial.printf("*************** %s ***************\n",VERSION);
  Serial.println("***************************************************************");
  Serial.println("*                                                             *");
  Serial.println("*             Inicio del setup del modulo                     *");
  Serial.println("*                                                             *");    
  Serial.println("***************************************************************");
  
  Serial.printf("\nMotivo del reinicio: %s\n",ESP.getResetReason().c_str());
  
  Serial.printf("\n\nInit Ficheros ---------------------------------------------------------------------\n");
  //Ficheros - Lo primero para poder leer los demas ficheros de configuracion
  inicializaFicheros(debugGlobal);
  apagaLed();

  //Configuracion general
  Serial.println("Init Config -----------------------------------------------------------------------");
  inicializaConfiguracion(debugGlobal);
  parpadeaLed(1);

  //Wifi
  Serial.println("Init WiFi -----------------------------------------------------------------------");
  if (inicializaWifi(true))//debugGlobal)) No tien esentido debugGlobal, no hay manera de activarlo
    {
    parpadeaLed(5,200);

    /*----------------Inicializaciones que necesitan red-------------*/
    //OTA
    Serial.println("Init OTA -----------------------------------------------------------------------");
    inicializaOTA(debugGlobal);
    parpadeaLed(1);
    //MQTT
    Serial.println("Init MQTT -----------------------------------------------------------------------");
    inicializaMQTT();
    parpadeaLed(2);
    //WebServer
    Serial.println("Init Web ------------------------------------------------------------------------");
    inicializaWebServer();
    parpadeaLed(3);
    //WebSockets
    Serial.println("Init Web ------------------------------------------------------------------------");
    inicializaWebSockets();
    parpadeaLed(4);    
    }
  else Serial.println("No se pudo conectar al WiFi");
  apagaLed();

  //Sensores
  Serial.println("Init sensores ---------------------------------------------------------------------");
  inicializaSensores();

  //Ordenes serie
  Serial.println("Init Ordenes ----------------------------------------------------------------------");  
  inicializaOrden();//Inicializa los buffers de recepcion de ordenes desde PC

  parpadeaLed(2);
  apagaLed();//Por si acaso....
  
  Serial.println("***************************************************************");
  Serial.println("*                                                             *");
  Serial.println("*               Fin del setup del modulo                      *");
  Serial.println("*                                                             *");    
  Serial.println("***************************************************************");
  }  

void  loop()
  {  
  //referencia horaria de entrada en el bucle
  time_t EntradaBucle=millis();//Hora de entrada en la rodaja de tiempo

  //------------- EJECUCION DE TAREAS --------------------------------------
  //Acciones a realizar en el bucle   
  //Prioridad 0: OTA es prioritario.
  if ((vuelta % frecuenciaOTA)==0) ArduinoOTA.handle(); //Gestion de actualizacion OTA
  //Prioridad 2: Funciones de control.
  if ((vuelta % frecuenciaLeeSensores)==0) leeSensores(debugGlobal); //lee los sensores de distancia
  //Prioridad 3: Interfaces externos de consulta  
  if ((vuelta % frecuenciaServidorWeb)==0) webServer(debugGlobal); //atiende el servidor web
  if ((vuelta % frecuenciaServidorWeb)==0) atiendeWebSocket(debugGlobal); //atiende el servidor web
  if ((vuelta % frecuenciaMQTT)==0) atiendeMQTT();
  if ((vuelta % frecuenciaEnviaDatos)==0) enviaDatos(debugGlobal);
  if ((vuelta % frecuenciaOrdenes)==0) while(HayOrdenes(debugGlobal)) EjecutaOrdenes(debugGlobal); //Lee ordenes via serie
  if ((vuelta % frecuenciaWifiWatchdog)==0) WifiWD();
  //------------- FIN EJECUCION DE TAREAS ---------------------------------  

  //sumo una vuelta de loop, si desborda inicializo vueltas a cero
  vuelta++;//sumo una vuelta de loop  
      
  //Espero hasta el final de la rodaja de tiempo
  while(millis()<EntradaBucle+anchoLoop)//ANCHO_INTERVALO)
    {
    if(millis()<EntradaBucle) break; //cada 49 dias el contador de millis desborda
    //delayMicroseconds(1000);
    delay(1);
    }
  }

///////////////CONFIGURACION GLOBAL/////////////////////
/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo global                           */
/************************************************/
boolean inicializaConfiguracion(boolean debug)
  {
  String cad="";
  if (debug) Serial.println("Recupero configuracion de archivo...");

  //cargo el valores por defecto
  //Contadores
  multiplicadorAnchoIntervalo=5;
  anchoIntervalo=1200;
  anchoLoop=anchoIntervalo;
  frecuenciaOTA=5;
  frecuenciaLeeSensores=50;
  frecuenciaServidorWeb=1;
  frecuenciaOrdenes=2;
  frecuenciaMQTT=50;
  frecuenciaEnviaDatos=100;
  frecuenciaWifiWatchdog=100;  
  
  ahorroEnergia=0; //ahorro de energia desactivado por defecto
    
  if(!leeFichero(GLOBAL_CONFIG_FILE, cad))
    {
    Serial.printf("No existe fichero de configuracion global\n");    
    return false;
    }  
    
  parseaConfiguracionGlobal(cad);
  
  //Ajusto el ancho del intervalo segun el modo de ahorro de energia  
  if(ahorroEnergia==0) anchoLoop=anchoIntervalo;
  else anchoLoop=multiplicadorAnchoIntervalo*anchoIntervalo;
  
  return true;
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio global                       */
/*********************************************/
/*
// Is there a value named "error" in the object?
if (obj.containsKey("error")) {
// Get the text of the error
const char* error = obj["error"];
// ...
}
*/
boolean parseaConfiguracionGlobal(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    multiplicadorAnchoIntervalo=json.get<uint16_t>("multiplicadorAnchoIntervalo"); if(multiplicadorAnchoIntervalo==0) multiplicadorAnchoIntervalo=5;
    anchoIntervalo=json.get<uint16_t>("anchoIntervalo");                           if(anchoIntervalo==0) anchoIntervalo=1200;
    frecuenciaOTA=json.get<uint16_t>("frecuenciaOTA");                             if(frecuenciaOTA==0) frecuenciaOTA=5;
    frecuenciaLeeSensores=json.get<uint16_t>("frecuenciaLeeSensores");             if(frecuenciaLeeSensores==0) frecuenciaLeeSensores=50;
    frecuenciaServidorWeb=json.get<uint16_t>("frecuenciaServidorWeb");             if(frecuenciaServidorWeb==0) frecuenciaServidorWeb=1;
    frecuenciaOrdenes=json.get<uint16_t>("frecuenciaOrdenes");                     if(frecuenciaOrdenes==0) frecuenciaOrdenes=2; 
    frecuenciaMQTT=json.get<uint16_t>("frecuenciaMQTT");                           if(frecuenciaMQTT==0) frecuenciaMQTT=50;
    frecuenciaEnviaDatos=json.get<uint16_t>("frecuenciaEnviaDatos");               if(frecuenciaEnviaDatos==0) frecuenciaEnviaDatos=100;
    frecuenciaWifiWatchdog=json.get<uint16_t>("frecuenciaWifiWatchdog");           if(frecuenciaWifiWatchdog==0) frecuenciaWifiWatchdog=100;

    ahorroEnergia=json.get<uint16_t>("ahorroEnergia");
    direccion = json.get<uint16_t>("id"); // "5"

    Serial.printf("Configuracion leida:\ndireccion: %i\nAhorro de energia: %i\n",direccion, ahorroEnergia);
    Serial.printf("\nContadores\nmultiplicadorAnchoIntervalo: %i\nanchoIntervalo: %i\nfrecuenciaOTA: %i\nfrecuenciaLeeSensores: %i\nfrecuenciaServidorWeb: %i\nfrecuenciaOrdenes: %i\nfrecuenciaMQTT: %i\nfrecuenciaEnviaDatos: %i\nfrecuenciaWifiWatchdog: %i\n",multiplicadorAnchoIntervalo, anchoIntervalo, frecuenciaOTA, frecuenciaLeeSensores,frecuenciaServidorWeb, frecuenciaOrdenes, frecuenciaMQTT, frecuenciaEnviaDatos, frecuenciaWifiWatchdog);
//************************************************************************************************
    }
  } 
