// the required libraries are included here
#include "WiFi.h"
#include <Wire.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>

// Declaration of variables for intranet webpage
const char* ssid = "Patrick_Bateman";
const char* password = "americanpsycho";
WiFiServer server(80);

// Variable Declaration for ultasonic sensor readings
const int trigPin = 5;
const int echoPin = 18;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

float trial = 50;
// Setup for LCD
LiquidCrystal_I2C lcd (0x27, 16,2); 

String html;
String message;

void setup() {

  // setup code corresponding to intranet webpage
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  // Print the IP address used for accessing the webpage
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.print("ssid: ");
  Serial.println(ssid);
  server.begin();


  // setup code for ultrasonic sensor
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input


  // setup code for buzzer and LEDs
  pinMode(12, OUTPUT);       //Buzzer will be output to ESP32  
  pinMode(14, OUTPUT);       //Green Led will be Output to ESP32
  pinMode(19, OUTPUT);       //Red Led will be output to ESP32
  digitalWrite(18, HIGH);    // the Green LED is turned off intially
  digitalWrite(19, HIGH);    // the Red LED is turned off initially

  // setup code for LED 
  // lcd.init ();
  lcd.backlight ();
  lcd.setCursor (0, 0);
  delay(1000);  

}

void loop() {
  // code for getting the distance using a ultrasonic sensor

  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);
  
  delay(1000);

  // Code for buzzer and LED glowing corresponding to the distance obtained from the Ultrasonic Sensor

  if (distanceCm <= 20)
  {
    digitalWrite(12, HIGH);   //Buzzer is ON
    digitalWrite(19, LOW);    //RED LED is ON  
    digitalWrite(18, HIGH);   //GREEN LED is OFF
    lcd.setCursor(0,1);
    lcd.print("ALERT!");
    delay(2000);
    message = "ALERT";
    digitalWrite(12, LOW);
  }
  if (distanceCm >20 && distanceCm <50) 
  {
    digitalWrite(12, LOW);     //Buzzer is OFF
    digitalWrite(19, LOW);     //RED LED is ON
    digitalWrite(18, HIGH);    //GREEN LED is OFF
    lcd.setCursor(0,1);
    lcd.print("CLOSE!");
    message = "CLOSE!";
    delay(2000);
    digitalWrite(12, LOW);
  }
  if (distanceCm >= 50)
  {
    digitalWrite(12, LOW);     //Buzzer is OFF
    digitalWrite(18, LOW);     //GREEN LED is ON
    digitalWrite(19, HIGH);    //RED LED id OFF
    lcd.setCursor(0,1);
    lcd.print("SAFE!");
    message = "SAFE!";
    delay(2000);
    
  }

  String html ="<!DOCTYPE html>\
  <html>\
  <body>\
  <form>\
    <p>" +String(message)+ " Distance(in cm): " +String(distanceCm)+ "</p>\
  </form>\
  </body>\
  </html>";

  WiFiClient client = server.available();
  if(client)
  {
    String request = client.readStringUntil('\r');
    if(request.indexOf("LED=ON")>=0)digitalWrite(2,HIGH);
    if(request.indexOf("LED=OFF")>=0)digitalWrite(2,LOW);
    client.print(html);
    request="";
  }

}
