#include <OneWire.h>  
#include <DallasTemperature.h>  
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
    
//DS18B20 온도 센서의 데이터선인 가운데 핀을 아두이노 3번에 연결합니다.   
#define ONE_WIRE_BUS 3  

// 팬이 동작할 온도
#define FAN_ON_TEMP 28
#define FAN_OFF_TEMP 27

// Relay 연결 핀 D5
#define RELAY_PIN 5
    
//1-wire 디바이스와 통신하기 위한 준비  
OneWire oneWire(ONE_WIRE_BUS);  
    
// oneWire선언한 것을 sensors 선언시 참조함.  
DallasTemperature sensors(&oneWire);  
    
//다비아스 주소를 저장할 배열 선언  
DeviceAddress insideThermometer;  

// OLED display TWI address
#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(-1);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

int fan = 0;

void setup(void)  
{  
  //시리얼 포트 초기화  
  Serial.begin(9600);  
  Serial.println("Dallas Temperature IC Control Library Demo");  
    
  //1-wire 버스 초기화  
  sensors.begin();  

  //Relay 연결 핀 초기화
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
      
  //발견한 디바이스 갯수  
  Serial.print("Found ");  
  Serial.print(sensors.getDeviceCount(), DEC);  
  Serial.println(" devices.");  
    
  // parasite power 모드일 때에는  2핀(GND와 DQ 핀)만 연결하면 됨.  
  Serial.print("Parasite power is: ");   
  if (sensors.isParasitePowerMode()) Serial.println("ON");  
  else Serial.println("OFF");  
      
     
  //버스에서 첫번째 장치의 주소를 가져온다.  
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");   
      
  //버스에서 발견한 첫번째 장치의 주소 출력  
  Serial.print("Device 0 Address: ");  
  printAddress(insideThermometer);  
  Serial.println();  
    
  //데이터시트에서 확인결과 9~12비트까지 설정 가능  
  sensors.setResolution(insideThermometer, 10);  
     
  Serial.print("Device 0 Resolution: ");  
  Serial.print(sensors.getResolution(insideThermometer), DEC);   
  Serial.println();

  // initialize and clear display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();

  // display a pixel in each corner of the screen
  display.drawPixel(0, 0, WHITE);
  display.drawPixel(127, 0, WHITE);
  display.drawPixel(0, 63, WHITE);
  display.drawPixel(127, 63, WHITE);

  // display a line of text
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(27,30);
}  
    
// 온도를 출력하는 함수  
void printTemperature(DeviceAddress deviceAddress)  
{  
  display.clearDisplay();
  //섭씨 온도를 가져옴  
  float  tempC = sensors.getTempC(deviceAddress);  
      
  Serial.print("Temp C: ");  
  Serial.println(tempC);
  
  //Serial.print(" Temp F: ");  
      
  //화씨 온도로 변환  
  //Serial.println(DallasTemperature::toFahrenheit(tempC));   

  display.setCursor(27,30);
  display.print(tempC);
  display.print(" C");
  display.display();
}
      
//디바이스 주소를 출력하는 함수  
void printAddress(DeviceAddress deviceAddress)  
{  
  for (uint8_t i = 0; i < 8; i++)  
  {  
    if (deviceAddress[i] < 16) Serial.print("0");  
        Serial.print(deviceAddress[i], HEX);  
  }  
}

void controlRelay(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
#if 0
  if ( tempC >= FAN_ON_TEMP )  {
    if (fan == 0) {
      Serial.println("FAN ON");
      digitalWrite(RELAY_PIN, HIGH);
      fan = 1;
    }
  } else {
    if (fan == 1) {
      Serial.println("FAN OFF");
      digitalWrite(RELAY_PIN, LOW);
      fan = 0;
    }
  }
#endif

  if (fan == 0 && tempC >= FAN_ON_TEMP) {
    Serial.println("FAN ON");
    digitalWrite(RELAY_PIN, HIGH);
    fan = 1;
  } else if (fan == 1 && tempC <= FAN_OFF_TEMP) {
    Serial.println("FAN OFF");
    digitalWrite(RELAY_PIN, LOW);
    fan = 0;
  }
}
    
void loop(void)  
{   
  Serial.print("Requesting temperatures...");  
  //sensors.requestTemperatures();   //연결되어 있는 전체 센서의 온도 값을 읽어옴  
  sensors.requestTemperaturesByIndex(0); //첫번째 센서의 온도값 읽어옴  
  Serial.println("DONE");  
     
  //센서에서 읽어온 온도를 출력  
  printTemperature(insideThermometer);
  controlRelay(insideThermometer);
  
  delay(1000);  
}
