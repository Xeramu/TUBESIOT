
// // ver fix
// // COBA FINAL - FULL SEPARATED BACKEND ONLY
// #include <Arduino.h>
// #include <Wire.h>
// #include <LiquidCrystal_I2C.h>
// #include <math.h>
// #include <WiFi.h>
// #include <WebServer.h>
// #include <HTTPClient.h>

// // 1. KONFIGURASI PIN & PERANGKAT HARDWARE
// #define RELAY_PIN         15
// #define FLOW_SENSOR_PIN   27
// #define WATER_SENSOR_PIN  35
// #define THERMISTOR_PIN    34
// #define BUZZER_PIN        32
// #define BUZZER_CHANNEL    0

// #define LED_RED_PIN       19
// #define LED_GREEN_PIN     33
// #define LED_BLUE_PIN      14

// LiquidCrystal_I2C lcd(0x27, 16, 2);
// WebServer server(80);

// // 2. PENGATURAN WI-FI & THINGSPEAK
// const char* ssid = "Admin baru 1";
// const char* password = "tanyabunda";

// const char* serverName = "http://api.thingspeak.com/update";
// String writeAPIKey = "7JQGAIZOBZGRIXWH";

// // 3. VARIABEL GLOBAL & PENGATURAN SENSOR
// unsigned long prevMillis1s = 0;             
// unsigned long prevThingSpeakMillis = 0;     
// const long thingSpeakInterval = 15000;

// volatile unsigned long pulseCount = 0;
// unsigned long noFlowStart = 0;
// bool isPumpOff = false;
// float flowRate = 0.0;
// float lastFlowRate = 0.0;
// float deltaFlow = 0.0;

// enum KelasKondisi { NORMAL_PUMP = 0, MACET = 1, ANOMALI_BOCOR = 2 };

// unsigned long prevMillisWaterSample = 0;
// const long waterSampleInterval = 2;
// long waterSampleSum = 0;
// int waterSampleCount = 0;
// int currentWaterLevelValue = 0;

// #define NUM_SAMPLES         10

// String currentTempStatus = "NORMAL";
// float globalTempC = 25.0; 

// // 4. INTERRUPT SERVICE ROUTINE (ISR)
// void IRAM_ATTR pulseCounter() {
//   pulseCount++;
// }

// // 5. ENGINE MACHINE LEARNING: DECISION TREE
// KelasKondisi prediksiDecisionTree(float flow, float delta) {
//   if (flow >= 0.7) {
//     return NORMAL_PUMP;
//   }
//   else {
//     if (delta <= -0.5) {
//       return MACET;
//     }
//     else {
//       if (flow <= 0.5) {
//         return MACET;
//       }
//       else {
//         return ANOMALI_BOCOR;
//       }
//     }
//   }
// }

// // 6. FUNGSI KONTROL INDIKATOR (RGB & BUZZER)
// void setRGB(bool red, bool green, bool blue) {
//   digitalWrite(LED_RED_PIN, red ? HIGH : LOW);
//   digitalWrite(LED_GREEN_PIN, green ? HIGH : LOW);
//   digitalWrite(LED_BLUE_PIN, blue ? HIGH : LOW);
// }

// void checkSystemSafety(int waterValue, String tempStatus) {
//   if (waterValue <= 500) {
//     Serial.println("ALARM: LOW WATER!");
//     ledcWriteTone(BUZZER_CHANNEL, 2000);
//     setRGB(true, false, false);
//   }
//   else if (tempStatus == "HOT") {
//     Serial.println("ALARM: WATER TOO HOT!");
//     ledcWriteTone(BUZZER_CHANNEL, 1500);
//     setRGB(false, false, false);
//   }
//   else if (waterValue < 3000) {
//     Serial.println("ALARM: WATER WARNING");
//     ledcWriteTone(BUZZER_CHANNEL, 1000);
//     setRGB(true, false, false);
//   }
//   else {
//     ledcWriteTone(BUZZER_CHANNEL, 0);
//     digitalWrite(BUZZER_PIN, LOW);
//     setRGB(false, false, false);
//   }
// }

// // 7. FUNGSI DRIVER SENSOR (NON-BLOCKING)
// void updateWaterLevelAsync() {
//   unsigned long currentMillis = millis();
//   if (currentMillis - prevMillisWaterSample >= waterSampleInterval) {
//     prevMillisWaterSample = currentMillis;
//     waterSampleSum += analogRead(WATER_SENSOR_PIN);
//     waterSampleCount++;

//     if (waterSampleCount >= NUM_SAMPLES) {  
//       currentWaterLevelValue = waterSampleSum / NUM_SAMPLES;   
//       waterSampleSum = 0;  
//       waterSampleCount = 0;  
//     }
//   }
// }

// float readTemperature() {
//   long sum = 0;
//   for (int i = 0; i < NUM_SAMPLES; i++) {
//     sum += analogRead(THERMISTOR_PIN);
//     delay(5);  
//   }

//   float average = sum / (float)NUM_SAMPLES;

//   if (average < 1.0) {
//     return -777; 
//   }

//   float tempC = 23.0 + ((average - 30.0) * (45.0 - 23.0) / (112.0 - 30.0));

//   if (tempC > 100.0) tempC = 100.0;
//   if (tempC < 0.0)   tempC = 0.0;

//   return tempC;
// }

// // 8. DATA RESPONDER HANDLER (MURNI JSON API & PENGAMAN CORS)
// void handleAPIStatus() {
//   KelasKondisi hasilML = prediksiDecisionTree(flowRate, deltaFlow);
  
//   // Membentuk format standard data JSON untuk dikonsumsi JavaScript index.html
//   String jsonResponse = "{";
//   jsonResponse += "\"flowRate\":" + String(flowRate, 2) + ",";
//   jsonResponse += "\"deltaFlow\":" + String(deltaFlow, 2) + ",";
//   jsonResponse += "\"isPumpOff\":" + String(isPumpOff ? "true" : "false") + ",";
//   jsonResponse += "\"prediksiML\":" + String((int)hasilML);
//   jsonResponse += "}";

//   // Pengiriman header CORS agar Browser tidak melakukan blokir keamanan cross-origin
//   server.sendHeader("Access-Control-Allow-Origin", "*");
//   server.send(200, "application/json", jsonResponse);
// }

// void handleNyalakan() {
//   noFlowStart = 0;
//   isPumpOff = false;
//   pinMode(RELAY_PIN, OUTPUT);
//   digitalWrite(RELAY_PIN, HIGH);
//   Serial.println("[API] Perintah REST API: Pompa Dinyalakan.");
  
//   server.sendHeader("Access-Control-Allow-Origin", "*");
//   server.send(200, "text/plain", "READY");
// }

// void handleMatikan() {
//   digitalWrite(RELAY_PIN, LOW);
//   pinMode(RELAY_PIN, INPUT);
//   isPumpOff = true;
//   Serial.println("[API] Perintah REST API: Pompa Dimatikan.");
  
//   server.sendHeader("Access-Control-Allow-Origin", "*");
//   server.send(200, "text/plain", "SHUTDOWN");
// }

// // 9. SETUP
// void setup() {
//   Serial.begin(115200);
//   delay(1000);
//   Serial.println("\n\n=== SYSTEM STARTING ===\n");

//   analogSetAttenuation(ADC_11db);  
//   analogSetWidth(12);              

//   pinMode(BUZZER_PIN, OUTPUT);
//   pinMode(LED_RED_PIN, OUTPUT);
//   pinMode(LED_GREEN_PIN, OUTPUT);
//   pinMode(LED_BLUE_PIN, OUTPUT);
//   setRGB(false, false, false);

//   ledcSetup(BUZZER_CHANNEL, 2000, 8);
//   ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

//   lcd.init();
//   lcd.backlight();
//   lcd.setCursor(0, 0); 
//   lcd.print("SYSTEM INIT");
//   lcd.setCursor(0, 1); 
//   lcd.print("WAIT CONNECT...");

//   pinMode(RELAY_PIN, OUTPUT);
//   pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
//   pinMode(WATER_SENSOR_PIN, INPUT);    
//   pinMode(THERMISTOR_PIN, INPUT);      

//   attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING);

//   digitalWrite(RELAY_PIN, LOW);

//   WiFi.disconnect(true);
//   delay(500);
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(ssid, password);
//   WiFi.setSleep(false);  

//   int attempt = 0;
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//     attempt++;
//     if (attempt > 20) {
//       WiFi.disconnect();
//       WiFi.begin(ssid, password);
//       attempt = 0;
//     }
//   }

//   Serial.println("\n[WiFi] WiFi CONNECTED!");
  
//   lcd.clear();
//   lcd.print("WiFi Connected!");
//   lcd.setCursor(0, 1);
//   lcd.print(WiFi.localIP());
//   delay(1500);

//   pinMode(RELAY_PIN, OUTPUT);
//   digitalWrite(RELAY_PIN, HIGH);
//   isPumpOff = false;

//   // Endpoint API Server baru
//   server.on("/api/status", handleAPIStatus);
//   server.on("/action_nyalakan", handleNyalakan);
//   server.on("/action_matikan", handleMatikan);
//   server.begin();
// }

// // 10. LOOP
// void loop() {
//   server.handleClient();         
//   updateWaterLevelAsync();       

//   static int lastCheckedWater = 0;
//   if (abs(currentWaterLevelValue - lastCheckedWater) > 100) {
//     checkSystemSafety(currentWaterLevelValue, currentTempStatus);
//     lastCheckedWater = currentWaterLevelValue;
//   }

//   unsigned long currentMillis = millis();

//   if (currentMillis - prevMillis1s >= 1000) {
//     prevMillis1s = currentMillis;

//     noInterrupts();   
//     unsigned long localPulseCount = pulseCount;  
//     pulseCount = 0;   
//     interrupts();     

//     flowRate = (float)localPulseCount / 7.5;  
//     deltaFlow = flowRate - lastFlowRate;   
//     lastFlowRate = flowRate;   

//     float tempC = readTemperature();  
    
//     if (tempC == -777) {
//       Serial.println("[SAFETY] Hentakan Daya Terdeteksi, Menggunakan Data Suhu Terakhir.");
//     } 
//     else if (tempC == -999) {
//       currentTempStatus = "ERROR";
//     } 
//     else {
//       globalTempC = tempC; 

//       if (tempC < 20.0) {
//         currentTempStatus = "COLD";     
//       }
//       else if (tempC <= 26.0) {
//         currentTempStatus = "NORMAL";   
//       }
//       else {
//         currentTempStatus = "HOT";      
//       }
//     }

//     checkSystemSafety(currentWaterLevelValue, currentTempStatus);  

//     KelasKondisi statusML = prediksiDecisionTree(flowRate, deltaFlow);  

//     if (statusML == MACET) {   
//       if (noFlowStart == 0) {  
//         noFlowStart = currentMillis;  
//       }  
//       if ((currentMillis - noFlowStart >= 5000) && !isPumpOff) {  
//         digitalWrite(RELAY_PIN, LOW);   
//         pinMode(RELAY_PIN, INPUT);
//         isPumpOff = true;  
//         Serial.println(" -> [ML] STATUS: POMPA MACET! SAFETY SHUTDOWN.");  
//       }  
//     }   
//     else if (statusML == ANOMALI_BOCOR) {  
//       Serial.println(" -> [ML] STATUS: ADA ANOMALI / INDIKASI BOCOR ALUS");  
//       noFlowStart = 0;   
//     }   
//     else {  
//       if (!isPumpOff) {  
//         noFlowStart = 0;  
//         Serial.println(" -> [ML] STATUS: NORMAL");  
//       }  
//     }   

//     // --- Cetak Log Serial ---  
//     Serial.print("Debit: "); Serial.print(flowRate, 2);  
//     Serial.print(" L/min | Delta: "); Serial.println(deltaFlow, 2);  
//     Serial.print("Water ADC: "); Serial.println(currentWaterLevelValue);  
//     Serial.print("Temp: "); Serial.print(globalTempC, 1); Serial.print(" C | Status Temp: "); Serial.println(currentTempStatus);  
//     Serial.println("---------------------------------");  

//     // --- Update Tampilan LCD ---  
//     lcd.clear();  
//     lcd.setCursor(0, 0);  
//     lcd.print("T:");
//     lcd.print(globalTempC, 1);
//     lcd.print("C ");
//     lcd.print(currentTempStatus);  
    
//     lcd.setCursor(0, 1);  
//     lcd.print("W:");
//     lcd.print(currentWaterLevelValue);  
//     lcd.print(" F:");
//     lcd.print(flowRate, 1);
//   }

//   if (currentMillis - prevThingSpeakMillis >= thingSpeakInterval) {
//     prevThingSpeakMillis = currentMillis;

//     if (WiFi.status() == WL_CONNECTED) {  
//       HTTPClient http;  
//       String url = String(serverName) + "?api_key=" + writeAPIKey   
//                    + "&field1=" + String(globalTempC, 1)   
//                    + "&field2=" + String(currentWaterLevelValue)   
//                    + "&field3=" + currentTempStatus;  
//       http.begin(url);  
//       int httpResponseCode = http.GET();  
//       http.end();  
//     }
//   }
// }





// ver fix - WITH AMBULANCE SIREN FOR PUMP ERROR
// COBA FINAL - FULL SEPARATED BACKEND ONLY
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// 1. KONFIGURASI PIN & PERANGKAT HARDWARE
#define RELAY_PIN         15
#define FLOW_SENSOR_PIN   27
#define WATER_SENSOR_PIN  35
#define THERMISTOR_PIN    34
#define BUZZER_PIN        32
#define BUZZER_CHANNEL    0

#define LED_RED_PIN       19
#define LED_GREEN_PIN     33
#define LED_BLUE_PIN      14

LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer server(80);

// 2. PENGATURAN WI-FI & THINGSPEAK
const char* ssid = "Admin baru 1";
const char* password = "tanyabunda";

const char* serverName = "http://api.thingspeak.com/update";
String writeAPIKey = "7JQGAIZOBZGRIXWH";

// 3. VARIABEL GLOBAL & PENGATURAN SENSOR
unsigned long prevMillis1s = 0;             
unsigned long prevThingSpeakMillis = 0;     
const long thingSpeakInterval = 15000;

volatile unsigned long pulseCount = 0;
unsigned long noFlowStart = 0;
bool isPumpOff = false;
float flowRate = 0.0;
float lastFlowRate = 0.0;
float deltaFlow = 0.0;

enum KelasKondisi { NORMAL_PUMP = 0, MACET = 1, ANOMALI_BOCOR = 2 };

unsigned long prevMillisWaterSample = 0;
const long waterSampleInterval = 2;
long waterSampleSum = 0;
int waterSampleCount = 0;
int currentWaterLevelValue = 0;

#define NUM_SAMPLES         10

String currentTempStatus = "NORMAL";
float globalTempC = 25.0; 

// --- LOGIC TAMBAHAN BUZZER AMBULANS NON-BLOCKING ---
unsigned long prevSirenMillis = 0;
bool sirenToneToggle = false;

// 4. INTERRUPT SERVICE ROUTINE (ISR)
void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

// 5. ENGINE MACHINE LEARNING: DECISION TREE
KelasKondisi prediksiDecisionTree(float flow, float delta) {
  if (flow >= 0.7) {
    return NORMAL_PUMP;
  }
  else {
    if (delta <= -0.5) {
      return MACET;
    }
    else {
      if (flow <= 0.5) {
        return MACET;
      }
      else {
        return ANOMALI_BOCOR;
      }
    }
  }
}

// 6. FUNGSI KONTROL INDIKATOR (RGB & BUZZER)
void setRGB(bool red, bool green, bool blue) {
  digitalWrite(LED_RED_PIN, red ? HIGH : LOW);
  digitalWrite(LED_GREEN_PIN, green ? HIGH : LOW);
  digitalWrite(LED_BLUE_PIN, blue ? HIGH : LOW);
}

void checkSystemSafety(int waterValue, String tempStatus) {
  // Jika pompa sedang mati akibat shutdown, serahkan kontrol buzzer ke sirine ambulans di loop()
  if (isPumpOff) return; 

  if (waterValue <= 500) {
    Serial.println("ALARM: LOW WATER!");
    ledcWriteTone(BUZZER_CHANNEL, 2000);
    setRGB(true, false, false);
  }
  else if (tempStatus == "HOT") {
    Serial.println("ALARM: WATER TOO HOT!");
    ledcWriteTone(BUZZER_CHANNEL, 1500);
    setRGB(false, false, false);
  }
  else if (waterValue < 3000) {
    Serial.println("ALARM: WATER WARNING");
    ledcWriteTone(BUZZER_CHANNEL, 1000);
    setRGB(true, false, false);
  }
  else {
    ledcWriteTone(BUZZER_CHANNEL, 0);
    digitalWrite(BUZZER_PIN, LOW);
    setRGB(false, false, false);
  }
}

// 7. FUNGSI DRIVER SENSOR (NON-BLOCKING)
void updateWaterLevelAsync() {
  unsigned long currentMillis = millis();
  if (currentMillis - prevMillisWaterSample >= waterSampleInterval) {
    prevMillisWaterSample = currentMillis;
    waterSampleSum += analogRead(WATER_SENSOR_PIN);
    waterSampleCount++;

    if (waterSampleCount >= NUM_SAMPLES) {  
      currentWaterLevelValue = waterSampleSum / NUM_SAMPLES;   
      waterSampleSum = 0;  
      waterSampleCount = 0;  
    }
  }
}

float readTemperature() {
  long sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += analogRead(THERMISTOR_PIN);
    delay(5);  
  }

  float average = sum / (float)NUM_SAMPLES;

  if (average < 1.0) {
    return -777; 
  }

  float tempC = 23.0 + ((average - 30.0) * (45.0 - 23.0) / (112.0 - 30.0));

  if (tempC > 100.0) tempC = 100.0;
  if (tempC < 0.0)   tempC = 0.0;

  return tempC;
}

// 8. DATA RESPONDER HANDLER (MURNI JSON API & PENGAMAN CORS)
void handleAPIStatus() {
  KelasKondisi hasilML = prediksiDecisionTree(flowRate, deltaFlow);
  
  String jsonResponse = "{";
  jsonResponse += "\"flowRate\":" + String(flowRate, 2) + ",";
  jsonResponse += "\"deltaFlow\":" + String(deltaFlow, 2) + ",";
  jsonResponse += "\"isPumpOff\":" + String(isPumpOff ? "true" : "false") + ",";
  jsonResponse += "\"prediksiML\":" + String((int)hasilML);
  jsonResponse += "}";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", jsonResponse);
}

void handleNyalakan() {
  noFlowStart = 0;
  isPumpOff = false;
  
  // Matikan buzzer & kembalikan indikator ke kondisi aman
  ledcWriteTone(BUZZER_CHANNEL, 0);
  digitalWrite(BUZZER_PIN, LOW);
  setRGB(false, false, false);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  Serial.println("[API] Perintah REST API: Pompa Dinyalakan. Sirine Mati.");
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "READY");
}

void handleMatikan() {
  digitalWrite(RELAY_PIN, LOW);
  pinMode(RELAY_PIN, INPUT);
  isPumpOff = true;
  Serial.println("[API] Perintah REST API: Pompa Dimatikan.");
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "SHUTDOWN");
}

// 9. SETUP
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== SYSTEM STARTING ===\n");

  analogSetAttenuation(ADC_11db);  
  analogSetWidth(12);              

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  setRGB(false, false, false);

  ledcSetup(BUZZER_CHANNEL, 2000, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); 
  lcd.print("SYSTEM INIT");
  lcd.setCursor(0, 1); 
  lcd.print("WAIT CONNECT...");

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  pinMode(WATER_SENSOR_PIN, INPUT);    
  pinMode(THERMISTOR_PIN, INPUT);      

  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING);

  digitalWrite(RELAY_PIN, LOW);

  WiFi.disconnect(true);
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);  

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempt++;
    if (attempt > 20) {
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      attempt = 0;
    }
  }

  Serial.println("\n[WiFi] WiFi CONNECTED!");
  
  lcd.clear();
  lcd.print("WiFi Connected!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(1500);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  isPumpOff = false;

  server.on("/api/status", handleAPIStatus);
  server.on("/action_nyalakan", handleNyalakan);
  server.on("/action_matikan", handleMatikan);
  server.begin();
}

// 10. LOOP
void loop() {
  server.handleClient();         
  updateWaterLevelAsync();       

  unsigned long currentMillis = millis();

  // --- LOGIC SIRINE AMBULANS UNTUK POMPA MACET ---
  if (isPumpOff) {
    // Berganti nada setiap 500 ms (Niuo-Niuo-Niuo)
    if (currentMillis - prevSirenMillis >= 500) {
      prevSirenMillis = currentMillis;
      sirenToneToggle = !sirenToneToggle;
      
      if (sirenToneToggle) {
        ledcWriteTone(BUZZER_CHANNEL, 1800); // Nada Tinggi
        setRGB(true, false, false);          // LED Merah Berkedip
      } else {
        ledcWriteTone(BUZZER_CHANNEL, 1200); // Nada Rendah
        setRGB(false, false, true);          // LED Biru Berkedip
      }
    }
  }

  static int lastCheckedWater = 0;
  if (abs(currentWaterLevelValue - lastCheckedWater) > 100) {
    checkSystemSafety(currentWaterLevelValue, currentTempStatus);
    lastCheckedWater = currentWaterLevelValue;
  }

  if (currentMillis - prevMillis1s >= 1000) {
    prevMillis1s = currentMillis;

    noInterrupts();   
    unsigned long localPulseCount = pulseCount;  
    pulseCount = 0;   
    interrupts();     

    flowRate = (float)localPulseCount / 7.5;  
    deltaFlow = flowRate - lastFlowRate;   
    lastFlowRate = flowRate;   

    float tempC = readTemperature();  
    
    if (tempC == -777) {
      Serial.println("[SAFETY] Hentakan Daya Terdeteksi, Menggunakan Data Suhu Terakhir.");
    } 
    else if (tempC == -999) {
      currentTempStatus = "ERROR";
    } 
    else {
      globalTempC = tempC; 

      if (tempC < 20.0) {
        currentTempStatus = "COLD";     
      }
      else if (tempC <= 26.0) {
        currentTempStatus = "NORMAL";   
      }
      else {
        currentTempStatus = "HOT";      
      }
    }

    checkSystemSafety(currentWaterLevelValue, currentTempStatus);  

    KelasKondisi statusML = prediksiDecisionTree(flowRate, deltaFlow);  

    if (statusML == MACET) {   
      if (noFlowStart == 0) {  
        noFlowStart = currentMillis;  
      }  
      if ((currentMillis - noFlowStart >= 5000) && !isPumpOff) {  
        digitalWrite(RELAY_PIN, LOW);   
        pinMode(RELAY_PIN, INPUT);
        isPumpOff = true;  
        Serial.println(" -> [ML] STATUS: POMPA MACET! SAFETY SHUTDOWN.");  
      }  
    }   
    else if (statusML == ANOMALI_BOCOR) {  
      Serial.println(" -> [ML] STATUS: ADA ANOMALI / INDIKASI BOCOR ALUS");  
      noFlowStart = 0;   
    }   
    else {  
      if (!isPumpOff) {  
        noFlowStart = 0;  
        Serial.println(" -> [ML] STATUS: NORMAL");  
      }  
    }   

    // --- Cetak Log Serial ---  
    Serial.print("Debit: "); Serial.print(flowRate, 2);  
    Serial.print(" L/min | Delta: "); Serial.println(deltaFlow, 2);  
    Serial.print("Water ADC: "); Serial.println(currentWaterLevelValue);  
    Serial.print("Temp: "); Serial.print(globalTempC, 1); Serial.print(" C | Status Temp: "); Serial.println(currentTempStatus);  
    Serial.println("---------------------------------");  

    // --- Update Tampilan LCD ---  
    lcd.clear();  
    lcd.setCursor(0, 0);  
    lcd.print("T:");
    lcd.print(globalTempC, 1);
    lcd.print("C ");
    lcd.print(currentTempStatus);  
    
    lcd.setCursor(0, 1);  
    lcd.print("W:");
    lcd.print(currentWaterLevelValue);  
    lcd.print(" F:");
    lcd.print(flowRate, 1);
  }

  if (currentMillis - prevThingSpeakMillis >= thingSpeakInterval) {
    prevThingSpeakMillis = currentMillis;

    if (WiFi.status() == WL_CONNECTED) {  
      HTTPClient http;  
      String url = String(serverName) + "?api_key=" + writeAPIKey   
                   + "&field1=" + String(globalTempC, 1)   
                   + "&field2=" + String(currentWaterLevelValue)   
                   + "&field3=" + currentTempStatus;  
      http.begin(url);  
      int httpResponseCode = http.GET();  
      http.end();  
    }
  }
}