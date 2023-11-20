#include <Adafruit_ADS1X15.h>
#include <MQUnifiedsensor.h>
#include <DHTesp.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <Arduino_JSON.h>

#define PM1PIN 12  // DSM501A input D6 on ESP8266 Warna merah
#define PM25PIN 14 // warna kuning

byte buff[2];
unsigned long durationPM1;
unsigned long durationPM25;
unsigned long starttime;
unsigned long endtime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancyPM1 = 0;
unsigned long lowpulseoccupancyPM25 = 0;
int i = 0;
float ratio1 = 0;
float ratio2 = 0;
float PM10 = 0;
float PM25 = 0;
float h = 0;
float t = 0;

unsigned long previousMillis = 0;
const long interval = 60000;
// Definitions
#define placa "ESP8266"
#define mq2 "MQ-2"     // MQ2
#define mq7 "MQ-7"     // MQ7
#define mq135 "MQ-135" // MQ135
#define Voltage_Resolution 5
#define pin A0                 // Analog input 0 of your arduino
#define ADC_Bit_Resolution 10  // For arduino UNO/MEGA/NANO
#define RatioMQ2CleanAir 9.83  // RS / R0 = 9.83 ppm
#define RatioMQ7CleanAir 27.5  // RS / R0 = 27.5 ppm
#define RatioMQ135CleanAir 3.6 // RS / R0 = 3.6 ppm
float factorEscala = 0.16875F;
// #define calibration_button 13 //Pin to calibrate your sensor
// Declare Sensor
MQUnifiedsensor MQ2(placa, mq2);
MQUnifiedsensor MQ7(placa, mq7);
// MQUnifiedsensor MQ135(placa, mq135);
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, mq135);

DHTesp dht;
Adafruit_ADS1115 ads;

// Connections
const char *ssid = "SMK TRUNOJOYO";
const char *password = "tanyamasoki";
const char *server = "ecoguardian.ygnv.my.id";
String espId = "cloodu0dk0000pff06r0pacus";
// #define ledPin 16 // pin D0

void setup(void)
{
  // Init the serial port communication - to debug the library
  Serial.begin(9600); // Init serial port
  delay(200);

  // Connection begin
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.hostname("ESP Orange");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }
  digitalWrite(LED_BUILTIN, LOW);
  // Connection end

  pinMode(PM1PIN, INPUT);
  pinMode(PM25PIN, INPUT);
  starttime = millis();

  dht.setup(13, DHTesp::DHT11); // DHT Sensor
  // Set math model to calculate the PPM concentration and the value of constants
  MQ2.setRegressionMethod(1);   //_PPM =  a*ratio^b
  MQ7.setRegressionMethod(1);   //_PPM =  a*ratio^b
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b

  /************************************************************************************/
  MQ2.init();
  MQ7.init();
  MQ135.init();

  // Iniciar el ADS1115
  ads.begin();
  if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS.");
    while (1)
      ;
  }

  Serial.print("Calibrating please wait.");
  float calcR0MQ135 = 0;
  for (int i = 1; i <= 10; i++)
  {
    // Obtener datos del A0 del ADS1115
    MQ135.update();
    calcR0MQ135 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0MQ135 / 10);
  Serial.println("  done! for MQ-135.");

  // Calibration code for MQ-2
  Serial.print("Calibrating MQ-2 please wait.");
  float calcR0MQ2 = 0;
  for (int i = 1; i <= 10; i++)
  {
    short adc0 = ads.readADC_SingleEnded(0); // Assuming MQ-2 is connected to A1
    float voltiosMQ2 = (adc0 * factorEscala) / 1000.0;
    MQ2.externalADCUpdate(voltiosMQ2);
    calcR0MQ2 += MQ2.calibrate(RatioMQ2CleanAir);
    Serial.print(".");
  }
  MQ2.setR0(calcR0MQ2 / 10);
  Serial.println("  done for MQ-2.");

  // Calibration code for MQ-7
  Serial.print("Calibrating MQ-7 please wait.");
  float calcR0MQ7 = 0;
  for (int i = 1; i <= 10; i++)
  {
    short adc1 = ads.readADC_SingleEnded(1); // Assuming MQ-7 is connected to A2
    float voltiosMQ7 = (adc1 * factorEscala) / 1000.0;
    MQ7.externalADCUpdate(voltiosMQ7);
    calcR0MQ7 += MQ7.calibrate(RatioMQ7CleanAir);
    Serial.print(".");
  }
  MQ7.setR0(calcR0MQ7 / 10);
  Serial.println("  done for MQ-7.");
}

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

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    reconnectWiFi();
  }
  // Get data from the sensor
  MQ135.update();

  MQ135.setA(605.18);
  MQ135.setB(-3.937);                  // Configure the equation to calculate CO concentration value
  float MQ135_CO = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(77.255);
  MQ135.setB(-3.18);                        // Configure the equation to calculate Alcohol concentration value
  float MQ135_Alcohol = MQ135.readSensor(); // SSensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(110.47);
  MQ135.setB(-2.862);                   // Configure the equation to calculate CO2 concentration value
  float MQ135_CO2 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(44.947);
  MQ135.setB(-3.445);                      // Configure the equation to calculate Toluen concentration value
  float MQ135_Toluen = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(102.2);
  MQ135.setB(-2.473);                   // Configure the equation to calculate NH4 concentration value
  float MQ135_NH4 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(34.668);
  MQ135.setB(-3.369);                      // Configure the equation to calculate Aceton concentration value
  float MQ135_Aceton = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  short adc0 = ads.readADC_SingleEnded(0); // Assuming MQ-2 is connected to A1
  float voltiosMQ2 = (adc0 * factorEscala) / 1000.0;
  MQ2.externalADCUpdate(voltiosMQ2);

  MQ2.setA(987.99);
  MQ2.setB(-2.162);                // Configure the equation to calculate LPG concentration value
  float MQ2_H2 = MQ2.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ2.setA(574.25);
  MQ2.setB(-2.222);                 // Configure the equation to calculate CH4 concentration value
  float MQ2_LPG = MQ2.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ2.setA(36974);
  MQ2.setB(-3.109);                // Configure the equation to calculate CH4 concentration value
  float MQ2_CO = MQ2.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ2.setA(3616.1);
  MQ2.setB(-2.675);                     // Configure the equation to calculate CH4 concentration value
  float MQ2_Alcohol = MQ2.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ2.setA(658.71);
  MQ2.setB(-2.168);                     // Configure the equation to calculate CH4 concentration value
  float MQ2_Propane = MQ2.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  short adc1 = ads.readADC_SingleEnded(1); // Assuming MQ-7 is connected to A2
  float voltiosMQ7 = (adc1 * factorEscala) / 1000.0;
  MQ7.externalADCUpdate(voltiosMQ7);

  MQ7.setA(69.014);
  MQ7.setB(-1.374);                // Configure the equation to calculate H2 concentration value
  float MQ7_H2 = MQ7.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ7.setA(700000000);
  MQ7.setB(-7.703);                 // Configure the equation to calculate LPG concentration value
  float MQ7_LPG = MQ7.readSensor(); //

  MQ7.setA(60000000000000);
  MQ7.setB(-10.54);                 // Configure the equation to calculate CH4 concentration value
  float MQ7_CH4 = MQ7.readSensor(); //

  MQ7.setA(99.042);
  MQ7.setB(-1.518);                // Configure the equation to calculate CO concentration value
  float MQ7_CO = MQ7.readSensor(); //

  MQ7.setA(40000000000000000);
  MQ7.setB(-12.35);                     // Configure the equation to calculate Alcohol concentration value
  float MQ7_Alcohol = MQ7.readSensor(); //
  /*
  Motivation:
  We have added 200 PPM because when the library is calibrated it assumes the current state of the
  air as 0 PPM, and it is considered today that the CO2 present in the atmosphere is around 400 PPM.
  https://www.lavanguardia.com/natural/20190514/462242832581/concentracion-dioxido-cabono-co2-atmosfera-bate-record-historia-humanidad.html
  */
  durationPM1 = pulseIn(PM1PIN, LOW);
  durationPM25 = pulseIn(PM25PIN, LOW);

  lowpulseoccupancyPM1 += durationPM1;
  lowpulseoccupancyPM25 += durationPM25;

  endtime = millis();
  if ((endtime - starttime) > sampletime_ms) // Only after 30s has passed we calcualte the ratio
  {
    ratio1 = lowpulseoccupancyPM1/(sampletime_ms*10.0);  // Integer percentage 0=>100
    PM10 = 1.1*pow(ratio1,3)-3.8*pow(ratio1,2)+520*ratio1+0.62; // using spec sheet curve

    ratio2 = lowpulseoccupancyPM25/(sampletime_ms*10.0);  // Integer percentage 0=>100
    PM25 = 1.1*pow(ratio2,3)-3.8*pow(ratio2,2)+520*ratio2+0.62; // 

    // conPM1 = calculateConcentration(lowpulseoccupancyPM1, 30);
    // conPM25 = calculateConcentration(lowpulseoccupancyPM25, 30);
    // PM10 = conPM1*1000;
    // PM25 = conPM25*1000;
    // Serial.print("PM1 ");
    // Serial.print(conPM1);
    // Serial.print("  PM25 ");
    // Serial.println(conPM25);
    lowpulseoccupancyPM1 = 0;
    lowpulseoccupancyPM25 = 0;
    starttime = millis();
  }
  // Print the values on the serial monitor
  // Serial.print("Sensor\t\t\t");
  // Serial.print("Value (ppm)");
  // Serial.println();

  // Serial.print("MQ-135 CO:\t\t");
  // Serial.print(MQ135_CO, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-135 Alcohol:\t");
  // Serial.print(MQ135_Alcohol, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-135 CO2:\t\t");
  // Serial.print(MQ135_CO2 + 400, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-135 Toluen:\t");
  // Serial.print(MQ135_Toluen, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-135 NH4:\t\t");
  // Serial.print(MQ135_NH4, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-135 Aceton:\t");
  // Serial.print(MQ135_Aceton, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-2 H2:\t\t");
  // Serial.print(MQ2_H2, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-2 LPG:\t\t");
  // Serial.print(MQ2_LPG, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-2 CO:\t\t");
  // Serial.print(MQ2_CO, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-2 Alcohol:\t");
  // Serial.print(MQ2_Alcohol, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-2 Propane:\t");
  // Serial.print(MQ2_Propane, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-7 H2:\t\t");
  // Serial.print(MQ7_H2, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-7 LPG:\t\t");
  // Serial.print(MQ7_LPG, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-7 CH4:\t\t");
  // Serial.print(MQ7_CH4, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-7 CO:\t\t");
  // Serial.print(MQ7_CO, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  // Serial.print("MQ-7 Alcohol:\t");
  // Serial.print(MQ7_Alcohol, 2); // Display with 2 decimal places
  // Serial.println(" ppm");

  h = dht.getHumidity();
  t = dht.getTemperature();
  // Serial.print("Kelembaban:\t\t");
  // Serial.print(h, 2); // Display with 2 decimal places
  // Serial.print("%\t");

  // Serial.print("Temperatur:\t\t");
  // Serial.print(t, 2); // Display with 2 decimal places
  // Serial.println("°C");

  // Serial.print("PM1:\t\t\t");
  // Serial.print(conPM1, 2); // Display with 2 decimal places
  // Serial.println();

  // Serial.print("PM2.5:\t\t\t");
  // Serial.print(conPM25, 2); // Display with 2 decimal places
  // Serial.println();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    WiFiClient client;
    String Link;
    HTTPClient http;
    Link = "http://" + String(server) + "/api/socket/data";
    http.begin(client, Link);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Accept", "application/json");
    String jsondata = R"({
    "id_esp": ")" + espId +
                      R"(",
    "temperature": )" +
                      t + R"(,
    "humidity": )" + h +
                      R"(,
    "mq135_co": )" + MQ135_CO +
                      R"(,
    "mq135_alcohol": )" +
                      MQ135_Alcohol + R"(,
    "mq135_co2": )" +
                      (MQ135_CO2 + 400) + R"(,
    "mq135_toluen": )" +
                      MQ135_Toluen + R"(,
    "mq135_nh4": )" +
                      MQ135_NH4 + R"(,
    "mq135_aceton": )" +
                      MQ135_Aceton + R"(,
    "mq2_h2": )" + MQ2_H2 +
                      R"(,
    "mq2_lpg": )" + MQ2_LPG +
                      R"(,
    "mq2_co": )" + MQ2_CO +
                      R"(,
    "mq2_alcohol": )" +
                      MQ2_Alcohol + R"(,
    "mq2_propane": )" +
                      MQ2_Propane + R"(,
    "mq7_h2": )" + MQ7_H2 +
                      R"(,
    "mq7_lpg": )" + MQ7_LPG +
                      R"(,
    "mq7_ch4": )" + MQ7_CH4 +
                      R"(,
    "mq7_co": )" + MQ7_CO +
                      R"(,
    "mq7_alcohol": )" +
                      MQ7_Alcohol + R"(,
    "pm10": )" + PM10 +
                      R"(,
    "pm25": )" + PM25 +
                      R"(
})";

    // Serial.print(jsondata);

    int httpCode = http.POST(jsondata);
    String payload = http.getString();
    if (httpCode > 0)
    {
      // Serial.println(httpCode);
      // Serial.println(payload);
    }
    else
    {
      Serial.println("Error on HTTP request");
    }
    http.end();
    previousMillis = currentMillis;
  }
  delay(2000); // Sampling frequency
}
