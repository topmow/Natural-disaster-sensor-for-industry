#define BLYNK_TEMPLATE_NAME "Disaster warning for industry"
#define BLYNK_TEMPLATE_ID "TMPL6vajFpDFd"
#define BLYNK_AUTH_TOKEN "SBjhyyMtmRPq6fTj5nGDIvBtzkG-2WvY"
#define BLYNK_PRINT Serial
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Blynk.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <String.h>
const int sw420Pin = 14;
const int buzzerPin = 16;
const int button1 = 3;
#define NUM_LEDS 12 
#define DATA_PIN 0
#define RX 12
#define TX 13
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

// Định nghĩa chân kết nối
#define PIN_MQ135 A0    // Cảm biến MQ135
#define PIN_DHT11 2      // Cảm biến DHT11 (GPIO2)

// Khởi tạo cảm biến DHT11
#define DHTTYPE DHT11
DHT dht(PIN_DHT11, DHTTYPE);

// Kích thước màn hình OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Địa chỉ I2C của màn hình OLED (thường là 0x3C)
#define OLED_ADDRESS 0x3C
// Khởi tạo màn hình OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
SoftwareSerial SIM800A(RX, TX);
char auth[] = BLYNK_AUTH_TOKEN;             // You should get Auth Token in the Blynk App.
char ssid[] = ".";                                     // Your WiFi credentials.
char pass[] = "123bk456";
String Earthquake;
String receiveNum = "0388649473";
String Warning;

void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}

void setup() {
  Serial.begin(9600); // Khởi tạo Serial để debug
  SIM800A.begin(9600);
  pinMode(buzzerPin, OUTPUT);
  Blynk.begin(auth, ssid, pass);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pinMode(sw420Pin, INPUT); 
  pinMode(button1, INPUT); 
  // Khởi tạo DHT11
  dht.begin();
  // Khởi tạo màn hình OLED
  if (!display.begin(SSD1306_PAGEADDR, OLED_ADDRESS)) {
    Serial.println(F("OLED display not found!"));
    while (true); // Dừng chương trình
  }

  // Xóa màn hình
  display.clearDisplay();

  // Hiển thị thông báo ban đầu
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("ESP8266 + OLED"));
  display.println(F("DHT11 & MQ135"));
  display.display();
  delay(2000); // Hiển thị trong 2 giây
}

void ShowSerialData() {
  while (SIM800A.available() != 0)
    Serial.write(SIM800A.read());
  delay(1000);
}
void loop() {
  Blynk.run();
  int sensorValue = digitalRead(sw420Pin); // Đọc tín hiệu từ SW420
  int buttonStatus1 = digitalRead(button1); 
    if (buttonStatus1 == LOW) { // Nếu mà button bị nhấn
    Serial.print("test on\n");
    digitalWrite(buzzerPin, HIGH); // Bật còi
     delay(1000);
     colorWipe(strip.Color(255, 255, 255), 50); // Màu trắng
     delay(1000); // Giữ sáng 500ms

  } else if(buttonStatus1 == HIGH) { // ngược lại
    digitalWrite(buzzerPin,LOW); // Bật còi
    Serial.print("test off\n");  
  }
  if (sensorValue == HIGH) { // Khi có rung động
    Earthquake = "There's vibration! Earthquake warning";
    Warning = "There's vibration! Earthquake warning";
    sentWarning();
    digitalWrite(buzzerPin, HIGH); // Bật còi
    Serial.println("-------------------------------------------------------------------------------------");
    Serial.println("Vibration, buzzer on!"); // In thông báo qua Serial
    Blynk.virtualWrite(V4, Earthquake);
    // Hiệu ứng nhấp nháy toàn bộ dải LED với màu đỏ
    for (int i = 0; i < 3; i++) { // Nhấp nháy 3 lần
      colorWipe(strip.Color(255, 0, 0), 50); // Đèn đỏ sáng
      delay(500); // Giữ sáng 500ms
      colorWipe(strip.Color(0, 0, 0), 50); // Tắt đèn
      delay(500); // Giữ tắt 500ms
    }

    delay(5000); // Giữ còi kêu trong 5 giây
  } else {
    digitalWrite(buzzerPin, LOW); // Tắt còi
    Serial.println("-------------------------------------------------------------------------------------");
    Serial.println("No vibration"); // In thông báo qua Serial
    Earthquake = "Normal";
    Blynk.virtualWrite(V4, Earthquake);
    // Tắt tất cả LED
    colorWipe(strip.Color(0, 0, 0), 50);
  }
  // Đọc giá trị ADC từ MQ135
  int mq135_adc = analogRead(PIN_MQ135);
  // Dùng giá trị ADC làm giá trị PPM trực tiếp
  float ppm = mq135_adc;
  Blynk.virtualWrite(V2, ppm);
  // Đọc nhiệt độ và độ ẩm từ DHT11
  float temperature = dht.readTemperature();
  Blynk.virtualWrite(V0, temperature);
  float humidity = dht.readHumidity();
  Blynk.virtualWrite(V1, humidity);
  // Kiểm tra lỗi đọc từ DHT11
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println(F("DHT11 sensor reading error!"));
    return;
  }
  if (temperature > 40){
    digitalWrite(buzzerPin, HIGH);
    Warning = "The temperatures is above 40°C! Careful for heat waves";
    sentWarning();
    colorWipe(strip.Color(255, 0, 0), 50);
  } else {
    digitalWrite(buzzerPin, LOW);

  }
  if (humidity > 85 && humidity <= 100){
    digitalWrite(buzzerPin, HIGH);
    Warning = "Humidity is high! There is a chance of flood hazzard";
    sentWarning();
    colorWipe(strip.Color(255, 0, 0), 50);
  } else {
    digitalWrite(buzzerPin, LOW);
  }
  // Phân loại chất lượng không khí
  String airQuality;
  if (ppm <= 50) {
    digitalWrite(buzzerPin, LOW);
    colorWipe(strip.Color(0, 255, 0), 50);
    airQuality = "Good";
  } else if (ppm <= 100) {
    digitalWrite(buzzerPin, LOW);
    colorWipe(strip.Color(255, 255, 0), 50); // Màu vàng
    airQuality = "Normal";
  } else if (ppm <= 200) {
    digitalWrite(buzzerPin, LOW);
    colorWipe(strip.Color(255, 165, 0), 50); // Màu cam
    airQuality = "Poor";
  } else if (ppm <= 300) {
    digitalWrite(buzzerPin, LOW);
    Warning = "Air quality in very poor condition! Please wear a mask";
    sentWarning();
    colorWipe(strip.Color(128, 0, 128), 50); // Màu tím
    airQuality = "Very Poor";
  } else {
    digitalWrite(buzzerPin, HIGH);
    Warning = "Air quality in danger zone! Gas leak warning";
    sentWarning();
    colorWipe(strip.Color(255, 0, 0), 50);
    airQuality = "Danger";
  }
  Blynk.virtualWrite(V3, airQuality);
  // Hiển thị thông tin trên màn hình OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("DHT11 & MQ135"));

  display.setCursor(0, 10);
  display.print(F("Temperature: "));
  display.print(temperature);
  display.println(F(" *C"));

  display.setCursor(0, 20);
  display.print(F("Humidity: "));
  display.print(humidity);
  display.println(F(" %"));

  display.setCursor(0, 30);
  display.print(F("AQI: "));
  display.print(ppm, 2);

  display.setCursor(0, 40);
  display.print(F("Level: "));
  display.println(airQuality);

  display.drawRect(0, 50, SCREEN_WIDTH, 10, SSD1306_WHITE);
  int barWidth = map(ppm, 0, 1023, 0, SCREEN_WIDTH);
  if (barWidth > SCREEN_WIDTH) barWidth = SCREEN_WIDTH;
  display.fillRect(0, 50, barWidth, 10, SSD1306_WHITE);

  display.display();
  Serial.print(F("Temperture: "));
  Serial.print(temperature);
  Serial.println(F(" °C"));
  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.println(F(" %"));
  Serial.print(F("AQI: "));
  Serial.print(ppm);
  Serial.print(F(", Level: "));
  Serial.println(airQuality);
  Serial.println("-------------------------------------------------------------------------------------");
  delay(1000);
}
void sentWarning(){
    if (SIM800A.available()){
    Serial.write(SIM800A.read());
    }
    SIM800A.println("AT");
    delay(1000);
    SIM800A.println("AT+IPR=9600");
    delay(1000);
    SIM800A.println("AT+CMGF=1");
    delay(1000);
    SIM800A.println("AT+CMGS=\"" + receiveNum + "\"");
    delay(1000);
    SIM800A.println(Warning);
    delay(1000);
    SIM800A.println((char)26);
    delay(1000);
    Serial.println("Message sent!");
}
