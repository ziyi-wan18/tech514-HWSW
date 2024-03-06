#include <Arduino.h>

// 将以下引脚编号替换为对应ESP32-S3的实际GPIO引脚编号
const int soilMoistureSensorPin = D0;  // 土壤湿度传感器连接的GPIO引脚，D0引脚对应的GPIO编号
const int lightSensorPin = D10;         // 光敏电阻连接的GPIO引脚，D10引脚对应的GPIO编号

void setup() {
  Serial.begin(115200); // 初始化串口通信，设置波特率为115200
  
  // 配置传感器引脚为输入模式
  pinMode(soilMoistureSensorPin, INPUT);
  pinMode(lightSensorPin, INPUT);
}

void loop() {
  // 读取土壤湿度传感器的值（模拟输入）
  int soilMoistureValue = analogRead(soilMoistureSensorPin);
  
  // 读取光敏电阻的值（模拟输入）
  int lightSensorValue = analogRead(lightSensorPin);
  
  // 通过串口输出读取的值
  Serial.print("Soil Moisture Value: ");
  Serial.print(soilMoistureValue);
  Serial.print(" | Light Sensor Value: ");
  Serial.println(lightSensorValue);

  delay(2000); // 等待2秒钟再次读取
}
