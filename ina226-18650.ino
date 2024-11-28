#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "INA226.h"
//2024年11月28日14:12:28 3.69万用表
INA226 INA(0x40);  // INA226 传感器的 I2C 地址

float batteryEnergy_mWh = 0.0;  // 电池能量（单位：mWh）

float batteryxiuzheng_mWh = 0.0;  // 修正电池能量（单位：mWh）
unsigned long lastMillis = 0;   // 用于计算每秒更新的定时器

// WiFi 配置信息
const char* ssid = "ChinaNet-ggzf";
const char* password = "jlkjxlav";

// 创建WebServer对象，监听端口80
WebServer server(80);

// 上次的电池数据
String lastBatteryData = "";

void setup() {
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("INA226_LIB_VERSION: ");
  Serial.println(INA226_LIB_VERSION);

  Serial.println("Starting setup...");

  Wire.begin(8, 9);  // sda，scl给电池的电压电流检测使用

  Serial.println("I2C Initialized");

  if (!INA.begin()) {
    Serial.println("Could not connect. Fix and Reboot");
    while (1);  // 如果初始化失败，停在这里
  }

  Serial.println("INA226 initialized successfully.");

  // 设置最大电流和分流电阻
  INA.setMaxCurrentShunt(0.5, 0.1);    // 设置最大电流为 0.5 A，分流电阻 100 mΩ

  // 初始化WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // 设置处理HTTP请求的回调函数
  server.on("/", HTTP_GET, handleRoot);
  
  // 启动Web服务器
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  // 获取电池电压、电流、功率
  float busVoltage = INA.getBusVoltage();  // 获取总电压（单位：伏特）
  float shuntVoltage = INA.getShuntVoltage_mV();  // 获取分流电压（单位：毫伏）
  float current = INA.getCurrent_mA();  // 获取电流（单位：毫安）
  float power = INA.getPower_mW();  // 获取功率（单位：毫瓦）

   

  // 计算消耗的能量，单位：mWh
  batteryEnergy_mWh += (power * 1.0 / 3600.0);  // 每秒的功率


  float wybVoltage = INA.getBusVoltage()+0.284;  // 万用表实测电压修正
  float jsgonglv = wybVoltage*current;  // 计算出的当前功率
   batteryxiuzheng_mWh += (jsgonglv * 1.0 / 3600.0);  // 每秒的功率

  // 计算ESP32运行的天数、小时和分钟
  unsigned long uptime = millis();  // 获取ESP32开机时间（毫秒）
  unsigned long days = uptime / (1000 * 60 * 60 * 24);
  unsigned long hours = (uptime % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60);
  unsigned long minutes = (uptime % (1000 * 60 * 60)) / (1000 * 60);

  // 将电池数据和运行时间保存到字符串中，供Web页面显示
  lastBatteryData = String("<h1>电池状态</h1>") +
                    "<p>电池电压: " + String(busVoltage, 3) + " V</p>" +
                    "<p>分流电压: " + String(shuntVoltage, 3) + " mV</p>" +
                    "<p>电流: " + String(current / 1000.0, 3) + " A</p>" +
                    "<p>功率: " + String(power, 3) + " mW</p>" +
                    "<p>计算的电流: " + String(shuntVoltage / 100.0, 3) + " A</p>" +
                    "<p>已消耗能量: " + String(batteryEnergy_mWh, 6) + " mWh</p> " +
                    "<p> </p> " +
                    "<p>修正电压万用表: " + String(wybVoltage, 3) + " V</p>" +
                    "<p>修正功率: " + String(jsgonglv, 3) + " V</p>" +
                    "<p>修正已消耗能量: " + String(batteryxiuzheng_mWh, 6) + " mWh</p>" +
                    "<p>设备运行时间: " + String(days) + " 天 " + String(hours) + " 小时 " + String(minutes) + " 分钟</p>";



  // 输出电池的各项数据
  Serial.println("\nBUS\tSHUNT\tCURRENT\tPOWER\tENERGY");
  Serial.print("电池电压: ");
  Serial.print(busVoltage, 3);
  Serial.println(" V");

  Serial.print("分流电压: ");
  Serial.print(shuntVoltage, 3);
  Serial.println(" mV");

  Serial.print("电流: ");
  Serial.print(current / 1000.0, 3);  // 电流单位转换为 A
  Serial.println(" A");

  Serial.print("功率: ");
  Serial.print(power, 3);  // 直接显示毫瓦
  Serial.println(" mW");

  // 输出分流电压对应的电流
  float calculatedCurrent = shuntVoltage / 100.0;  // 计算电流 I = V / R
  Serial.print("计算的电流: ");
  Serial.print(calculatedCurrent, 3);
  Serial.println(" A");

  // 输出已消耗能量（mWh）
  Serial.print("已消耗能量: ");
  Serial.print(batteryEnergy_mWh, 6);
  Serial.println(" mWh");

  delay(1000);  // 每秒更新一次

  // 处理客户端请求
  server.handleClient();
}

// Web页面处理函数
void handleRoot() {
  // 设置Content-Type为text/html并指定字符编码为UTF-8
  server.send(200, "text/html; charset=UTF-8", lastBatteryData);
}
