#include "MQ135.h"
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>

int air_quality;

const char* ssid     = "UltraViolet";           //Replace with your network name
const char* password = "password";  //Replave with your network password

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s);
const long timeoutTime = 2000;
LiquidCrystal_I2C lcd (0x27, 16,2); 

void setup() {

  lcd. init ();
  lcd.backlight ();
  lcd.setCursor (0, 0);
  delay(1000);
  Serial.begin(115200);
  pinMode(12, OUTPUT);       //Buzzer will be output to ESP32  
  pinMode(18, OUTPUT);       //Green Led will be Output to ESP32
  pinMode(19, OUTPUT);       //Red Led will be output to ESP32
  pinMode(34, INPUT);        //Gas sensor will be an input to the ESP32
  digitalWrite(18, HIGH);    
  digitalWrite(19, HIGH);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}


void loop() {

  MQ135 gasSensor = MQ135(34);
  float air_quality = gasSensor.getPPM();
  lcd.clear();
    delay(100);
    lcd.setCursor(0,0);
    delay(100);
    lcd.print(air_quality);
    lcd.setCursor(8,0);
    lcd.print("PPM");
    delay(100);
  if (air_quality >= 2500)
  {
    digitalWrite(12, HIGH);   //Buzzer is ON
    digitalWrite(19, LOW);    //RED LED is ON  
    digitalWrite(18, HIGH);   //GREEN LED is OFF
    lcd.setCursor(0,1);
    lcd.print("ALERT!TOXIC AIR!");
    delay(2000);
    digitalWrite(12, LOW);
  }
  if (air_quality >700 && air_quality <2500) 
  {
    digitalWrite(12, LOW);     //Buzzer is OFF
    digitalWrite(19, LOW);     //RED LED is ON
    digitalWrite(18, HIGH);    //GREEN LED is OFF
    lcd.setCursor(0,1);
    lcd.print("POOR AIR!");
    delay(2000);
    digitalWrite(12, LOW);
  }
  if (air_quality < 700)
  {
    digitalWrite(12, LOW);     //Buzzer is OFF
    digitalWrite(18, LOW);     //GREEN LED is ON
    digitalWrite(19, HIGH);    //RED LED id OFF
    lcd.setCursor(0,1);
    lcd.print("FRESH AIR!");
    delay(2000);
    
  }
  Serial.print(air_quality);      
  Serial.println("PPM");
  delay(1000);

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the table
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial;}");
            client.println("table { border-collapse: collapse; width:35%; margin-left:auto; margin-right:auto; }");
            client.println("th { padding: 12px; background-color: #0043af; color: white; }");
            client.println("tr { border: 1px solid #ddd; padding: 12px; }");
            client.println("tr:hover { background-color: #bcbcbc; }");
            client.println("td { border: none; padding: 12px; }");
            client.println(".sensor { color:black; font-weight: bold; padding: 1px; }");

            // Web Page Heading

             client.println("</style></head><body><h2>AIR POLLUTION MONITOR</h2>");
            if (air_quality < 700)
            {
              client.println("</style></head><body><h2>FRESH AIR!</h2>");
            }
            else if (air_quality >700 && air_quality <2500) 
            {
              client.println("</style></head><body><h2>POOR AIR :(</h2>");
            }
      else if (air_quality >= 2500)
      {
        client.println("</style></head><body><h2>ALERT!TOXIC AIR!ALERT</h2>");
      }
      
            client.println("<table><tr><th>MEASUREMENT</th><th>VALUE</th></tr>");
            client.println("<tr><td>Pollution PPM</td><td><span class=\"sensor\">");
            client.println(air_quality);
            client.println("</span></td></tr></table>");
            

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

}