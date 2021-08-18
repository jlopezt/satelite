/***************************************************************************************************************/
/*                                                                                                             */
/* https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php?ID=IMAJAD19&PASSWORD=v4fRRSYH&dateutc= */
/* Documentation: https://support.weather.com/s/article/PWS-Upload-Protocol?language=en_US                     */
/*                                                                                                             */
/***************************************************************************************************************/

#ifdef WUNDERGROUND
#include <StackThunk.h>

#include <WiFiClientSecure.h>
#include "system_variables.h"

String WU_pwsID       = "";// "IMAJAD19";
String WU_pwsPASSWORD = "";//"v4fRRSYH";

String WUHost      = "";//"https://www.wunderground.com"; //"weatherstation.wunderground.com";
uint16_t WUPort   = 0;//443;
//uint8_t fingerprint[20]; // = {0xb1, 0xdf, 0x0c, 0x07, 0x6f, 0x56, 0xd0, 0xa0, 0xc5, 0xc1, 0xcd, 0xad, 0x0e, 0x17, 0xd9, 0x04, 0x83, 0x9a, 0x67, 0x7c};
String fingerprint = "b1df0c076f56d0a0c5c1cdad0e17d904839a677c";

String timenow; 

/****************************************** Fin configuracion ********************************************/
/***************************************************/
/* Inicializa valiables de conexion a WUnderground */
/***************************************************/
void inicializaWU(void)
  {
  //recupero datos del fichero de configuracion
  if (!recuperaDatosWU(false)) Serial.printf("error al recuperar config WU.\nConfiguracion por defecto.\n");

  StartAndGetTime();
  }

/************************************************/
/* Recupera los datos de configuracion          */
/* del archivo de WUnderground                  */
/************************************************/
boolean recuperaDatosWU(boolean debug)
  {
  String cad="";
  if (debug) Serial.println("Recupero configuracion de archivo...");

  //cargo el valores por defecto
  WU_pwsID="";
  WU_pwsPASSWORD="";  
  WUHost="";
  WUPort=0;
  fingerprint="";

  if(!leeFichero(WU_CONFIG_FILE, cad))
    {
    Serial.printf("No existe fichero de configuracion WU o esta corrupto\n");
    return false;
    }

  return parseaConfiguracionWU(cad);
  }  

/*********************************************/
/* Parsea el json leido del fichero de       */
/* configuracio de WUnderground              */
/*********************************************/
boolean parseaConfiguracionWU(String contenido)
  {  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(contenido.c_str());
  //json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");
//******************************Parte especifica del json a leer********************************
    //cargo el valores por defecto
    WU_pwsID=json.get<String>("WU_pwsID");;
    WU_pwsPASSWORD=json.get<String>("WU_pwsPASSWORD");  
    WUHost=json.get<String>("WUHost");
    WUPort=json.get<uint16_t>("WUPort");
    fingerprint=json.get<String>("fingerprint");

    Serial.printf("Configuracion leida:\nId del pws: %s\nPassword del pws: %s\nWU host: %s\nWU port: %i\nFingerprint: %s\n",WU_pwsID.c_str(), WU_pwsPASSWORD.c_str(), WUHost.c_str(), WUPort, fingerprint.c_str());
//************************************************************************************************
    return true;
    }
  return false;
  }
/****************************************** Fin configuracion ********************************************/

/****************************************** Envio datos ********************************************/
/******************************************/
/*     Envia datos a WUnderground.com     */
/******************************************/
boolean UploadDataToWU(){
  UpdateTime();

  WiFiClientSecure WUclient; 
  WUclient.setInsecure();
  //WUclient.setFingerprint(fingerprint.c_str());

  // Use WiFiClientSecure class to create SSL connection
  Serial.println("Connecting to   : " + WUHost);
  if (!WUclient.connect(WUHost.c_str(), WUPort)) {
    Serial.println("Connection failed");
    return false;
  }

  Serial.printf("1\n");
  String url = "/weatherstation/updateweatherstation.php?ID="+WU_pwsID+"&PASSWORD="+WU_pwsPASSWORD+"&dateutc="+timenow +
               #ifdef SW_tempf
                 "&tempf="+ getTemperatura() +
               #endif
               #ifdef SW_humidity
                 "&humidity=" + getHumedad() +
               #endif
               #ifdef SW_dewptf
                 "&dewptf="+ String(getTemperatura()-(float)10.0) +             // ***** You must report Dew Point for a valid display to be shown on WU
               #endif
               #ifdef SW_baromin
                 "&baromin="+ getPresion() + 
               #endif
               // ancillary parameters
               #ifdef SW_winddir
                 "&winddir="+ 0 + 
               #endif
               #ifdef SW_windspeedmph
                 "&windspeedmph="+ 0 + 
               #endif
               #ifdef SW_windgustmph
                 "&windgustmph="+ 0 +
               #endif
               #ifdef SW_rainin
                 "&rainin="+ 0  +
               #endif
               #ifdef SW_dailyrainin
                 "&dailyrainin="+ 0 + 
               #endif
               #ifdef SW_solarradiation
                 "&solarradiation="+ getLuz() +
               #endif
               #ifdef SW_UV
                 "&UV="+ getLuz() +
               #endif
               /*****
               #ifdef SW_indoortempf
                 "&indoortempf="+ getTemperatura() + 
               #endif
               #ifdef SW_indoorhumidity
                 "&indoorhumidity="+ getHumedad() + 
               #endif
               #ifdef SW_soiltempf
                 "&soiltempf="+WU_soiltempf + 
               #endif
               #ifdef SW_soilmoisture
                 "&soilmoisture="+WU_soilmoisture + 
               #endif
               #ifdef SW_leafwetness
                 "&leafwetness="+WU_leafwetness + 
               #endif
               #ifdef SW_visibility
                 "&visibility="+WU_visibility + 
               #endif
               #ifdef SW_weather
                 "&weather="+WU_weather +
               #endif
               #ifdef SW_clouds
                 "&clouds="+WU_clouds +
               #endif
               ***********/
               "&action=updateraw&realtime=1&rtfreq=60";
  Serial.println("Requesting      : "+url);
  WUclient.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + WUHost + "\r\n" +
               //"User-Agent: G6EJDFailureDetectionFunction\r\n" +
               "User-Agent: Satelite - ESP8266\r\n" +
               "Connection: close\r\n\r\n");
  Serial.print("Request sent    : ");
  while (WUclient.connected()) {
    String line = WUclient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Headers received");
      break;
    }
  }
  String line = WUclient.readStringUntil('\n');
  //Serial.println(line);
  boolean Status = true;
  if (line == "success") line = "Server confirmed all data received";
  if (line == "INVALIDPASSWORDID|Password or key and/or id are incorrect") {
    line = "Invalid PWS/User data entered in the ID and PASSWORD or GET parameters";
    Status = false;
  }
  if (line == "RapidFire Server") {
    line = "The minimum GET parameters of ID, PASSWORD, action and dateutc were not set correctly";
    Status = false;
  }
  Serial.println("Server Response : "+line);
  Serial.println("Status          : Closing connection");
  return Status;
}

void StartAndGetTime(){
  configTime(0, 0, "0.uk.pool.ntp.org", "time.nist.gov"); 
  setenv("TZ", "GMT0BST,M3.5.0/01,M10.5.0/02",1); 
  delay(200); 
  time_t rawtime;
  struct tm *info;
  char buffer[80];
  time( &rawtime );
  info = localtime( &rawtime );
  // Upload format for time = dateutc [CCYY-MM-DD HH:MM:SS (mysql format)]
  // 2018-04-30 10:32:35 becomes 2018-04-30+10%3A32%3A35 in url escaped format
  // See: http://www.cplusplus.com/reference/ctime/strftime/
  //Serial.println("Upload date-time: CCYY-MM-DD HH:MM:SS   expected");
  strftime(buffer,80,"%Y-%m-%d+%H:%M:%S", info);
  //printf("Upload date-time: %s\n", buffer );
  timenow = buffer; // timenow is correctly formated for WU
  timenow.replace(":", "%3A");
}

void UpdateTime(){
  time_t rawtime;
  struct tm *info;
  char buffer[80];
  time( &rawtime );
  info = localtime( &rawtime );
  strftime(buffer,80,"%Y-%m-%d+%H:%M:%S", info);
  printf("Upload date-time: %s\n", buffer );
  timenow = buffer; 
  timenow.replace(":", "%3A"); // timenow is correctly formated for WU
}
#endif
