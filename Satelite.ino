 /*
 * Satelite de medidas para Termostato
 *
 * Medidor distribuido
 * 
 * Un sensor de Tª LM35 en el pin XX
 * Un sensor de luminancia LDR en pin YYY
 * Servicio web levantado en puerto ZZZ
 */
#include <FS.h>                   //this needs to be first, or it all crashes and burns...

//Defines generales
#define NOMBRE_FAMILIA "Termometro_Satelite"
#define VERSION "1.7.4.1 (ESP8266v2.4.2 OTA|json|MQTT|Cont. dinamicos)"
#define PUERTO_WEBSERVER 80
#define MAX_SATELITES 16 //numero maximo de satelites de 0 a 15 controlado por los DIP Switch
#define POLLING_TIME_OUT 60000 //Milisegundos transcurridos entre dos peticiones del controlador antes de intentar registrarse

#define SEPARADOR        '|'
#define SUBSEPARADOR     '#'
#define KO               -1
#define OK                0
#define MAX_VUELTAS  UINT16_MAX// 32767 

#define GLOBAL_CONFIG_FILE     "/Config.json"
#define GLOBAL_CONFIG_BAK_FILE "/Config.json.bak"
#define WIFI_CONFIG_FILE       "/WiFiConfig.json"
#define WIFI_CONFIG_BAK_FILE   "/WiFiConfig.json.bak"
#define MQTT_CONFIG_FILE       "/MQTTConfig.json"
#define MQTT_CONFIG_BAK_FILE   "/MQTTConfig.json.bak"

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
#define FRECUENCIA_WIFI_WATCHDOG    100 //cada cuantas vueltas comprueba si se ha perdido la conexion WiFi

#include <TimeLib.h>  // download from: http://www.arduino.cc/playground/Code/Time
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

//IPs de los diferentes dispositivos 
IPAddress IPControlador;
IPAddress IPSatelites[MAX_SATELITES];
IPAddress IPGateway;
int8_t direccion=0; //Direccion del modulo

/*-----------------Variables comunes---------------*/
String nombre_dispoisitivo(NOMBRE_FAMILIA);//Nombre del dispositivo, por defecto el de la familia
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
uint16_t frecuenciaWifiWatchdog=100;

void setup()
  {
  Serial.begin(115200);
  Serial.printf("\n\n\n");
  Serial.printf("*************** %s ***************\n",NOMBRE_FAMILIA);
  Serial.printf("*************** %s ***************\n",VERSION);
  Serial.println("***************************************************************");
  Serial.println("*                                                             *");
  Serial.println("*             Inicio del setup del modulo                     *");
  Serial.println("*                                                             *");    
  Serial.println("***************************************************************");

  //Configuracion general
  Serial.println("Init Config -----------------------------------------------------------------------");
  inicializaConfiguracion(debugGlobal);

  //Wifi
  Serial.println("Init WiFi -----------------------------------------------------------------------");
  if (inicializaWifi(true))//debugGlobal)) No tien esentido debugGlobal, no hay manera de activarlo
    {
    /*----------------Inicializaciones que necesitan red-------------*/
    //OTA
    Serial.println("Init OTA -----------------------------------------------------------------------");
    inicializaOTA(debugGlobal);
    //MQTT
    Serial.println("Init MQTT -----------------------------------------------------------------------");
    inicializaMQTT();
    //WebServer
    Serial.println("Init Web --------------------------------------------------------------------------");
    inicializaWebServer();
    }
  else Serial.println("No se pudo conectar al WiFi");

  //Sensores
  Serial.println("Init sensores ---------------------------------------------------------------------");
  inicializaSensores();

  //Ordenes serie
  Serial.println("Init Ordenes ----------------------------------------------------------------------");  
  inicializaOrden();//Inicializa los buffers de recepcion de ordenes desde PC

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
  if ((vuelta % frecuenciaMQTT)==0) atiendeMQTT(debugGlobal);
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
  frecuenciaOTA=5;
  frecuenciaLeeSensores=50;
  frecuenciaServidorWeb=1;
  frecuenciaOrdenes=2;
  frecuenciaMQTT=50;
  frecuenciaWifiWatchdog=100;  
  
  ahorroEnergia=0; //ahorro de energia desactivado por defecto
  IPControlador.fromString("0.0.0.0");
  for(int8_t id=0;id<MAX_SATELITES;id++) IPSatelites[id].fromString("0.0.0.0");  
    
  if(leeFichero(GLOBAL_CONFIG_FILE, cad)) parseaConfiguracionGlobal(cad);

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
    multiplicadorAnchoIntervalo=atol(json["multiplicadorAnchoIntervalo"]);
    anchoIntervalo=atol(json["anchoIntervalo"]);
    frecuenciaOTA=atol(json["frecuenciaOTA"]);
    frecuenciaLeeSensores=atol(json["frecuenciaLeeSensores"]);
    frecuenciaServidorWeb=atol(json["frecuenciaServidorWeb"]);
    frecuenciaOrdenes=atol(json["frecuenciaOrdenes"]);
    frecuenciaMQTT=atol(json["frecuenciaMQTT"]);
    frecuenciaWifiWatchdog=atol(json["frecuenciaWifiWatchdog"]);  
    
    ahorroEnergia=atoi(json["ahorroEnergia"]);
    direccion = atoi(json["id"]); // "5"
    IPControlador.fromString((const char *)json["IPControlador"]);
    IPSatelites[0].fromString((const char *)json["IPPrimerTermometro"]);
    IPGateway.fromString((const char *)json["IPGateway"]);
    for(int8_t id=1;id<MAX_SATELITES;id++)
      {
      IPSatelites[id]=IPSatelites[id-1];//copio la anterior
      IPSatelites[id][3]++;//paso a la siguiente
      }            
    Serial.printf("Configuracion leida:\ndireccion: %i\nIP controlador: %s\nIP primer satelite: %s\nIP Gateway: %s\nAhorro de energia: %i\n",direccion,IPControlador.toString().c_str(),IPSatelites[0].toString().c_str(),IPGateway.toString().c_str(), ahorroEnergia);
    Serial.printf("\nContadores\nmultiplicadorAnchoIntervalo: %i\nanchoIntervalo: %i\nfrecuenciaOTA: %i\nfrecuenciaLeeSensores: %i\nfrecuenciaServidorWeb: %i\nfrecuenciaOrdenes: %i\nfrecuenciaMQTT: %i\nfrecuenciaWifiWatchdog: %i\n",multiplicadorAnchoIntervalo, anchoIntervalo, frecuenciaOTA, frecuenciaLeeSensores,frecuenciaServidorWeb, frecuenciaOrdenes, frecuenciaMQTT, frecuenciaWifiWatchdog);
//************************************************************************************************
    }
  }

///////////////FUNCIONES COMUNES/////////////////////
/********************************************/
/*  Genera una cadena con todas las IPs     */
/********************************************/
String leerIPs(void)
  {
  String cad="";

  for(int8_t i=0;i<MAX_SATELITES;i++)
    {
    cad += "IP " + String(i) + ": ";
    cad += IPSatelites[i][0];
    cad += ".";
    cad += IPSatelites[i][1];
    cad += ".";
    cad += IPSatelites[i][2];
    cad += ".";
    cad += IPSatelites[i][3];
    cad += "\n";
    }
  return cad;
  }

/*************************************************/
/*  Dado un long, lo paso a binario y cambio los */
/*  bits pares. Devuelve el nuevo valor          */ 
/*************************************************/
int generaId(int id_in)
  {
  const long mascara=43690;
  return (id_in^mascara);
  }


const char* wl_status_to_string(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
  }
}  
