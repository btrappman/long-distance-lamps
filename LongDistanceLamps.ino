
/*  
 *   Long Distance Lamps
 *   Brandon Trappman
 *   btrappman@protonmail.com
 *   
 *   RGB lamp controlled remotely via google sheets. 
 *   
 *   NodeMCU checks cell value of a partnered google sheet, accessed
 *   using HTTPSRedirect. Google sheet must be published as a web app, 
 *   accessible by anyone (even anonymous). RGB values will change
 *   according to cell value.
 *   
 *   Thanks to electronicsguy for HTTPSRedirect library and google
 *   sheets and gscript examples.
 *   
 *   See https://github.com/electronicsguy/HTTPSRedirect
 *   
 *   Thanks to Taste The Code for the inspiration and building blocks,
 *   from his Controlling a light bulb through Google Sheets and GSDevice
 *   
 *   https://tastethecode.com
 *   
 *   Controlling a light bulb through Google Sheets
 *   https://www.youtube.com/watch?v=aKgtz3-gwrY
 *   
 *   GSDevice.ino
 *   https://github.com/bkolicoski/arduino-google-sheets-smarthome/blob/master/GSDevice/GSDevice.ino
 *   
 */

#include "HTTPSRedirect.h"
#include <ESP8266WiFi.h>

// Network credentials
const char* ssid = "";
const char* password = "";

// Domain for published google web app
const char* host = "script.google.com";

// Google web app id: script.google.com/macros/s/<GScriptID>/exec
const char *GScriptId = "";

// Google sheets cell addresses
const char *cellAddress = "A1";

// NodeMCU pins
const int RLED = 4;
const int GLED = 14;
const int BLED = 12;

const int httpsPort = 443;

// Web app url
String url3 = String("/macros/s/") + GScriptId + "/exec?read=" + cellAddress;

// Returned value
String payload = "";

HTTPSRedirect* client = nullptr;

void setup() {
  // Pin initialization
  pinMode(RLED, OUTPUT);
  pinMode(GLED, OUTPUT);
  pinMode(BLED, OUTPUT);

  // TODO: Not sure I need this, since we're doing analogwrite
  //digitalWrite(led, HIGH);

  // Connect to network; ensure correct baud rate in serial monitor
  Serial.begin(9600);
  Serial.flush();
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);

  // flush() is needed to print the above (connecting...) message reliably
  Serial.flush();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Ensure valid connection
  static int error_count = 0;
  static int connect_count = 0;
  const unsigned int MAX_CONNECT = 20;
  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(false);
    client->setContentTypeHeader("application/json");
  }

  if (client != nullptr){
    if (!client->connected()){
      client->connect(host, httpsPort);
    }
  }
  else{
    Serial.println("Error creating client object!");
    error_count = 5;
  }
  
  if (connect_count > MAX_CONNECT){
    // error_count = 5;
    connect_count = 0;
    flag = false;
    delete client;
    return;
  }

  // Make GET request
  Serial.print("GET Data from cell: ");
  Serial.println(cellAddress);
  if (client->GET(url3, host)){
    // Grab cell value
    payload = client->getResponseBody();
    payload.trim();
    payload.toLowerCase();
    
    // Set RGB values depending on cell value
    Serial.println(payload);
    if (payload == "red") {
      digitalWrite(RLED, HIGH);
      digitalWrite(GLED, LOW);
      digitalWrite(BLED, LOW);
    }
    else if (payload == "green") {
      digitalWrite(RLED, LOW);
      digitalWrite(GLED, HIGH);
      digitalWrite(BLED, LOW);
    }
    else if (payload == "blue") {
      digitalWrite(RLED, LOW);
      digitalWrite(GLED, LOW);
      digitalWrite(BLED, HIGH);
    }
    else {
      digitalWrite(RLED, LOW);
      digitalWrite(GLED, LOW);
      digitalWrite(BLED, LOW);
    }
    ++connect_count;
  }
  else{
    ++error_count;
    Serial.print("Error-count while connecting: ");
    Serial.println(error_count);
  }
  
  if (error_count > 3){
    Serial.println("Halting processor..."); 
    delete client;
    client = nullptr;
    Serial.flush();
    ESP.deepSleep(0);
  }
  
  // Delay so server doesn't get overwhelmed
  delay(1000);
                          
}
