#include <WiFi.h>
#include <PubSubClient.h>

// --- KONFIGURASI WIFI & MQTT ---
const char* ssid = "Cloud#9-Lab";
const char* password = "l4bhcmlt9";

const char* mqtt_server = "103.106.72.181";
const int mqtt_port = 1883;
const char* mqtt_user = "MEDLOC"; 
const char* mqtt_password = "MEDLOC";

// --- DEFINISI TOPIK (DIPISAH) ---
const char* topic_ph = "jar_test/sensor/ph";
const char* topic_ntu = "jar_test/sensor/ntu";
const char* topic_tds = "jar_test/sensor/tds";

// --- PIN SENSOR ---
const int PH_PIN = 35;
const int TURBIDITY_PIN = 34;
const int TDS_PIN = 32;

// --- KONSTANTA KALIBRASI ---
const float ADC_RESOLUTION = 4095.0;
const float REF_VOLTAGE = 2.5; 
const float VOLTAGE_PH7 = 2.50;
const float VOLTAGE_PH4 = 2.16;

// --- VARIABEL GLOBAL ---
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const long interval = 10000; // Kirim data setiap 10 detik

// --- SETUP WIFI ---
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// --- FUNGSI BACA & KIRIM SENSOR ---
void readAndPublishSensorData() {
  Serial.println("--- Reading Sensors ---");
  
  // 1. pH Sensor
  int rawValuePH = analogRead(PH_PIN);
  float voltagePH = (rawValuePH / ADC_RESOLUTION) * REF_VOLTAGE;
  float PH_step = (VOLTAGE_PH7 - VOLTAGE_PH4) / (7.0 - 4.0);
  float pH = 7.0 + ((VOLTAGE_PH7 - voltagePH) / PH_step);
  if (pH < 0) pH = 0; if (pH > 14) pH = 14;
  
  // 2. Turbidity Sensor
  int sensorValueTurbidity = analogRead(TURBIDITY_PIN);
  float voltageTurbidity = sensorValueTurbidity * (3.3 / 4096.0);
  float turbidity_NTU = 0;
  if (voltageTurbidity < 1.46) {
    turbidity_NTU = (1.46 - voltageTurbidity) * 1000;
  }
  if (turbidity_NTU < 0) turbidity_NTU = 0; 
  if (turbidity_NTU > 4000) turbidity_NTU = 4000;
  
  // 3. TDS Sensor
  int sensorValueTDS = analogRead(TDS_PIN);
  float voltageTDS = sensorValueTDS * (3.3 / 4096.0);
  float calibrationFactor = 0.5; 
  float tdsValue = (133.42 * voltageTDS * voltageTDS * voltageTDS - 255.86 * voltageTDS * voltageTDS + 857.39 * voltageTDS) * calibrationFactor;
  if (tdsValue < 0) tdsValue = 0; if (tdsValue > 2000) tdsValue = 2000;

  // --- PUBLISH KE MASING-MASING TOPIK ---

  // Kirim pH
  char phString[8];
  dtostrf(pH, 1, 2, phString);
  client.publish(topic_ph, phString);
  Serial.print("Published pH: "); Serial.println(phString);

  // Kirim NTU
  char ntuString[8];
  dtostrf(turbidity_NTU, 1, 0, ntuString);
  client.publish(topic_ntu, ntuString);
  Serial.print("Published NTU: "); Serial.println(ntuString);

  // Kirim TDS
  char tdsString[8];
  dtostrf(tdsValue, 1, 0, tdsString);
  client.publish(topic_tds, tdsString);
  Serial.print("Published TDS: "); Serial.println(tdsString);
}

// --- RECONNECT MQTT ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32-SENSORS-" + String(random(0xffff), HEX);
    
    // Connect saja, tidak perlu subscribe karena tidak menerima perintah
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

// --- SETUP UTAMA ---
void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  // Callback dihapus karena tidak ada kontrol motor
}

// --- LOOP UTAMA ---
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Timer Non-Blocking untuk kirim data
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;
    readAndPublishSensorData();
  }
}