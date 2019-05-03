/**********************************************/
/*                                            */
/*  Gestion del WiFi                          */
/*  Control de la conexion, SSID, clave       */
/*  Control de la IP, DefaulGw, DNS...        */
/*                                            */
/**********************************************/
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

//needed for library
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#define TRUE 1
#define FALSE 0
#define MAX_LONG_NOMBRE_DISPOSITIVO 32
#define WIFI_PORTAL_TIMEOUT 5*60 //5 minutos en segundos
#define TIME_OUT 30000
#define DELAY 1000

IPAddress wifiIPFija(0, 0, 0, 0);//0.0.0.0 significa que no hay IP fija
IPAddress wifiNet(255, 255, 255, 0);
IPAddress wifiGW(0, 0, 0, 0);//(10, 68, 1, 1);
IPAddress wifiDNS1(8, 8, 8, 8);//los de google
IPAddress wifiDNS2(8, 8, 4, 4);

const char* ssid;
const char* password;

ESP8266WiFiMulti WiFiMulti;

boolean conectado=false; //Si el portal de configuracion devolvio OK a la conexion

void miSaveConfigCallback(void)
  {
  String cad="";
  
  Serial.println("---------------------Salvando configuracion---------------");
  Serial.println("Valores que voy a salvar");
  Serial.print("SSID : ");
  Serial.println(WiFi.SSID());
  Serial.print("Password : ");
  Serial.println(WiFi.psk());   

  if(!leeFichero(WIFI_CONFIG_FILE, cad)) Serial.println("No se pudo leer el fichero");
  cad=generaJsonConfiguracionWifi(cad, WiFi.SSID(),WiFi.psk());
  if(!salvaFichero(WIFI_CONFIG_FILE, WIFI_CONFIG_BAK_FILE, cad)) Serial.println("No se pudo salvar el fichero");  
  Serial.println("---------------------Fin salvando configuracion---------------");
  
  conectado=true;
  }

/*
void miAPCallback(WiFiManager *myWifiManager)
  {
  Serial.println("Portal de configuracion levantado");  
  Serial.print("Base activada con el nombre ");
  Serial.println(myWifiManager->getConfigPortalSSID());  
  }
*/

/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo de Wifi                          */
/************************************************/
boolean recuperaDatosWiFi(boolean debug)
  {
  String cad="";
  if (debug) Serial.println("Recupero configuracion de archivo...");

  //cargo valores ya leidos
  wifiIPFija=IPSatelites[direccion];//Configuro la IP que se leyo en la configuracion general
  wifiGW=IPGateway;//Configuro la IP que se leyo en la configuracion general

  //cargo el valores por defecto
  ////////No aplican en este caso
    
  if(leeFichero(WIFI_CONFIG_FILE, cad)) parseaConfiguracionWifi(cad);

  return true;
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio Wifi                         */
/*********************************************/
boolean parseaConfiguracionWifi(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    JsonArray& wifi = json["wifi"];

    for(uint8_t i=0;i<wifi.size();i++)
      {
      const char* wifi_ssid = wifi[i]["ssid"];
      const char* wifi_password = wifi[i]["password"];
      WiFiMulti.addAP(wifi_ssid , wifi_password);
      Serial.printf("Red *%s* añadida.\n",wifi_ssid);
      }//del for
//************************************************************************************************
    return true;
    }
  return false;
  }

boolean inicializaWifi(boolean debug)
  {
  if(recuperaDatosWiFi(debug))
    {
    //Configuro la IP fija
    if (wifiIPFija!=(0,0,0,0) && wifiGW!=(0,0,0,0))
      {
      Serial.printf("Datos WiFi: IP fija-> %s, GW-> %s, subnet-> %s, DNS1-> %s, DNS2-> %s\n",wifiIPFija.toString().c_str(), wifiGW.toString().c_str(), wifiNet.toString().c_str(), wifiDNS1.toString().c_str(), wifiDNS2.toString().c_str());
      WiFi.config(wifiIPFija, wifiGW, wifiNet, wifiDNS1, wifiDNS2);
      }
    else Serial.println("No hay IP fija");

    //Activo el modo de autoreconexion nuevo en version 1.5 (con el cambio a esp8266 2.4.2)
    WiFi.setAutoReconnect(true);

    Serial.println("Conectando multibase");
    if (conectaMultibase(debug)) 
      {
      Serial.println("------------------------WiFi conectada (configuracion almacenada)--------------------------------------");
      Serial.println("WiFi conectada");
      WiFi.printDiag(Serial);
      Serial.print("IP : ");
      Serial.println(WiFi.localIP());
      Serial.println("-------------------------------------------------------------------------------------------------------");

      return true;
      }
    }

  Serial.println("Conectando autodetect");
  if (conectaAutodetect(debug))
    {
    Serial.println("------------------------WiFi conectada(autodetect)--------------------------------------");
    Serial.println("WiFi conectada");
    WiFi.printDiag(Serial);
    Serial.print("IP : ");
    Serial.println(WiFi.localIP());
    Serial.println("----------------------------------------------------------------------------------------");
    
    return true;
    }
  return false;
  }

boolean conectaAutodetect(boolean debug)
  {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  Serial.println("\n Entrando...");
  
  //WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);    
  WiFiManagerParameter Nombre_Parametro("1","dispositivo",nombre_dispoisitivo.c_str(),MAX_LONG_NOMBRE_DISPOSITIVO+1,"Nombre del dispositivo");
  Serial.println(Nombre_Parametro.getID());
  Serial.println(Nombre_Parametro.getValue());
  Serial.println(Nombre_Parametro.getPlaceholder());
  Serial.println(Nombre_Parametro.getValueLength());
  Serial.println(Nombre_Parametro.getCustomHTML());
  wifiManager.addParameter(&Nombre_Parametro);
  
  //preparo la llamada a la funcion para salvar configuracion, 
  wifiManager.setSaveConfigCallback(miSaveConfigCallback);//Preparo la funcion para salvar la configuracion
  wifiManager.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT);
  //wifiManager.setAPCallback(miAPCallback);//llamada cuando se actie el portal de configuracion
  
  //Si se ha configurado IP fija
  if (wifiIPFija!=(0,0,0,0)) wifiManager.setSTAStaticIPConfig(wifiIPFija,wifiGW,wifiNet);//Preparo la IP fija (IPAddress ip, IPAddress gw, IPAddress sn) 

  if (!wifiManager.startConfigPortal(("AP_"+nombre_dispoisitivo+"_"+direccion).c_str())) 
    {
    Serial.println("failed to connect and hit timeout");
    ESP.restart();
    }
    
  return true;
  }

boolean conectaMonobase(boolean debug)
  { 
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) 
    {
    delay(100);
    if (debug) Serial.print("(MonoBase)Conectando WiFi....");
    }
 
  return TRUE;
  }

boolean conectaMultibase(boolean debug)
  {
  // wait for WiFi connection
  int time_out=0;
  while(WiFiMulti.run()!=WL_CONNECTED)
    {
    Serial.println("(Multi) Conectando Wifi...");  
    delay(DELAY);  
    time_out += DELAY;
    if (time_out>TIME_OUT) 
      {
      if (debug) Serial.println("No se pudo conectar al Wifi...");    
      return FALSE; //No se ha conectado y sale con KO
      }
    }
    
  return TRUE; //se ha conectado y sale con OK
  }

String getIP(int debug)
  {  
  return WiFi.localIP().toString();
  }

/**********************************************************************/
/* Salva la configuracion de las bases wifi conectada en formato json */
/**********************************************************************/  
String generaJsonConfiguracionWifi(String configActual, String ssid, String password)
  {
  boolean nuevo=true;
  String salida="";

  if(configActual=="") 
    {
    Serial.println("No existe el fichero. Se genera uno nuevo");
    return "{\"wifi\":[{\"ssid\":\"" + ssid + "\",\"password\":\"" + password + "\"}]}";;
    }
    
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(configActual.c_str());
  json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");          
    JsonArray& wifi = json["wifi"];//parseo del ficehro que he leido
/************************/
    for(uint8_t i=0;i<wifi.size();i++)
      {
      if(strcmp(wifi[i]["ssid"],"NONE")) //Si la base no es NONE que es la que pongo cuando no hay fichero
        {
        //Comparo el que he leido del json con el que acabao de usar para conectar
        if (!strcmp(wifi[i]["ssid"],ssid.c_str())) //si ya existe actualizo la password
          {
          wifi[i]["password"] = password; //si son iguales guardo la password nueva
          Serial.printf("Se ha modificado la pass de %s\n",(const char *)wifi[i]["ssid"]);
          nuevo=false;//no es nuevo, ya he guardado el que acabo de usar
          }        
        }
      }//del for
      
    if(nuevo==true)
      {
      JsonObject& elemento = wifi.createNestedObject();
      elemento["ssid"] = ssid;
      elemento["password"] = password;
      Serial.printf("Red %s añadida\n",ssid.c_str());
      }
/************************/
    json.printTo(salida);//pinto el json que he creado
    Serial.printf("json creado:\n#%s#\n",salida.c_str());

    }//la de parsear el json

  return salida;  
  }

String nombreSSID(void) {return WiFi.SSID();}  

void WifiWD(void)
  {
  if(WiFi.status() != WL_CONNECTED) ESP.restart();
  }
