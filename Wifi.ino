/**********************************************/
/*                                            */
/*  Gestion del WiFi                          */
/*  Control de la conexion, SSID, clave       */
/*  Control de la IP, DefaulGw, DNS...        */
/*                                            */
/**********************************************/
#define NOMBRE_AP          "AP_actuador"
#define NOMBRE_mDNS_CONFIG "configurame"

//needed for library
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#define TRUE 1
#define FALSE 0
#define MAX_LONG_NOMBRE_DISPOSITIVO 32
#define WIFI_PORTAL_TIMEOUT 5*60 //5 minutos en segundos
#define TIME_OUT 10000
#define DELAY_MULTIBASE 1000

IPAddress wifiIPPrimerSatelite(0, 0, 0, 0);
IPAddress wifiIP(0, 0, 0, 0);//0.0.0.0 significa que no hay IP fija
IPAddress wifiNet(0, 0, 0, 0);
IPAddress wifiGW(0, 0, 0, 0);
IPAddress wifiDNS1(0, 0, 0, 0);
IPAddress wifiDNS2(0, 0, 0, 0);
String mDNS="";

const char* ssid;
const char* password;

ESP8266WiFiMulti MiWiFiMulti;

boolean conectado=false; //Si el portal de configuracion devolvio OK a la conexion

boolean inicializamDNS(const char *nombre)
  {  
  String mDNSnombre;
  
  if(nombre==NULL) mDNSnombre=NOMBRE_mDNS_CONFIG;//mDNS;
  else mDNSnombre=nombre;
  
  if (MDNS.begin(mDNSnombre.c_str()))
    {
    Serial.printf("mDNS iniciado. Nombre del dispositivo: %s\n",mDNSnombre.c_str());
    MDNS.addService("_http", "_tcp", 80);

    return true;    
    }
  else Serial.printf("Error al iniciar mDNS\n");

  return false;    
  }  

void salvaConfiguracion(void)
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

  //cargo el valores por defecto
  wifiIPPrimerSatelite=IPAddress(0,0,0,0);
  wifiIP=IPAddress(0,0,0,0);
  wifiGW=IPAddress(0,0,0,0);
  wifiNet=IPAddress(0,0,0,0);
  wifiDNS1=IPAddress(0,0,0,0);
  wifiDNS2=IPAddress(0,0,0,0);
  mDNS=NOMBRE_mDNS_CONFIG;
   
  if(!leeFichero(WIFI_CONFIG_FILE, cad)) 
    {
    Serial.printf("No existe fichero de configuracion WiFi o no es valido\n");//Confgiguracion por defecto
    return false;//NUEVO
    }

  return(parseaConfiguracionWifi(cad));
  }

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio Wifi                         */
/*********************************************/
boolean parseaConfiguracionWifi(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;

  Serial.printf("JSON a parsear:\n %s\n",contenido.c_str());
  
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    if (json.containsKey("wifiIP_PrimerSatelite")) wifiIP.fromString((const char *)json["wifiIP_PrimerSatelite"]); 
    if(wifiIP!=IPAddress(0,0,0,0)) 
      {
      wifiIPPrimerSatelite=wifiIP;
      wifiIP[3]+=direccion;  //calculo la IP del satelite en funcion de su direccion
      }

    if (json.containsKey("wifiGW")) wifiGW.fromString((const char *)json["wifiGW"]);
    if (json.containsKey("wifiNet")) wifiNet.fromString((const char *)json["wifiNet"]); 
    if (json.containsKey("wifiDNS1")) wifiDNS1.fromString((const char *)json["wifiDNS1"]);
    if (json.containsKey("wifiDNS2")) wifiDNS2.fromString((const char *)json["wifiDNS2"]);
    if (json.containsKey("mDNS")) mDNS=String((const char *)json["mDNS"]);
    
    Serial.printf("Configuracion leida:\nmDNS: %s\nIP actuador: %s\nIP Gateway: %s\nIPSubred: %s\nIP DNS1: %s\nIP DNS2: %s\n",mDNS.c_str(),wifiIP.toString().c_str(),wifiGW.toString().c_str(),wifiNet.toString().c_str(),wifiDNS1.toString().c_str(),wifiDNS2.toString().c_str());    

    if (!json.containsKey("wifi")) return false;

    JsonArray& wifi = json["wifi"];
    if(wifi.size()==0) return false;

    for(uint8_t i=0;i<wifi.size();i++)
      {
      const char* wifi_ssid = wifi[i]["ssid"];
      const char* wifi_password = wifi[i]["password"];
      MiWiFiMulti.addAP(wifi_ssid , wifi_password);
      Serial.printf("Red *%s* añadida.\n",wifi_ssid);
      }//del for
//************************************************************************************************
    return true;
    }
  return false;
  }

boolean inicializaWifi(boolean debug)
  {
  //Desconecto si esta conectado
  WiFi.disconnect(true);//(false);   
  //No reconecta a la ultima WiFi que se conecto
  WiFi.persistent(false);  
  //Activo el modo de autoreconexion nuevo en version 1.5 (con el cambio a esp8266 2.4.2)
  WiFi.setAutoReconnect(true);   
  //Activo el modo solo estacion, no access point
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);

  if(recuperaDatosWiFi(debug))
    {
    Serial.println("Conectando multibase");
    if (conectaMultibase(debug)) 
      {
      //Configuro la IP fija
      if (wifiIP!=IPAddress(0,0,0,0) && wifiGW!=IPAddress(0,0,0,0))
        {
        Serial.printf("Datos WiFi: IP fija-> %s, GW-> %s, subnet-> %s, DNS1-> %s, DNS2-> %s\n",wifiIP.toString().c_str(), wifiGW.toString().c_str(), wifiNet.toString().c_str(), wifiDNS1.toString().c_str(), wifiDNS2.toString().c_str());
        WiFi.config(wifiIP, wifiGW, wifiNet, wifiDNS1, wifiDNS2);
        }
      else Serial.println("No hay IP fija");

      //Inicializo mDNS para localizar el dispositivo
      inicializamDNS(mDNS.c_str());//nombre_dispositivo.c_str());
  
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
/*  WiFiManagerParameter Nombre_Parametro("1","dispositivo",nombre_dispositivo.c_str(),MAX_LONG_NOMBRE_DISPOSITIVO+1,"Nombre del dispositivo");
  wifiManager.addParameter(&Nombre_Parametro);*/
  //IP
  WiFiManagerParameter IPParametro("IP","IP","IP",15+1);
  wifiManager.addParameter(&IPParametro);
  //Gateway
  WiFiManagerParameter GatewayParametro("Gateway","Gateway","Gateway",15+1);
  wifiManager.addParameter(&GatewayParametro);
  //Subnet
  WiFiManagerParameter SubnetParametro("Subnet","Subnet","Subnet",15+1);
  wifiManager.addParameter(&SubnetParametro);
  //DNS1
  WiFiManagerParameter DNS1Parametro("DNS1","DNS1","DNS1",15+1);
  wifiManager.addParameter(&DNS1Parametro);
  //DNS2
  WiFiManagerParameter DNS2Parametro("DNS2","DNS2","DNS2",15+1);
  wifiManager.addParameter(&DNS2Parametro);
  //mDNS name
  WiFiManagerParameter mDNSParametro("mDNS","mDNS","mDNS",15+1);
  wifiManager.addParameter(&mDNSParametro);
  
  //preparo la llamada a la funcion para salvar configuracion, 
  //wifiManager.setSaveConfigCallback(miSaveConfigCallback);//Preparo la funcion para salvar la configuracion
  //wifiManager.setAPCallback(miAPCallback);//llamada cuando se actie el portal de configuracion
  wifiManager.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT);
  
  //Si se ha configurado IP fija
  //if (wifiIP!=IPAddress(0,0,0,0)) wifiManager.setSTAStaticIPConfig(wifiIP,wifiGW,wifiNet);//Preparo la IP fija (IPAddress ip, IPAddress gw, IPAddress sn) 

  //Inicializo mDNS para localizar la base
  inicializamDNS(NULL);//NOMBRE_mDNS_CONFIG);
  
  if (!wifiManager.startConfigPortal(NOMBRE_AP)) Serial.println("Error al conectar. Salida por time-out\n");      
  else 
    {
    wifiIP.fromString(String(IPParametro.getValue()));
    wifiIPPrimerSatelite=wifiIP; wifiIPPrimerSatelite[3]-=direccion; 
    wifiGW.fromString(String(GatewayParametro.getValue()));
    wifiNet.fromString(String(SubnetParametro.getValue()));
    wifiDNS1.fromString(String(DNS1Parametro.getValue()));
    wifiDNS2.fromString(String(DNS2Parametro.getValue()));
    mDNS=String(mDNSParametro.getValue()); //Serial.printf("longitud mDNS: %i\n",mDNSParametro.getValueLength());
    Serial.printf("Datos leidos del portal: \n Primer satelite-> %s\n IP fija-> %s\n GW-> %s\n subnet-> %s\n DNS1-> %s\n DNS2-> %s\n mDNS->#%s#\n",wifiIPPrimerSatelite.toString().c_str(), wifiIP.toString().c_str(), wifiGW.toString().c_str(), wifiNet.toString().c_str(), wifiDNS1.toString().c_str(), wifiDNS2.toString().c_str(),mDNS.c_str());

    salvaConfiguracion();
    Serial.printf("Configuracion finalizada correctamente.\n Reinciando...\n");
    }
  
  ESP.restart();
  return true;
  }

boolean conectaMultibase(boolean debug)
  {
  // wait for WiFi connection
  int time_out=0;
  while(MiWiFiMulti.run()!=WL_CONNECTED)
    {
    Serial.println("(Multi) Conectando Wifi...");  
    //incluido en la funcion parpadeo como delay(DELAY/2);  
    parpadeaLed(1,DELAY_MULTIBASE);
    time_out += DELAY_MULTIBASE;
    if (time_out>TIME_OUT) 
      {
      if (debug) Serial.println("No se pudo conectar al Wifi...");    
      return FALSE; //No se ha conectado y sale con KO
      }
    }
    
  return TRUE; //se ha conectado y sale con OK
  }

/**********************************************************************/
/*            Devuelve la IP configurada en el dispositivo            */
/**********************************************************************/ 
String getIP(int debug) { return WiFi.localIP().toString();}

/*********************************************************************/
/*       Devuelve el nombre de la red a la que se ha conectado       */
/*********************************************************************/ 
String nombreSSID(void) {return WiFi.SSID();}  

/*********************************************************************/
/*             Watchdog de control para la conexion WiFi             */
/*********************************************************************/ 
void WifiWD(void) {if(WiFi.status() != WL_CONNECTED) ESP.restart();}

/**********************************************************************/
/* Salva la configuracion de las bases wifi conectada en formato json */
/**********************************************************************/  
String generaJsonConfiguracionWifi(String configActual, String ssid, String password)
  {
  boolean nuevo=true;
  String salida="";

  //Genero el nuevo JSON
  //7 elementos, 2 arrays de 2 parametros cada uno
  const size_t capacity = JSON_ARRAY_SIZE(2) + 2*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(7) + 230;
  DynamicJsonBuffer jsonBufferNuevo(capacity);
  JsonObject& nuevoJson = jsonBufferNuevo.createObject();      

  //añado al nuevo los campos de IPs
  char cadIp[16]="0.0.0.0";
  char cadGw[16]="0.0.0.0";
  char cadNet[16]="0.0.0.0";
  char cadDNS1[16]="0.0.0.0";
  char cadDNS2[16]="0.0.0.0";

  if(wifiIPPrimerSatelite!=IPAddress(0,0,0,0)) 
    {
    strcpy(cadIp,wifiIPPrimerSatelite.toString().c_str());
    strcpy(cadGw,wifiGW.toString().c_str());
    strcpy(cadNet,wifiNet.toString().c_str());
    strcpy(cadDNS1,wifiDNS1.toString().c_str());
    strcpy(cadDNS2,wifiDNS2.toString().c_str());    
    }
    
  nuevoJson["mDNS"] = mDNS.c_str(); 
  nuevoJson["wifiIP_PrimerSatelite"] = cadIp; 
  nuevoJson["wifiGW"] = cadGw; 
  nuevoJson["wifiNet"] = cadNet; 
  nuevoJson["wifiDNS1"] = cadDNS1; 
  nuevoJson["wifiDNS2"] = cadDNS2; 

  Serial.printf("Traza: \n mDNS-> %s\n IP primer termometro-> %s\n GW-> %s\n subnet-> %s\n DNS1-> %s\n DNS2-> %s\n", mDNS.c_str(), cadIp, cadGw, cadNet, cadDNS1, cadDNS2);
  nuevoJson.printTo(Serial);
  Serial.println("");

/*
  Serial.printf("destino: %s | origen: %s\n",nuevoJson["mDNS"], mDNS.c_str());
  Serial.printf("destino: %s | origen: %s\n",nuevoJson["wifiIP"], cadIp); 
  Serial.printf("destino: %s | origen: %s\n",nuevoJson["wifiGW"] , cadGw); 
  Serial.printf("destino: %s | origen: %s\n",nuevoJson["wifiNet"] , cadNet); 
  Serial.printf("destino: %s | origen: %s\n",nuevoJson["wifiDNS1"] , cadDNS1); 
  Serial.printf("destino: %s | origen: %s\n",nuevoJson["wifiDNS2"] , cadDNS2); 

  nuevoJson["mDNS"] = "actuador";
  nuevoJson["wifiIP"] = "10.68.0.52";
  nuevoJson["wifiGW"] = "10.68.1.1";
  nuevoJson["wifiNet"] = "255.255.0.0";
  nuevoJson["wifiDNS1"] = "8.8.8.8";
  nuevoJson["wifiDNS2"] = "4.4.4.4";

  Serial.printf("destino: %s | origen: %s\n",nuevoJson.get<const char*>("mDNS"), mDNS.c_str());
  Serial.printf("destino: %s | origen: %s\n",nuevoJson.get<const char*>("wifiIP"), cadIp); 
  Serial.printf("destino: %s | origen: %s\n",nuevoJson.get<const char*>("wifiGW") , cadGw); 
  Serial.printf("destino: %s | origen: %s\n",nuevoJson.get<const char*>("wifiNet") , cadNet); 
  Serial.printf("destino: %s | origen: %s\n",nuevoJson.get<const char*>("wifiDNS1") , cadDNS1); 
  Serial.printf("destino: %s | origen: %s\n",nuevoJson.get<const char*>("wifiDNS2") , cadDNS2); 
*/

  JsonArray& nuevoWifi=nuevoJson.createNestedArray("wifi");
  
  //Abro el actual
  DynamicJsonBuffer jsonBuffer;  
  JsonObject& json = jsonBuffer.parseObject(configActual.c_str());
  json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("\nparsed json");   
/************************/
    JsonArray& wifi=(json.containsKey("wifi")?json["wifi"]:json.createNestedArray("wifi"));//parseo del fichero que he leido

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
          break;
          }        
        }
      }//del for

    //si es nuevo, lo añado al nuevo
    if(nuevo==true)
      {
      JsonObject& elemento = nuevoWifi.createNestedObject();
      elemento["ssid"] = ssid;
      elemento["password"] = password;
      Serial.printf("Red %s añadida\n",ssid.c_str());
      }

    //Copio del viejo al nuevo
    for(uint8_t i=0;i<wifi.size();i++)
      {
      JsonObject& elemento = nuevoWifi.createNestedObject();
      elemento["ssid"] = wifi[i]["ssid"];
      elemento["password"] = wifi[i]["password"];
      Serial.printf("Red %s copiada al nuevo fichero\n",elemento.get<const char*>("ssid"));    
      }      
    }//la de parsear el json
  else//si no pude parsear el original, añado al nuevo la red configurada
    {
    JsonObject& elemento = nuevoWifi.createNestedObject();
    elemento["ssid"] = ssid;
    elemento["password"] = password;
    Serial.printf("Red %s creada en el nuevo fichero\n",ssid.c_str());     
    }

/************************/
  nuevoJson.printTo(salida);//pinto el json que he creado
  Serial.printf("json creado:\n#%s#\n",salida.c_str());

  return salida;  
  }
