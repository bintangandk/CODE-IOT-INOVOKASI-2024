///////////////////////COPYRIGHT INOVOKASI 2024////////////////
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define DHTPIN D7
#define PINMQ A0
#define PINRELAY D6
#define PINLEDMerah D1
#define PINLEDBiru D2
#define PINLEDHijau D5
#define LED_INTERNAL LED_BUILTIN

#define DHTTYPE 21
DHT dht(DHTPIN, DHTTYPE);

bool DHTCondition = false;
bool MQ135Condition = false;
bool ESPRun = false;

float humidity;
float temperature;
float nilai_Amonia;

//////////////// Milis For send data
unsigned long previousMillis = 0;
const long interval = 60000; // Interval 1 menit (60000 ms)

//////////////// Milis For control Relay
unsigned long previousMillis2 = 0;
const long interval2 = 1000; // Interval 1 detik (1000 ms)

// HTTP client Config
const char *ssid = "SSID13";
const char *password = "1sampai8";
const int id_device = 7;

// Control Relay
const char *id_alat_ctrl;
const char *control_type;
int control_value; // Mengubah control_value menjadi tipe int

/////////////////// Pemilihan alat /////////////////////////
#define PENETASAN_ITIK 0 // Set to 1 to enable, 0 to disable
#define NURSERY_KOPI 1   // Set to 1 to enable, 0 to disable
#define KANDANG_AYAM 0   // Set to 1 to enable, 0 to disable
////////////////////////////////////////////////////////////

// Conditional compilation check to ensure only one feature is enabled
#if (PENETASAN_ITIK + NURSERY_KOPI + KANDANG_AYAM) > 1
#error "Hanya Aktifkan Salah Satu."
#elif (PENETASAN_ITIK + NURSERY_KOPI + KANDANG_AYAM) == 0
#error "Aktifkan Setidaknya Satu."
#endif

// Link Selection
#if PENETASAN_ITIK == 1
// Code specific to Feature 1
const char URL_temperature[] = "http://iqacs-chick.research-ai.my.id/public/temperature_data.php";
const char URL_amonia[] = "http://iqacs-chick.research-ai.my.id/public/amonia_data.php";
const char URL_humidity[] = "http://iqacs-chick.research-ai.my.id/public/humidity_data.php";
#elif NURSERY_KOPI == 1
// Code specific to Feature 2
const char URL_humidity[] = "https://is4ac-nursery.research-ai.my.id/public/humidity_data.php";
const char URL_temperature[] = "https://is4ac-nursery.research-ai.my.id/public/temperature_data.php";
const char URL_control_relay[] = "http://is4ac-nursery.research-ai.my.id/public/controlPHP.php";
#elif KANDANG_AYAM == 1
// Code specific to Feature 3
const char URL_temperature[] = "http://iqacs-chick.research-ai.my.id/public/temperature_data.php";
const char URL_amonia[] = "http://iqacs-chick.research-ai.my.id/public/amonia_data.php";
const char URL_humidity[] = "http://iqacs-chick.research-ai.my.id/public/humidity_data.php";
#endif

/////////////// Compile check

void readDHT21()
{
  delay(1);
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    DHTCondition = false;
    return;
  }
  else
  {
    DHTCondition = true;
  }

  Serial.println("humidity :" + String(humidity));
  Serial.println("temperature :" + String(temperature));
  delay(1);
}

void readMQ135()
{
  nilai_Amonia = analogRead(PINMQ);
  Serial.println("Nilai Amonia : " + String(nilai_Amonia));
  delay(1);
  if (nilai_Amonia < 50 || nilai_Amonia > 1000)
  {
    MQ135Condition = false;
  }
  else
  {
    MQ135Condition = true;
  }
}

void showSensorError()
{
#if KANDANG_AYAM || PENETASAN_ITIK
  if ((DHTCondition == true && MQ135Condition == true))
  {
    Serial.println("AllSensor OK");
    delay(1);
  }
  else if (DHTCondition == true && MQ135Condition == false)
  {
    for (int i = 0; i < 2; i++)
    {
      digitalWrite(PINLEDBiru, HIGH);
      digitalWrite(PINLEDMerah, HIGH);
      delay(200);
      digitalWrite(PINLEDBiru, LOW);
      digitalWrite(PINLEDMerah, LOW);
      delay(200);
      Serial.println("MQ135 Error");
      delay(1);
    }
  }
  else if (DHTCondition == false && MQ135Condition == true)
  {
    for (int i = 0; i < 3; i++)
    {
      digitalWrite(PINLEDBiru, HIGH);
      digitalWrite(PINLEDMerah, HIGH);
      delay(200);
      digitalWrite(PINLEDBiru, LOW);
      digitalWrite(PINLEDMerah, LOW);
      delay(200);
      Serial.println("DHT Error");
      delay(1);
    }
  }
  else if (DHTCondition == false && MQ135Condition == false)
  {
    for (int i = 0; i < 4; i++)
    {
      digitalWrite(PINLEDBiru, HIGH);
      digitalWrite(PINLEDMerah, HIGH);
      delay(200);
      digitalWrite(PINLEDBiru, LOW);
      digitalWrite(PINLEDMerah, LOW);
      delay(200);
      Serial.println("All Sensor Error");
      delay(1);
    }
  }
#elif NURSERY_KOPI
  if (DHTCondition == true)
  {
    Serial.println("DHT OK");
    delay(1);
  }
  else if (DHTCondition == false)
  {
    Serial.println("DHT Error");
    delay(1);
  }
#endif
}

void handleAllLed(bool ledMerah, bool ledKuning, bool ledBiru)
{
  digitalWrite(PINLEDMerah, ledMerah);
  digitalWrite(PINLEDBiru, ledKuning);
  digitalWrite(PINLEDHijau, ledBiru);
  delay(1);
}

// Mengirim data menggunakan char array alih-alih String
// void sendDataHandle(const int idalat, float nilaisensor, const char *URL)
// {
//   char postData[100]; // Memastikan buffer cukup untuk menampung data
//   snprintf(postData, sizeof(postData), "id_alat=%d&nilai=%.2f", idalat, nilaisensor);

//   WiFiClient client;
//   HTTPClient http;
//   http.begin(client, URL);
//   http.addHeader("Content-Type", "application/x-www-form-urlencoded");

//   int httpCode = http.POST(postData);
//   String payload = http.getString();
//   Serial.println(URL);
//   Serial.println(postData);
//   if (httpCode > 0)
//   {
//     String payload = http.getString();
//     Serial.println("Response: " + payload);
//   }
//   else
//   {
//     Serial.printf("Error on HTTP request: %s\n", http.errorToString(httpCode).c_str());
//   }

//   http.end();
// }

// Mengirim data String
void sendDataHandle(const int idalat, float nilaisensor, String URL)
{
  String postData = (String) "id_alat=" + idalat + "&nilai=" + nilaisensor;

  WiFiClient client;
  HTTPClient http;
  http.begin(client, URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  auto httpCode = http.POST(postData);
  String payload = http.getString();
  Serial.println(URL);
  Serial.println(postData);
  Serial.println(payload);

  http.end();
}
#if NURSERY_KOPI == 1
void handleControl(int action_value)
{
  Serial.println(action_value);
  delay(1);
  if (action_value == 1)
  {
    digitalWrite(PINRELAY, LOW);
    Serial.println("led hidup");
    delay(10);
  }
  else if (action_value == 0)
  { // Memastikan untuk mematikan LED jika action_value 0
    digitalWrite(PINRELAY, HIGH);
    Serial.println("led mati");
    delay(10);
  }
  else
  {
    Serial.println("Wrong State Value");
  }
}

void reciveDataHandle()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    WiFiClient client;
    http.begin(client, URL_control_relay);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
      Serial.println("Data dari server:");
      Serial.println(payload.c_str());

      // Membuat salinan dari `payload` untuk digunakan oleh `strtok`
      char data[100];
      payload.toCharArray(data, 100);

      // Memisahkan data menggunakan delimiter koma
      id_alat_ctrl = strtok(data, ",");
      control_type = strtok(NULL, ",");
      const char *control_value_str = strtok(NULL, ","); // Simpan hasil strtok

      if (id_alat_ctrl != NULL && control_type != NULL && control_value_str != NULL)
      {
        Serial.print("ID Alat: ");
        Serial.println(id_alat_ctrl);
        Serial.print("Control Type: ");
        Serial.println(control_type);
        Serial.print("Control Value: ");
        Serial.println(control_value_str);

        // Konversi `control_value_str` ke integer
        control_value = atoi(control_value_str); // Mengonversi string ke integer
      }
      else
      {
        Serial.println("Data tidak lengkap atau format salah.");
      }
    }
    else
    {
      Serial.println("Gagal terhubung ke server, Kode HTTP: " + String(httpCode));
    }

    http.end();
  }
  else
  {
    Serial.println("Koneksi WiFi terputus!");
  }

  // Kontrol perangkat berdasarkan nilai `control_value`
  handleControl(control_value);
}
#endif

void handleLEDState()
{
  if (WL_CONNECTED)
  {
    digitalWrite(PINLEDBiru, HIGH);
    delay(1);
    if (ESPRun)
    {
      digitalWrite(PINLEDMerah, HIGH);
      delay(1);
    }
  }
}

void checkDeviceConfig()
{
  if (PENETASAN_ITIK)
  {
    Serial.println("Konfigurasi Alat Untuk Penetasan Itik");
  }
  if (NURSERY_KOPI)
  {
    Serial.println("Konfigurasi Alat Untuk Nursery Kopi");
  }
  if (KANDANG_AYAM)
  {
    Serial.println("Konfigurasi Alat Untuk Kandang Ayam");
  }
}

void CheckAllSensor()
{
  if (PENETASAN_ITIK)
  {
    readDHT21();
    readMQ135();
  }
  if (NURSERY_KOPI)
  {
    readDHT21();
    MQ135Condition = true;
  }
  if (KANDANG_AYAM)
  {
    readDHT21();
    readMQ135();
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Start Program");
  delay(1);
  ESPRun = true;
  dht.begin();
  pinMode(PINMQ, INPUT);
  pinMode(LED_INTERNAL, OUTPUT);
  pinMode(PINLEDMerah, OUTPUT);
  pinMode(PINLEDBiru, OUTPUT);
  pinMode(PINLEDHijau, OUTPUT);
  handleAllLed(0, 0, 0);
  digitalWrite(LED_INTERNAL, HIGH);
  pinMode(PINRELAY, OUTPUT);
  digitalWrite(PINRELAY, HIGH);
  digitalWrite(PINLEDMerah, HIGH);
  checkDeviceConfig();
  delay(1);

  WiFi.begin(ssid, password);
  Serial.println(String("Connecting to :") + ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(PINLEDBiru, HIGH);
    delay(200);
    digitalWrite(PINLEDBiru, LOW);
    delay(200);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  delay(1);
  digitalWrite(LED_INTERNAL, LOW);
  delay(500);
}

void loop()
{
  Serial.println("Run Loop Program");
  delay(2000);
  unsigned long currentMillis = millis();
  unsigned long currentMillis2 = millis();

  CheckAllSensor();
  handleLEDState();
  showSensorError();
  Serial.println(DHTCondition);
  Serial.println(MQ135Condition);

#if NURSERY_KOPI == 1
  if (currentMillis2 - previousMillis2 >= interval2)
  {
    previousMillis2 = currentMillis2;
    reciveDataHandle();
    handleControl(control_value);
    delay(10);
  }
#endif

  if (DHTCondition && MQ135Condition)
  {
    Serial.println("Data Siap Di Kirim");
    delay(1);
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
      previousMillis = currentMillis;

      // fungsi untuk mengatur jenis url yang akan di kirim
#if PENETASAN_ITIK == 1 || KANDANG_AYAM == 1
      sendDataHandle(id_device, temperature, URL_temperature);
      digitalWrite(PINLEDHijau, HIGH);
      delay(1000);
      digitalWrite(PINLEDHijau, LOW);
      delay(1000);

      sendDataHandle(id_device, humidity, URL_humidity);
      digitalWrite(PINLEDHijau, HIGH);
      delay(1000);
      digitalWrite(PINLEDHijau, LOW);
      delay(1000);

      sendDataHandle(id_device, nilai_Amonia, URL_amonia);
      digitalWrite(PINLEDHijau, HIGH);
      delay(1000);
      digitalWrite(PINLEDHijau, LOW);
      delay(1000);

#elif NURSERY_KOPI == 1

      sendDataHandle(id_device, temperature, URL_temperature);
      digitalWrite(PINLEDHijau, HIGH);
      delay(1000);
      digitalWrite(PINLEDHijau, LOW);
      delay(1000);

      sendDataHandle(id_device, humidity, URL_humidity);
      digitalWrite(PINLEDHijau, HIGH);
      delay(1000);
      digitalWrite(PINLEDHijau, LOW);
      delay(1000);
#endif
    }
  }
}
