int pin = D5;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 1000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
 
void setup() {
  Serial.begin(9600);
  pinMode(pin,INPUT);
  delay(2000);
  starttime = millis();//get the current time;
}
 
void loop() {
  duration = pulseIn(pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;
 
  if ((millis()-starttime) > sampletime_ms)
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    lowpulseoccupancy = 0;
  Serial.println("C: "+String(concentration)); 
 
 
 
 if (concentration < 1000) {
 
  Serial.print("Clean"); 
  }
 
  if (concentration > 1000 && concentration < 10000) {
 
  Serial.print("Good"); 
    }
 
     if (concentration > 10000 && concentration < 20000) {
 
  Serial.print("Acceptable"); 
    }
 
    if (concentration > 20000 && concentration < 50000) {
  Serial.print("Heavy"); 
  }
 
 if (concentration > 50000 ) {
 
  Serial.print("Hazard"); 
    } 
 
    starttime = millis();
  }
}