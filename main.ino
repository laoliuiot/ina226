#include "INA226.h"
//大部分代码是各平台生成的，使用arduion 2.0.3 使用INA226 0.6.0 库，这个gpt生成的很多代码不通用这个库版本注意了。
//测量出来的结果比万用表低0.3v，不知道怎么办，先就这样了
INA226 INA(0x40);  // INA226 传感器的 I2C 地址

float batteryEnergy_mWh = 0.0;  // 电池能量（单位：mWh）
unsigned long lastMillis = 0;   // 用于计算每秒更新的定时器

void setup()
{
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
}

void loop()
{
  // 获取电池电压、电流、功率
  float busVoltage = INA.getBusVoltage();  // 获取总电压（单位：伏特）
  float shuntVoltage = INA.getShuntVoltage_mV();  // 获取分流电压（单位：毫伏）
  float current = INA.getCurrent_mA();  // 获取电流（单位：毫安）
  float power = INA.getPower_mW();  // 获取功率（单位：毫瓦）

  // 计算消耗的能量，单位：mWh
  batteryEnergy_mWh += (power * 1.0 / 3600.0);  // 每秒的功率

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
}
