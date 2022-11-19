#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> 
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "time.h"

// Replace with your network credentials
const char* ssid = "UltraViolet";
const char* password = "password";

// Initialize Telegram BOT
#define BOTtoken "5651161362:AAFyD4reEoT5M6uU3EaZv3iolI5QhmgWQXE"  // your Bot Token (Get from Botfather)
// The user's chat id
#define CHAT_ID "1418009365"


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

// Flags
bool FlagU = false; //Flag for user
bool FlagP = false; //Flag for password
bool Logged_in = false; // Not used yet

// Variables used in Arduino
String user;
String PassPIN;

// Variables used for sending to GoogleScript
String namelogin;
String pinlogin;

String GOOGLE_SCRIPT_ID = "AKfycbwBQ6m05OOnbmjon9TnMmoQqRV2JRycqN2QFh7BoBuz5xwPhCuAH6ulOHHK1cjx6VY1";    // change Gscript ID

// Functions 

// verification of username and password
void verification(String Name,String Pass){

  Serial.println("Verification : " + Name + Pass);

  String url_write = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?namelogin="+String(user)+"&pinlogin="+String(PassPIN);

  HTTPClient http;
  http.begin(url_write.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int conf = http.GET();
  String result = String(http.getString());

  Serial.println(result);
  
  if(result == "0"){
    Serial.println("The password MATCHED!");
  }
  if(result == "1"){
    Serial.println("The password DOESN'T match!");
  }
  if(result == "2"){
    Serial.println("User doesn't exist");
  }
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
  
  // Start Function: Prompts the user to various available functions
    if (text == "/start") {
    
      String msg = "Welcome, " + from_name + ".\n";
      msg += "Use the following commands to control your outputs.\n\n";
      
      // String keyboardJson = "[[\"/login\", \"/Cash_Withdrawal\"],[\"/PIN_Change\",\"/Fund_Transfer\"],[\"/Mini_Statement\",\"/Balance_Inquiry\"]]";
      String keyboardJson = "[[\"/Login\", \"/New_User\"],[\"/Exit\"],]";
      bot.sendMessageWithReplyKeyboard(chat_id, msg, "", keyboardJson, true);

    }

    if (text == "/Login"){
      String keyboardJson = "[[\"/Exit\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Enter username", "", keyboardJson, true);
      FlagU = true;
      return;
    }

    if (text == "/Enter_PIN"){
      String keyboardJson = "[[\"/Exit\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Enter the numeric PIN", "", keyboardJson, true);
      FlagP = true;
      return;
    }

    if (FlagU == true){
      user = text;
      FlagU = false;
      Serial.println("user : " + user);
      String keyboardJson = "[[\"/Enter_PIN\"],[\"/Exit\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Choose from one of the following options", "", keyboardJson, true);


    }

    if (FlagP == true){
      PassPIN = text;
      FlagP = false;
      Serial.println("PIN : " + PassPIN);
      verification(user,PassPIN);
    }
    
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  Serial.println(ssid);
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}