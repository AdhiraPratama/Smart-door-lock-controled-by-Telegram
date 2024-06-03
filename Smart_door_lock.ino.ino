#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "Dira";
const char* password = "12345678";

#define BOTtoken "7104296249:AAFj2g3zU3EojapoGAmlJMKwzyL4V96K4VM"  // Bot Token dari BotFather

const char* allowedChatIDs[] = {"6846584541"}; 

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000;  // Mengubah delay menjadi 1000ms (1 detik) untuk menghindari spamming
unsigned long lastTimeBotRan;

const int selenoid = 27;
bool StatusSelenoid = LOW;

const int buzzerPin = 22; // Pin buzzer
const int buzzerDuration = 2000; 
unsigned long buzzerStartTime = 0;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    bool authorized = false;

    // Check if the sender's chat ID is in the allowedChatIDs array
    for (int j = 0; j < sizeof(allowedChatIDs) / sizeof(allowedChatIDs[0]); j++) {
      if (chat_id == allowedChatIDs[j]) {
        authorized = true;
        break;
      }
    }

    if (!authorized) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      // Nyalakan buzzer jika pengguna tidak diotorisasi
      digitalWrite(buzzerPin, HIGH);
      buzzerStartTime = millis();
      continue;
    }
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String control = "Selamat Datang, " + from_name + ".\n";
      control += "Gunakan Commands Di Bawah Untuk Monitoring dan Kontrol Selenoid\n\n";
      control += "/open Untuk Membuka Selenoid \n";
      control += "/close Untuk Menutup Selenoid \n";
      control += "/StatusSelenoid Untuk Cek Status Selenoid \n";

      bot.sendMessage(chat_id, control, "");
    }

    if (text == "/open") {
      bot.sendMessage(chat_id, "Membuka Selenoid", "");
      StatusSelenoid = HIGH;
      Serial.println("Membuka Selenoid - StatusSelenoid = HIGH");
      digitalWrite(selenoid, StatusSelenoid);
    }
    if (text == "/close") {
      bot.sendMessage(chat_id, "Menutup Selenoid", "");
      StatusSelenoid = LOW;
      Serial.println("Menutup Selenoid - StatusSelenoid = LOW");
      digitalWrite(selenoid, StatusSelenoid);
    }

    if (text == "/StatusSelenoid") {
      int readsel = digitalRead(selenoid);
      if (readsel == LOW) {
        bot.sendMessage(chat_id, "Selenoid dalam Kondisi Tertutup", "");
        Serial.println("Selenoid dalam Kondisi Tertutup");
      } else if (readsel == HIGH) {
        bot.sendMessage(chat_id, "Selenoid dalam Kondisi Terbuka", "");
        Serial.println("Selenoid dalam Kondisi Terbuka");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(selenoid, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW); // Pastikan buzzer dimatikan pada awalnya

  digitalWrite(selenoid, StatusSelenoid); // Inisialisasi status selenoid

  Serial.println(selenoid);

  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
#ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
#endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  delay(1000);
}

void loop() {
  if (millis() - lastTimeBotRan > botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    // Memeriksa apakah buzzer harus dimatikan setelah waktu tertentu
    if (digitalRead(buzzerPin) == HIGH && millis() - buzzerStartTime >= buzzerDuration) {
      digitalWrite(buzzerPin, LOW);
    }

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
