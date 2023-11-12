#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino_JSON.h>
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, D2, D1); // D2=SDK  D1=SCK  As per labeling on NodeMCU

//=======================================================================
//                    Power on setup
//=======================================================================
const int SCREEN_WIDTH = 128; // Add this line
const int SCREEN_HEIGHT = 64; // Add this line
double temp = 0;
double hum = 0;
String polusi = "...";
unsigned long previousMillis = 0; // Variable to store last time suhutemp() was called
const long interval = 60000;      // Interval at which to run suhutemp() in milliseconds
// Connections
const char *ssid = "SMK TRUNOJOYO";
const char *password = "tanyamasoki";
const char *server = "ecopy.ygnv.my.id";

void setup()
{
  delay(1000);
  Serial.begin(9600);

  Serial.println("");

  Serial.println("Initializing OLED Display");
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  // Connection begin
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.hostname("ESP Display");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 20, "Connecting...");
    display.display();
    delay(500);
  }
  digitalWrite(LED_BUILTIN, LOW);
  // Connection end
  suhutemp();
}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    reconnectWiFi();
  }
  unsigned long currentMillis = millis(); // Get the current time

  // Check if 60 seconds have passed since the last call to suhutemp()
  if (currentMillis - previousMillis >= interval)
  {
    suhutemp();                     // Call suhutemp()
    previousMillis = currentMillis; // Save the current time
  }
  delay(2000);
}
//=========================================================================

void reconnectWiFi()
{
  Serial.println("Reconnecting to Wi-Fi...");
  WiFi.reconnect();
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    // Serial.println("\nConnected to Wi-Fi");
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    // Serial.println("\nFailed to reconnect to Wi-Fi");
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void drawFontFaceDemo()
{
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

void suhutemp()
{

  WiFiClient client;
  String Link;
  HTTPClient http;
  Link = "http://" + String(server) + "/display";
  http.begin(client, Link);
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    String payload = http.getString();
    Serial.println(httpCode);

    JSONVar myObject = JSON.parse(http.getString());
    temp = myObject["Temperature"];
    hum = myObject["Humidity"];
    polusi = String(myObject["Polution"]);
    // clear the display
    String formattedTemp = String(temp, 1);
    String formattedHum = String(hum, 1);
    display.clear();
    // Font Demo1
    // create more fonts at http://oleddisplay.squix.ch/
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(1, 2, String(formattedTemp) + " °C  |  " + String(formattedHum) + " %");
    display.drawHorizontalLine(1, 28, 124);
    display.setFont(ArialMT_Plain_24);
    display.drawString(1, 35, polusi);
    // write the buffer to the display
    display.display();
  }
  else
  {
    Serial.println("Error on HTTP request");
  }
  http.end();
}
