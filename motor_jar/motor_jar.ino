#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// ================= KONFIGURASI WIFI & MQTT =================
const char* ssid = "Cloud#9-Lab";
const char* password = "l4bhcmlt9";

const char* mqtt_server = "103.106.72.181";
const int mqtt_port = 1883;
const char* mqtt_user = "MEDLOC"; 
const char* mqtt_password = "MEDLOC";

// ================= KONFIGURASI SENSOR DHT =================
#define DHTPIN 15       // Pin Data DHT terhubung ke D15
#define DHTTYPE DHT22   // Menggunakan DHT22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);

// Topik MQTT untuk Sensor
const char* temp_topic = "PDAM/jartesting/temperature";
const char* humidity_topic = "PDAM/jartesting/humidity";

// Timer untuk pengiriman data DHT (Non-blocking)
long lastMsg = 0;
const int interval = 30000; // Kirim data sensor setiap 30 detik

// Variabel suhu/lembab terakhir agar tidak spam data yang sama
float lastTemp = -999.0;
float lastHum = -999.0;

// ================= KONFIGURASI MOTOR (L298N) =================
// Motor 1
#define M1_ENA 21
#define M1_IN1 19
#define M1_IN2 18
// Motor 2
#define M2_IN3 5
#define M2_IN4 4
#define M2_ENB 2
// Motor 3
#define M3_ENA 26
#define M3_IN1 25
#define M3_IN2 33
// Motor 4
#define M4_IN3 32
#define M4_IN4 27 
#define M4_ENB 14 
// Motor 5 (BARU)
#define M5_ENA 23
#define M5_IN1 22
#define M5_IN2 13
// Motor 6 (BARU)
#define M6_IN3 12
#define M6_IN4 16
#define M6_ENB 17

// Konfigurasi PWM
const int freq = 1000;
const int resolution = 8; // 0-255

// Variabel Speed Motor
int speed1 = 0, speed2 = 0, speed3 = 0, speed4 = 0, speed5 = 0, speed6 = 0;

// ================= OBJEK CLIENT =================
WiFiClient espClient;
PubSubClient client(espClient);

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // 1. Setup Pin Motor (Output Arah)
  pinMode(M1_IN1, OUTPUT); pinMode(M1_IN2, OUTPUT);
  pinMode(M2_IN3, OUTPUT); pinMode(M2_IN4, OUTPUT);
  pinMode(M3_IN1, OUTPUT); pinMode(M3_IN2, OUTPUT);
  pinMode(M4_IN3, OUTPUT); pinMode(M4_IN4, OUTPUT);
  // Setup Pin Motor 5 & 6
  pinMode(M5_IN1, OUTPUT); pinMode(M5_IN2, OUTPUT);
  pinMode(M6_IN3, OUTPUT); pinMode(M6_IN4, OUTPUT);

  // 2. Setup PWM Motor
  // Menggunakan API ESP32 Core v3.0+. Jika error, lihat komentar di bawah.
  ledcAttach(M1_ENA, freq, resolution);
  ledcAttach(M2_ENB, freq, resolution);
  ledcAttach(M3_ENA, freq, resolution);
  ledcAttach(M4_ENB, freq, resolution);
  ledcAttach(M5_ENA, freq, resolution);
  ledcAttach(M6_ENB, freq, resolution);
  
  /* CATATAN: Jika menggunakan ESP32 Core v2.x (lama) dan error 'ledcAttach':
     Gunakan kode ini:
     ledcSetup(0, freq, resolution); ledcAttachPin(M1_ENA, 0);
     ledcSetup(1, freq, resolution); ledcAttachPin(M2_ENB, 1);
     ledcSetup(2, freq, resolution); ledcAttachPin(M3_ENA, 2);
     ledcSetup(3, freq, resolution); ledcAttachPin(M4_ENB, 3);
     ledcSetup(4, freq, resolution); ledcAttachPin(M5_ENA, 4);
     ledcSetup(5, freq, resolution); ledcAttachPin(M6_ENB, 5);
  */

  // 3. Setup Sensor DHT
  dht.begin();

  // 4. Setup Koneksi
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // Fungsi callback wajib untuk menerima perintah motor
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke "); Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi terhubung");
  Serial.println(WiFi.localIP());
}

// ================= LOGIKA MOTOR (CALLBACK MQTT) =================
// Fungsi ini dipanggil otomatis saat ada pesan masuk dari MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  // Debugging pesan masuk
  // Serial.print("Pesan masuk ["); Serial.print(topic); Serial.print("]: "); Serial.println(message);

  String topicStr = String(topic);
  int motorId = 0;

  // Cek topik untuk menentukan motor mana yang dikontrol
  if (topicStr.indexOf("mixer1") >= 0) motorId = 1;
  else if (topicStr.indexOf("mixer2") >= 0) motorId = 2;
  else if (topicStr.indexOf("mixer3") >= 0) motorId = 3;
  else if (topicStr.indexOf("mixer4") >= 0) motorId = 4;
  else if (topicStr.indexOf("mixer5") >= 0) motorId = 5;
  else if (topicStr.indexOf("mixer6") >= 0) motorId = 6;

  if (motorId > 0) {
    int currentSpeed = 0;
    if (motorId == 1) currentSpeed = speed1;
    if (motorId == 2) currentSpeed = speed2;
    if (motorId == 3) currentSpeed = speed3;
    if (motorId == 4) currentSpeed = speed4;
    if (motorId == 5) currentSpeed = speed5;
    if (motorId == 6) currentSpeed = speed6;

    // Jika perintah ON/OFF
    if (topicStr.endsWith("cmd")) {
      if (message == "ON") {
         // Jika speed sebelumnya 0, set ke full (255), jika tidak gunakan speed terakhir
         int runSpeed = (currentSpeed > 0) ? currentSpeed : 255;
         controlMotor(motorId, runSpeed);
         updateSpeedVar(motorId, runSpeed);
      } 
      else if (message == "OFF") {
         controlMotor(motorId, 0);
      }
    }
    // Jika perintah Set Speed (angka 0-255)
    else if (topicStr.endsWith("speed")) {
      int newSpeed = message.toInt();
      controlMotor(motorId, newSpeed);
      updateSpeedVar(motorId, newSpeed);
    }
  }
}

// Fungsi fisik menggerakkan motor
void controlMotor(int motorId, int speed) {
  int pin1, pin2, pinPWM;
  
  // Mapping Pin
  switch (motorId) {
    case 1: pin1 = M1_IN1; pin2 = M1_IN2; pinPWM = M1_ENA; break;
    case 2: pin1 = M2_IN3; pin2 = M2_IN4; pinPWM = M2_ENB; break;
    case 3: pin1 = M3_IN1; pin2 = M3_IN2; pinPWM = M3_ENA; break;
    case 4: pin1 = M4_IN3; pin2 = M4_IN4; pinPWM = M4_ENB; break;
    case 5: pin1 = M5_IN1; pin2 = M5_IN2; pinPWM = M5_ENA; break;
    case 6: pin1 = M6_IN3; pin2 = M6_IN4; pinPWM = M6_ENB; break;
    default: return;
  }

  // Tulis PWM
  // Jika Core v2.x: ganti ledcWrite(pinPWM, speed) dengan ledcWrite(channel, speed)
  ledcWrite(pinPWM, speed); 

  // Atur Arah (Default CW - Searah Jarum Jam)
  if (speed == 0) {
    digitalWrite(pin1, LOW); digitalWrite(pin2, LOW);
  } else {
    digitalWrite(pin1, HIGH); digitalWrite(pin2, LOW);
  }
}

void updateSpeedVar(int id, int val) {
  if (id == 1) speed1 = val;
  if (id == 2) speed2 = val;
  if (id == 3) speed3 = val;
  if (id == 4) speed4 = val;
  if (id == 5) speed5 = val;
  if (id == 6) speed6 = val;
}

// ================= KONEKSI ULANG (RECONNECT) =================
void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan MQTT...");
    String clientId = "ESP32Mixer-" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("Terhubung!");
      
      // === SUBSCRIBE TOPIC MOTOR ===
      client.subscribe("mixer1/cmd"); client.subscribe("mixer1/speed");
      client.subscribe("mixer2/cmd"); client.subscribe("mixer2/speed");
      client.subscribe("mixer3/cmd"); client.subscribe("mixer3/speed");
      client.subscribe("mixer4/cmd"); client.subscribe("mixer4/speed");
      client.subscribe("mixer5/cmd"); client.subscribe("mixer5/speed");
      client.subscribe("mixer6/cmd"); client.subscribe("mixer6/speed");
      
    } else {
      Serial.print("Gagal, rc="); Serial.print(client.state());
      Serial.println(" coba lagi 5 detik");
      delay(5000);
    }
  }
}

// ================= MAIN LOOP =================
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Penting untuk memproses pesan MQTT masuk

  // --- LOGIKA SENSOR DHT (Non-Blocking) ---
  long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Cek validitas data sensor
    if (isnan(h) || isnan(t)) {
      Serial.println("Gagal membaca sensor DHT!");
    } else {
      // Kirim Suhu
      String tempStr = String(t, 1); // 1 desimal
      Serial.print("Suhu: "); Serial.print(tempStr); Serial.println(" C");
      client.publish(temp_topic, tempStr.c_str(), true); // retain = true

      // Kirim Kelembapan
      String humStr = String(h, 1);
      Serial.print("Kelembapan: "); Serial.print(humStr); Serial.println(" %");
      client.publish(humidity_topic, humStr.c_str(), true);
    }
  }
}