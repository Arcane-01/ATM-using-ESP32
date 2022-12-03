#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> 
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "time.h"

// Replace with your network credentials
const char* ssid = "UltraViolet";
const char* password = "password"; //1565+qD7

// Initialize Telegram BOT
#define BOTtoken "5651161362:AAFyD4reEoT5M6uU3EaZv3iolI5QhmgWQXE"  // your Bot Token (Get from Botfather)
// The user's chat id
#define CHAT_ID "1418009365"


WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

//Global Variables
String text = "";
String prev_text = "";
// Flags
bool FlagU = false; //Flag for user
bool FlagP = false; //Flag for password
bool Logged_in = false; // Flag to check whether the user is Logged in

// Variables used in Arduino
String user;
String PassPIN;

// Variables used for sending to GoogleScript (Might not be needed)
String namelogin;
String pinlogin;

String GOOGLE_SCRIPT_ID = "AKfycbyc8yjG1WPc8Js_58CN2qHh8FGglYbjSgvWa_NcGNK_SkOHV3_uKBjCwa8ROjY33gCl";    // change Gscript ID

// Functions 

// verification of username and password
int verification(String Name,String Pass){

  Serial.println("Verification started: " + Name +" " + Pass);

  String url_write = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?namelogin="+String(user)+"&pinlogin="+String(PassPIN);

  HTTPClient http;
  http.begin(url_write.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int conf = http.GET();
  String result = String(http.getString());

  Serial.println(result);
  
  if(result == "0"){
    return 0;
    Serial.println("The password MATCHED!");
  }
  if(result == "1"){
    return 1;
    Serial.println("The password DOESN'T match!");
  }
  if(result == "2"){
    return 2;
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
    // if (chat_id != CHAT_ID){
    //   bot.sendMessage(chat_id, "Unauthorized user", "");
    //   continue;
    // }
    
    // Print the received message
    text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;
  
  // Start Function: Prompts the user to various available functions
    if (text == "/start") {
    
      String msg = "Welcome, " + from_name + ".\n";
      msg += "Use the following commands to control your outputs.\n\n";
      
      // String keyboardJson = "[[\"/login\", \"/Cash_Withdrawal\"],[\"/PIN_Change\",\"/Fund_Transfer\"],[\"/Mini_Statement\",\"/Balance_Inquiry\"]]";
      String keyboardJson = "[[\"/Login\", \"/New_User\"],[\"/Exit\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, msg, "", keyboardJson, true);

    }

    else if (text == "/Login"){
      String keyboardJson = "[[\"/Exit\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Enter username", "", keyboardJson, true);
      FlagU = true;
      return;
    }

    else if (text == "/Enter_PIN"){
      String keyboardJson = "[[\"/Exit\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Enter the numeric PIN", "", keyboardJson, true);
      FlagP = true;
      return;
    }
    // Code for Withdrawal 
    else if (text == "/Debit"){

      if(Logged_in == true){
        bot.sendMessage(chat_id, "Enter amount to be debited", "");
      }
      else if (Logged_in == false) {
        bot.sendMessage(chat_id, "The user is not Logged in", "");
      }

    }
    else if (prev_text == "/Debit"){
      String amount = text;
      Serial.println("Withdrawal started : " + amount+ " " +user);

      String url_write = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?withdraw="+ String(amount) +"&userid="+String(user);

      HTTPClient http;
      http.begin(url_write.c_str());
      http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
      int conf = http.GET();
      String result = String(http.getString());
    }
    //Code For Balance Inquiry
    else if (text == "/Balance_Inquiry"){
      Serial.println("Balance Inquiry started : " +user);

      String url_write = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?balancecheckuser="+ String(user);

      HTTPClient http;
      http.begin(url_write.c_str());
      http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
      int conf = http.GET();
      String result = String(http.getString());

      bot.sendMessage(chat_id, "The total balance of the user " + user + " is " + result, "");
    }
    // Code for Credit
    else if (text == "/Credit"){
      if(Logged_in == true){
        bot.sendMessage(chat_id, "Enter amount to be debited", "");
      }
      else if (Logged_in == false) {
        bot.sendMessage(chat_id, "The user is not Logged in", "");
      }
    }
    else if (prev_text == "/Credit"){
      String amount = text;
      Serial.println("Credit started : " + amount +" "+user);

      String url_write = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?credit="+ String(amount) +"&userid="+String(user);

      HTTPClient http;
      http.begin(url_write.c_str());
      http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
      int conf = http.GET();
      String result = String(http.getString());
    }

    else if (prev_text == "/Login"){
      user = text;
      FlagU = false;
      Serial.println("user : " + user);
      String keyboardJson = "[[\"/Enter_PIN\"],[\"/Exit\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Choose from one of the following options", "", keyboardJson, true);


    }

    else if (prev_text == "/Enter_PIN"){
      PassPIN = text;
      FlagP = false;
      Serial.println("PIN : " + PassPIN);
      int v = verification(user,PassPIN);
      if(v==0){
        Logged_in = true;
        String keyboardJson = "[[\"/Debit\", \"/Credit\"],[\"/Balance_Inquiry\",\"/PIN_Change\"],[\"/Exit\"]]";
        bot.sendMessageWithReplyKeyboard(chat_id, "Select the command", "", keyboardJson, true);
      }
      if(v==1){
        String keyboardJson = "[[\"/Re-enter Password\", \"/Forgot Password\"],[\"/Exit\"]";
        bot.sendMessageWithReplyKeyboard(chat_id, "Incorrect Password", "", keyboardJson, true);
      }
      if(v==2){
        bot.sendMessage(chat_id, "Unauthorized user", "");
      }
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
  prev_text = bot.messages[0].text;
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      Serial.println("Text: " + text);
      Serial.println("Previous Text: " + prev_text);
    }
    lastTimeBotRan = millis();
  }

}