/**
 * BasicHTTPClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <ESP8266HTTPClient.h>

struct tipo_respuestaHTTP 
  {
  String payload="";
  int httpCode=-1;
  };

void inicializaClienteHTTP() 
  {
  }

/**********************************************************/
/* Recive un String (URL)                                 */
/* y devuelve una estructura con codigo http y payload    */
/**********************************************************/
struct tipo_respuestaHTTP ClienteHTTP(String URL) 
  {
  struct tipo_respuestaHTTP respuestaHTTP;
  HTTPClient http;

  if(URL=="") return respuestaHTTP;//Si esta vacio salgo rapido

  http.begin(URL);//HTTP
  http.setTimeout(2000);  

  // start connection and send HTTP header
  respuestaHTTP.httpCode= http.GET();

  if(respuestaHTTP.httpCode > 0) //httpCode will be negative on error
    {
    // HTTP header has been send and Server response header has been handled
    if(respuestaHTTP.httpCode == HTTP_CODE_OK) respuestaHTTP.payload=http.getString();
    } 
  else Serial.printf("Error comunicacion. \n Peticion: %s\n error: %i : %s\n", URL.c_str(), respuestaHTTP.httpCode, http.errorToString(respuestaHTTP.httpCode).c_str());

  http.end();
  return respuestaHTTP;
  }  
