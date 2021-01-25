#include <SimpleDHT.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <TFT_eSPI.h>
#include <ESP8266WiFi.h>
#include <FS.h> // 本程序使用SPIFFS库

int counter = 0;
int count = 0; //时间计数
int count1;    //Ticker计数用变量
int timeNum = 0;
bool WIFI_Status = true; //WIFI状态标志位
char auth[] = "ac8cb23e3b49";
bool result = 0;                                                              
bool picture = false;

Ticker ticker;
WiFiClient wificlient;
PubSubClient mqttClient(wificlient);

#define greenLed D3 //要操作的IO
#define redLed D4
//定义光照传感器引脚
#define PIN_A A0
#define PIN_D D0
int val1, val2;

//设置WiFi接入信息
const char *ssid = "IOT";
const char *password = "987654123";
const char *mqttServer = "39.105.123.55";

//MQTT服务器账号密码
const char *mqttId = "admin";
const char *mqttPwd = "password";

//定义温湿度模块引脚及变量
int pinDHT11 = D2;
SimpleDHT11 dht11(pinDHT11);
byte temp_read = 0;
byte humi_read = 0;

// 遗嘱设置
const char *willMsg = "CLIENT-OFFLINE"; // 遗嘱消息内容
const int willQoS = 2;                  // 遗嘱QoS
const bool willRetain = true;           // 遗嘱保留

//定义系统时间变量，用作定时执行函数
unsigned long currentMillis = millis();
unsigned long previousMillis = 0;
unsigned long period1 = 10000;
unsigned long period2 = 100;

static const unsigned char PROGMEM zheng[] = {
    0x00, 0x00, 0x7F, 0xFC, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x11, 0x00, 0x11, 0xF8,
    0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0xFF, 0xFE, 0x00, 0x00}; /*"正",0*/
static const unsigned char PROGMEM zai[] = {
    0x02, 0x00, 0x02, 0x00, 0x04, 0x00, 0xFF, 0xFE, 0x08, 0x00, 0x08, 0x40, 0x10, 0x40, 0x30, 0x40,
    0x57, 0xFC, 0x90, 0x40, 0x10, 0x40, 0x10, 0x40, 0x10, 0x40, 0x10, 0x40, 0x1F, 0xFE, 0x10, 0x00}; /*"在",1*/
static const unsigned char PROGMEM lian[] = {
    0x00, 0x40, 0x20, 0x40, 0x17, 0xFE, 0x10, 0x80, 0x00, 0xA0, 0x01, 0x20, 0xF3, 0xFC, 0x10, 0x20,
    0x10, 0x20, 0x10, 0x20, 0x17, 0xFE, 0x10, 0x20, 0x10, 0x20, 0x28, 0x20, 0x47, 0xFE, 0x00, 0x00}; /*"连",2*/
static const unsigned char PROGMEM jie[] = {
    0x10, 0x80, 0x10, 0x40, 0x13, 0xFC, 0x10, 0x00, 0xFD, 0x08, 0x10, 0x90, 0x17, 0xFE, 0x10, 0x40,
    0x18, 0x40, 0x37, 0xFE, 0xD0, 0x88, 0x11, 0x08, 0x10, 0x90, 0x10, 0x60, 0x51, 0x98, 0x26, 0x04}; /*"接",3*/
static const unsigned char PROGMEM guang[] = {
    0x01, 0x00, 0x21, 0x08, 0x11, 0x08, 0x09, 0x10, 0x09, 0x20, 0x01, 0x00, 0xFF, 0xFE, 0x04, 0x40,
    0x04, 0x40, 0x04, 0x40, 0x04, 0x40, 0x08, 0x42, 0x08, 0x42, 0x10, 0x42, 0x20, 0x3E, 0xC0, 0x00 /*"光",0*/
};
static const unsigned char PROGMEM zhao[] = {
    0x00, 0x00, 0x7D, 0xFC, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x94, 0x7D, 0x08, 0x44, 0xFC,
    0x44, 0x84, 0x44, 0x84, 0x44, 0x84, 0x7C, 0xFC, 0x00, 0x00, 0x48, 0x88, 0x44, 0x44, 0x84, 0x44 /*"照",1*/
};
static const unsigned char PROGMEM qiang[] = {
    0x00, 0x00, 0xF9, 0xFC, 0x09, 0x04, 0x09, 0x04, 0x09, 0xFC, 0x78, 0x20, 0x40, 0x20, 0x43, 0xFE,
    0x42, 0x22, 0x7A, 0x22, 0x0B, 0xFE, 0x08, 0x20, 0x08, 0x24, 0x08, 0x22, 0x57, 0xFE, 0x20, 0x02 /*"强",2*/
};
static const unsigned char PROGMEM du[] = {
    0x01, 0x00, 0x00, 0x80, 0x3F, 0xFE, 0x22, 0x20, 0x22, 0x20, 0x3F, 0xFC, 0x22, 0x20, 0x22, 0x20,
    0x23, 0xE0, 0x20, 0x00, 0x2F, 0xF0, 0x24, 0x10, 0x42, 0x20, 0x41, 0xC0, 0x86, 0x30, 0x38, 0x0E /*"度",3*/
};
static const unsigned char PROGMEM wen[] = {
    0x00, 0x00, 0x23, 0xF8, 0x12, 0x08, 0x12, 0x08, 0x83, 0xF8, 0x42, 0x08, 0x42, 0x08, 0x13, 0xF8,
    0x10, 0x00, 0x27, 0xFC, 0xE4, 0xA4, 0x24, 0xA4, 0x24, 0xA4, 0x24, 0xA4, 0x2F, 0xFE, 0x00, 0x00 /*"温",0*/
};
static const unsigned char PROGMEM shi[] = {
    0x00, 0x00, 0x27, 0xF8, 0x14, 0x08, 0x14, 0x08, 0x87, 0xF8, 0x44, 0x08, 0x44, 0x08, 0x17, 0xF8,
    0x11, 0x20, 0x21, 0x20, 0xE9, 0x24, 0x25, 0x28, 0x23, 0x30, 0x21, 0x20, 0x2F, 0xFE, 0x00, 0x00 /*"湿",1*/
};

static const unsigned char PROGMEM wang[] = {
    0x00, 0x00, 0x7F, 0xFC, 0x40, 0x04, 0x40, 0x04, 0x42, 0x14, 0x52, 0x94, 0x4A, 0x54, 0x44, 0x24,
    0x44, 0x24, 0x4A, 0x54, 0x4A, 0x54, 0x52, 0x94, 0x61, 0x04, 0x40, 0x04, 0x40, 0x14, 0x40, 0x08 /*"网",0*/
};
static const unsigned char PROGMEM luo[] = {
    0x10, 0x80, 0x10, 0x80, 0x20, 0xF8, 0x21, 0x08, 0x4B, 0x10, 0xFC, 0xA0, 0x10, 0x40, 0x20, 0xA0,
    0x41, 0x18, 0xFA, 0x06, 0x45, 0xF8, 0x01, 0x08, 0x19, 0x08, 0xE1, 0x08, 0x41, 0xF8, 0x01, 0x08 /*"络",1*/
};
static const unsigned char PROGMEM duan[] = {
    0x04, 0x00, 0x04, 0x04, 0x55, 0x78, 0x4E, 0x40, 0x44, 0x40, 0x7F, 0x40, 0x44, 0x7E, 0x4E, 0x48,
    0x55, 0x48, 0x65, 0x48, 0x44, 0x48, 0x44, 0x48, 0x40, 0x48, 0x7F, 0x88, 0x00, 0x88, 0x01, 0x08 /*"断",2*/
};
static const unsigned char PROGMEM kai[] = {
    0x00, 0x00, 0x7F, 0xFC, 0x08, 0x20, 0x08, 0x20, 0x08, 0x20, 0x08, 0x20, 0x08, 0x20, 0xFF, 0xFE,
    0x08, 0x20, 0x08, 0x20, 0x08, 0x20, 0x08, 0x20, 0x10, 0x20, 0x10, 0x20, 0x20, 0x20, 0x40, 0x20 /*"开",3*/
};
static const unsigned char PROGMEM douhao[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x30, 0x00, 0x10, 0x00, 0x20, 0x00, 0x00, 0x00 /*"，",4*/
};
static const unsigned char PROGMEM chong[] = {
    0x00, 0x10, 0x00, 0xF8, 0x3F, 0x00, 0x01, 0x00, 0xFF, 0xFE, 0x01, 0x00, 0x1F, 0xF0, 0x11, 0x10,
    0x1F, 0xF0, 0x11, 0x10, 0x1F, 0xF0, 0x01, 0x00, 0x3F, 0xF8, 0x01, 0x00, 0xFF, 0xFE, 0x00, 0x00 /*"重",5*/
};
static const unsigned char PROGMEM xin[] = {
    0x10, 0x00, 0x08, 0x04, 0x7F, 0x78, 0x00, 0x40, 0x22, 0x40, 0x14, 0x40, 0xFF, 0x7E, 0x08, 0x48,
    0x08, 0x48, 0x7F, 0x48, 0x08, 0x48, 0x2A, 0x48, 0x49, 0x48, 0x88, 0x88, 0x28, 0x88, 0x11, 0x08 /*"新",0*/
};
static const unsigned char PROGMEM pei[] = {
    0x00, 0x00, 0xFE, 0x00, 0x28, 0xF8, 0x28, 0x08, 0xFE, 0x08, 0xAA, 0x08, 0xAA, 0x08, 0xAA, 0xF8,
    0xAE, 0x88, 0xC2, 0x80, 0x82, 0x80, 0xFE, 0x80, 0x82, 0x82, 0x82, 0x82, 0xFE, 0x7E, 0x82, 0x00 /*"配",0*/
};
static const unsigned char PROGMEM shi_bai[] = {
    0x01, 0x00, 0x11, 0x00, 0x11, 0x00, 0x11, 0x00, 0x3F, 0xF8, 0x21, 0x00, 0x41, 0x00, 0x01, 0x00,
    0xFF, 0xFE, 0x02, 0x80, 0x04, 0x40, 0x04, 0x40, 0x08, 0x20, 0x10, 0x10, 0x20, 0x08, 0xC0, 0x06 /*"失",1*/
};
static const unsigned char PROGMEM bai[] = {
    0x00, 0x40, 0x7C, 0x40, 0x44, 0x40, 0x54, 0x80, 0x54, 0xFE, 0x55, 0x08, 0x56, 0x88, 0x54, 0x88,
    0x54, 0x88, 0x54, 0x50, 0x54, 0x50, 0x10, 0x20, 0x28, 0x50, 0x24, 0x88, 0x45, 0x04, 0x82, 0x02 /*"败",2*/
};
static const unsigned char PROGMEM qing[] = {
    0x00, 0x40, 0x40, 0x40, 0x27, 0xFC, 0x20, 0x40, 0x03, 0xF8, 0x00, 0x40, 0xE7, 0xFE, 0x20, 0x00,
    0x23, 0xF8, 0x22, 0x08, 0x23, 0xF8, 0x22, 0x08, 0x2B, 0xF8, 0x32, 0x08, 0x22, 0x28, 0x02, 0x10 /*"请",0*/
};

TFT_eSPI tft = TFT_eSPI();

void setup()
{

  //初始化端口
  Serial.begin(115200);

  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS initialisation failed!");
    while (1)
      yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nSPIFFS initialised.");

  //初始化RGB灯
  initRGB();

  //初始化显示屏
  tft.init();
  tft.setRotation(1);
  tft.setTextWrap(true);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);

  //连接WiFi
  connectWiFi();

  //设置MQTT服务器和端口号
  mqttClient.setServer(mqttServer, 61613);
  mqttClient.setKeepAlive(5);
  //设置MQTT订阅回调函数
  mqttClient.setCallback(receiveCallback);

  //连接MQTT服务器
  connectMQTTServer();

  //Ticker定时对象
  ticker.attach(1, tickerCount);

  tft.setCursor(0, 10);
  tft.setTextColor(TFT_BLUE);
  tft.print("Connected to ");
  tft.println(WiFi.SSID());
  tft.println("IP address:");
  tft.setTextColor(TFT_GREEN);
  tft.println(WiFi.localIP());
}

void loop()
{

  if (WiFi.status() != WL_CONNECTED)
  {
    tft.fillScreen(TFT_BLACK);
    tft.drawBitmap(2, 2, wang, 16, 16, TFT_RED);
    tft.drawBitmap(20, 2, luo, 16, 16, TFT_RED);
    tft.drawBitmap(38, 2, duan, 16, 16, TFT_RED);
    tft.drawBitmap(56, 2, kai, 16, 16, TFT_RED);
    tft.drawBitmap(74, 2, douhao, 16, 16, TFT_RED);
    tft.drawBitmap(92, 2, zheng, 16, 16, TFT_RED);
    tft.drawBitmap(110, 2, zai, 16, 16, TFT_RED);
    tft.drawBitmap(128, 2, chong, 16, 16, TFT_RED);
    tft.drawBitmap(146, 2, lian, 16, 16, TFT_RED);
    tft.setCursor(164, 2);
    tft.setTextColor(TFT_RED);
    tft.print("...");
    Serial.println("Connecting...");
    delay(1000);
  }
  else if (picture == false && WiFi.status() == WL_CONNECTED)
  {
    tft.fillRect(0, 0, 240, 90, TFT_BLACK);
    tft.setCursor(0, 10);
    tft.setTextColor(TFT_BLUE);
    tft.print("Connected to ");
    tft.println(WiFi.SSID());
    tft.println("IP address:");
    tft.setTextColor(TFT_GREEN);
    tft.println(WiFi.localIP());

    if (mqttClient.connected())
    { //如果开发板成功连接服务器
      tft.setCursor(0, 60);
      tft.print("Connected to MQTT!");
      if (count1 >= 3)
      {
        publishOnlineStatus(); //发布在线状态
        pubMQTTmsg();
        count1 = 0;
      }
      mqttClient.loop(); //保持客户端心跳
    }
    else
    { //如果开发板未能成功连接服务器
      tft.fillRect(0, 60, 240, 18, TFT_BLACK);
      tft.setCursor(0, 60);
      tft.setTextColor(TFT_RED);
      tft.print("Connect to MQTT failed!");
      connectMQTTServer(); //则尝试连接服务器
    }

    //定时执行获取温湿度函数
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= period1)
    {                                 // check if 1000ms passed
      previousMillis = currentMillis; // save the last time you blinked the LED
      getHumChk();
    }
    getLight();
  }
  else
  {
    drawBmp("/1.bmp", 0, 0);
  }
}

void tickerCount()
{
  count1++;
}

//初始化RGB灯
void initRGB()
{
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);

  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, HIGH);
}

//连接MQTT服务器
void connectMQTTServer()
{
  //根据ESP8266的MAC地址生成客户端ID（避免与其他设备产生冲突）
  String clientId = "esp8266-" + WiFi.macAddress();

  // 建立遗嘱主题。
  String willString = "Mymqtt/module1/willMessage";
  char willTopic[willString.length() + 1];
  strcpy(willTopic, willString.c_str());

  //连接MQTT服务器
  if (mqttClient.connect(clientId.c_str(), mqttId, mqttPwd, willTopic, willQoS, willRetain, willMsg))
  {
    Serial.println("MQTT Server Connected.");
    Serial.println("Server Address:");
    Serial.println(mqttServer);
    Serial.println("ClientId:");
    Serial.println(clientId);
    publishOnlineStatus(); //发布在线状态
    //subscribeTopic();   //订阅指定主题
  }
  else
  {
    Serial.println("MQTT Server Connect Failed.Client State: ");
    Serial.println(mqttClient.state());
    delay(2000);
  }
}

//订阅MQTT消息
void subscribeTopic()
{
  //订阅获取RGB灯主题信息
  String rgbTopic = "mymqtt/rgb";
  char subRgbTopic[rgbTopic.length() + 1];
  strcpy(subRgbTopic, rgbTopic.c_str());

  // 通过串口监视器输出是否成功订阅主题以及订阅的主题名称
  if (mqttClient.subscribe(subRgbTopic))
  {
    Serial.println("Subscrib Topic:");
    Serial.println(subRgbTopic);
  }
  else
  {
    Serial.print("Subscribe Fail...");
  }
}

// 收到信息后的回调函数
void receiveCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message Received [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
  Serial.print("Message Length(Bytes) ");
  Serial.println(length);

  if ((char)payload[0] == '1')
  {
    digitalWrite(greenLed, LOW); // 则点亮LED。
    Serial.println("Green LED ON");
  }
  else if ((char)payload[0] == '2')
  {
    digitalWrite(redLed, LOW); // 否则熄灭LED。
    Serial.println("Red LED ON");
  }
  else if ((char)payload[0] == '3')
  {
    digitalWrite(redLed, HIGH); // 否则熄灭LED。
    Serial.println("Red LED OFF");
  }
  else if ((char)payload[0] == '0')
  {
    digitalWrite(greenLed, HIGH); // 否则熄灭LED。
    Serial.println("Green LED OFF");
  }
}

// 发布在线信息
void publishOnlineStatus()
{
  // 建立遗嘱主题。
  String willString = "Mymqtt/module1/willMessage";
  char willTopic[willString.length() + 1];
  strcpy(willTopic, willString.c_str());

  // 建立设备在线的消息。此信息将以保留形式向遗嘱主题发布
  String onlineMessageString = "CLIENT-ONLINE";
  char onlineMsg[onlineMessageString.length() + 1];
  strcpy(onlineMsg, onlineMessageString.c_str());

  // 向遗嘱主题发布设备在线消息
  if (mqttClient.publish(willTopic, onlineMsg, true))
  {
    Serial.print("Published Online Message: ");
    Serial.println(onlineMsg);
  }
  else
  {
    Serial.println("Online Message Publish Failed.");
  }
}

//发布MQTT信息
void pubMQTTmsg()
{

  //建立发布光照强度主题。主题名称为“light”，位于“mymqtt”主题下一级
  String lightTopic = "Mymqtt/module1/light";
  char publishLightTopic[lightTopic.length() + 1];
  strcpy(publishLightTopic, lightTopic.c_str());

  //建立发布光照强度信息。
  String lightMessage = (String)val1;
  char publishLightMsg[lightMessage.length() + 1];
  strcpy(publishLightMsg, lightMessage.c_str());

  //建立发布温湿度主题。主题名称为“DHT11”，位于“mymqtt”主题下一级
  String DHT11Topic = "Mymqtt/module1/DHT11";
  char publishDHT11Topic[DHT11Topic.length() + 1];
  strcpy(publishDHT11Topic, DHT11Topic.c_str());

  //建立发布温湿度信息。
  char json[100];
  String tem = (String)temp_read;
  String hum = (String)humi_read;
  String payload = "{";
  payload += "\"temperature\":";
  payload += tem;
  payload += ",";
  payload += "\"humidity\":";
  payload += hum;
  payload += "}";
  char attributes[100];
  payload.toCharArray(attributes, 100);
  //实现8266向主题发布信息
  if (mqttClient.publish(publishDHT11Topic, attributes) && mqttClient.publish(publishLightTopic, publishLightMsg))
  {
    //温湿度
    Serial.print("Publish DHT11Topic:");
    Serial.println(publishDHT11Topic);
    Serial.print("Publish DHT11Message:");
    Serial.println(attributes);

    //光照强度
    Serial.print("Publish lightTopic:");
    Serial.println(publishLightTopic);
    Serial.print("Publish lightMessage:");
    Serial.println(publishLightMsg);
    //DHT11Message = "";
  }
  else
  {
    Serial.println("Message Publish Failed.");
  }
}

//温湿度检测
void getHumChk()
{
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temp_read, &humi_read, NULL)) != SimpleDHTErrSuccess)
  {
    Serial.print("Read DHT11 failed, err=");
    Serial.println(err);
    delay(1500);
    return;
  }
  tft.setTextSize(2);
  tft.drawBitmap(0, 115, shi, 16, 16, TFT_WHITE);
  tft.drawBitmap(18, 115, du, 16, 16, TFT_WHITE);
  tft.setCursor(36, 115);
  tft.setTextColor(TFT_WHITE);
  tft.print(":");
  tft.fillRect(44, 115, 30, 20, TFT_BLACK);
  tft.setTextColor(TFT_RED);
  tft.print(humi_read);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(80, 115);
  tft.println("%");
  Serial.print("hum:");
  Serial.print(humi_read);
  Serial.print("%");

  Serial.print("tem:");
  Serial.print(temp_read);
  Serial.println("*C");
  tft.setTextSize(2);
  tft.drawBitmap(120, 115, wen, 16, 16, TFT_WHITE);
  tft.drawBitmap(138, 115, du, 16, 16, TFT_WHITE);
  tft.setCursor(156, 115);
  tft.print(":");
  tft.fillRect(164, 115, 30, 20, TFT_BLACK);
  tft.setTextColor(TFT_RED);
  tft.print(temp_read);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(200, 115);
  tft.println("C");
}

/* 微信智能配网 */
void smartConfig()
{
  WiFi.mode(WIFI_STA);                           //设置STA模式
  Serial.println("\r\nWait for Smartconfig..."); //打印log信息
  WiFi.beginSmartConfig();                       //开始SmartConfig，等待手机端发出用户名和密码
  while (1)
  {
    Serial.println(".");
    digitalWrite(LED_BUILTIN, HIGH); //指示灯闪烁
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW); //指示灯闪烁
    delay(1000);
    if (WiFi.smartConfigDone()) //配网成功，接收到SSID和密码
    {
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      break;
    }
  }
}

/*连接网络*/
void connectWiFi()
{
  drawBmp("/1.bmp", 0, 0);
  delay(2000);
  WiFi.mode(WIFI_STA); //设置STA模式
  WiFi.begin(ssid, password);
  Serial.println("\r\n正在连接WIFI...");
  while (WiFi.status() != WL_CONNECTED) //判断是否连接WIFI成功
  {
    if (WIFI_Status)
    {
      Serial.print(".");
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
      delay(500);
      count++;
      if (count >= 10)
      {
        WIFI_Status = false;
        Serial.println("WiFi连接失败，请用手机进行配网");
        tft.fillScreen(TFT_BLACK);
        tft.drawBitmap(2, 2, lian, 16, 16, TFT_RED);
        tft.drawBitmap(20, 2, jie, 16, 16, TFT_RED);
        tft.drawBitmap(38, 2, shi_bai, 16, 16, TFT_RED);
        tft.drawBitmap(56, 2, bai, 16, 16, TFT_RED);
        tft.drawBitmap(74, 2, douhao, 16, 16, TFT_RED);
        tft.drawBitmap(92, 2, qing, 16, 16, TFT_RED);
        tft.drawBitmap(110, 2, chong, 16, 16, TFT_RED);
        tft.drawBitmap(128, 2, xin, 16, 16, TFT_RED);
        tft.drawBitmap(146, 2, pei, 16, 16, TFT_RED);
        tft.drawBitmap(164, 2, wang, 16, 16, TFT_RED);
        tft.setCursor(184, 2);
        tft.setTextColor(TFT_RED);
        tft.print("...");
      }
    }
    else
    {
      smartConfig(); //微信智能配网
    }
  }
  Serial.println("连接成功");
  Serial.print("IP:");
  Serial.println(WiFi.localIP());
  tft.fillScreen(TFT_BLACK);
}

//检测光照强度
void getLight()
{
  val1 = analogRead(PIN_A);
  Serial.print("a:");
  Serial.print(val1);
  Serial.print(", d:");
  val2 = digitalRead(PIN_D);
  Serial.println(val2);
  if (val1 > 600)
  {
    digitalWrite(greenLed, HIGH);
  }
  else
  {
    digitalWrite(greenLed, LOW);
  }
  tft.drawBitmap(0, 95, guang, 16, 16, TFT_WHITE);
  tft.drawBitmap(18, 95, zhao, 16, 16, TFT_WHITE);
  tft.drawBitmap(36, 95, qiang, 16, 16, TFT_WHITE);
  tft.drawBitmap(54, 95, du, 16, 16, TFT_WHITE);
  tft.setCursor(71, 95);
  tft.setTextColor(TFT_WHITE);
  tft.print(":");
  tft.fillRect(80, 95, 50, 20, TFT_BLACK);
  tft.setTextColor(TFT_RED);
  tft.print(val1);
  delay(1000);
}
