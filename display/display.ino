#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino_JSON.h>
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

// Initialize the OLED display using Wire library
SSD1306Wire  display(0x3c, D2, D1);  //D2=SDK  D1=SCK  As per labeling on NodeMCU

//=======================================================================
//                    Power on setup
//=======================================================================

double temp = 0;
double hum = 0;
String polusi = "...";
// Connections
const char *ssid = "SMK TRUNOJOYO";
const char *password = "tanyamasoki";
const char *server = "ecopy.ygnv.my.id";

void setup() {
  delay(1000);
  Serial.begin(9600);  

  Serial.println("");
  
  Serial.println("Initializing OLED Display");
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  
  // Connection begin
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.hostname("ESP Orange");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 20, "Connecting to WiFi");
    display.display();
    delay(500);
  }
  digitalWrite(LED_BUILTIN, LOW);
  // Connection end

}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  suhutemp();
  delay(2000);
}
//=========================================================================


void drawFontFaceDemo() {
  // clear the display
  display.clear();
    // Font Demo1
    // create more fonts at http://oleddisplay.squix.ch/
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Hello world");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 10, "Hello world");
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 26, "Hello world");
  // write the buffer to the display
  display.display();
}

void suhutemp() {

  WiFiClient client;
  String Link;
  HTTPClient http;
  Link = "http://"+ String(server) +"/display";
  http.begin(client, Link);
  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(httpCode);
  
  JSONVar myObject = JSON.parse(http.getString());
  temp = myObject["Temperature"];
  hum = myObject["Humidity"];
  polusi = String(myObject["Polution"]);
  // clear the display
  display.clear();
    // Font Demo1
    // create more fonts at http://oleddisplay.squix.ch/
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(1, 1, String(temp) + " °C    |    " + String(hum) + " %");
    display.drawHorizontalLine(1, 25, 124);
    display.setFont(ArialMT_Plain_24);
    display.drawString(1, 30, polusi);
  // write the buffer to the display
  display.display();

  } else {
    Serial.println("Error on HTTP request");
  }
  http.end();
}
