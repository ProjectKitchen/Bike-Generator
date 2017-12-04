//parts of this code are taken from: https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/example-sketch-ap-web-server
#include <ESP8266WiFi.h>

//wifi password
const char WiFiAPPSK[] = "testing";

WiFiServer server(80);

void setup() 
{
  setupWiFi();
  server.begin();
  Serial.begin(115200);
}

void loop() 
{
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  client.flush();
  
  // use 'val' to see if the correct request has been made, 
  // if so send a request to recieve data from the Atmegea328P
  int val = -1; 
  if (req.indexOf("/read") != -1)
  {
    val = 1;
    Serial.write('1');
  }

  client.flush();

  // Star to build the response. Start with the common header:
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n";
  s += "Refresh: 1\r\n";
  s += "\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";

  // if data has been transmitted start to read it and output it
  // to the string s in an already user friendly format.
  if (val == 1)
  { 
    if (Serial.available() > 0)  
    {
      s += "Data from 328P: ";
      s += "<br><br>Input Data:";
      s += "<br>Input Voltage: ";
      s += String(float(Serial.readStringUntil(',').toInt())*0.0234375);
      s += "V";
      s += "<br>Input Current:  ";
      s += String(float((Serial.readStringUntil(',').toInt())-512)*-0.04878);
      s += "A";
      s += "<br><br>Output Data:";
      s += "<br>Output Voltage: ";
      s += String(float(Serial.readStringUntil(',').toInt())*0.0234375);
      s += "V";
      s += "<br>Battery Current:  ";
      s += String(float((Serial.readStringUntil(',').toInt())-512)*-0.04878);
      s += "A";
      s += "<br>Rectifier Current: ";
      s += String(float((Serial.readStringUntil(',').toInt())-512)*-0.04878);
      s += "A";
      s += "<br><br>Converter Status:";
      s += "<br>Converter Mode: ";
      int converterMode = Serial.readStringUntil(',').toInt();
      if(converterMode == 1)
      {s += "Buck ";}
      else if(converterMode == 2)
      {s += "Boost ";}
      else
      {s += "Off or error";}
      s += "<br>PWM of T1 (Duty): ";
      s += String(float(Serial.readStringUntil(',').toInt())*0.003921);
      s += "<br>PWM of T2 (Duty): ";
      s += String(float(Serial.readStringUntil(',').toInt())*0.003921);
      s += "<br>Message Counter: ";
      s += String(Serial.readStringUntil('#'));
    }
    
    //if no data has come back, eihter the Atmega 328p is busy or the switches are in the wrong state.
    else
    {
      s += String("no data, check Uart connections");
    }
  }
  else
  {
    s += "Invalid Request.<br> Try /read.";
  }
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  
  // The client will be disconnected 
  // when the function returns and 'client' object is detroyed
}

//WIFI Setup as per the url at the top of the code
void setupWiFi()
{
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "ESP8266 Thing " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);
}

