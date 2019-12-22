#include <U8glib.h>
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE); 
#define setFont_L u8g.setFont(u8g_font_7x13)
#define setFont_M u8g.setFont(u8g_font_osb21)
#define setFont_S u8g.setFont(u8g_font_fixed_v0r)
#define setFont_SS u8g.setFont(u8g_font_04b_03bn)

//#define INTERVAL_LCD 20 
//unsigned long lcd_time = millis(); 
//#include<Microduino_ColorLED.h>
//ColorLED strip = ColorLED(2,4);

#define SSID        "111" 
#define PASSWORD    "88888888"
String apiKey = "KfFEQzwmmMLcInwXkIlfCiRqRLI=";

#define HOST_NAME   "api.heclouds.com"
#define HOST_PORT   (80)
#define INTERVAL_SENSOR   5000          
#define INTERVAL_NET      5000         
#include <Wire.h>                              
#include <ESP8266.h>
#include <I2Cdev.h>                               
#include <Microduino_SHT2x.h>
#define IDLE_TIMEOUT_MS  3000  
char buf[10];
#define DEVICEID "575384827"
#define INTERVAL_sensor 2000
unsigned long sensorlastTime = millis();
float tempOLED, humiOLED;
#define INTERVAL_OLED 1000
String mCottenData;
String jsonToSend;
String jsonToRegister;
float sensor_tem, sensor_hum;                     
char  sensor_tem_c[7], sensor_hum_c[7] ;
#include <SoftwareSerial.h>
#define EspSerial mySerial
#define UARTSPEED  9600
SoftwareSerial mySerial(2, 3);
ESP8266 wifi(&EspSerial);                                    
unsigned long net_time1 = millis();                         
unsigned long sensor_time = millis();                                 
String postString;                         
String getString;                  
String postRegister;
Tem_Hum_S2 TempMonitor;


void setup(void)    
{   
//    strip.begin();
//    strip.setBrightness(60);     
//    strip.show();
    Wire.begin();
    Serial.begin(9600);
    pinMode(4,OUTPUT);
    while (!Serial); // wait for Leonardo enumeration, others continue immediately
    Serial.print(F("setup begin\r\n"));
    delay(100);
    
  WifiInit(EspSerial, UARTSPEED);

  Serial.print(F("FW Version:"));
  Serial.println(wifi.getVersion().c_str());

  if (wifi.setOprToStationSoftAP()) {
    Serial.print(F("to station + softap ok\r\n"));
  } else {
    Serial.print(F("to station + softap err\r\n"));
  }

  if (wifi.joinAP(SSID, PASSWORD)) {
    Serial.print(F("Join AP success\r\n"));

    Serial.print(F("IP:"));
    Serial.println( wifi.getLocalIP().c_str());
  } else {
    Serial.print(F("Join AP failure\r\n"));
  }

  if (wifi.disableMUX()) {
    Serial.print(F("single ok\r\n"));
  } else {
    Serial.print(F("single err\r\n"));
  }

  Serial.print(F("setup end\r\n"));
     
}
void loop(void)    
{ 
  char* DEVICE_ID = new char[9]; 
  DEVICE_ID = getdevice();
  for(uint32_t i=0;i<9;i++)
  {
    Serial.print(DEVICEID[i]);  
  }
  u8g.firstPage();
  do {
 setFont_L;//选择字体
 u8g.setPrintPos(0, 10);//选择位置
 u8g.print("DEVICEID:");//打印字符串
 u8g.setPrintPos(0, 20);//选择位置
 u8g.print(DEVICEID);//打印字符串
 }while( u8g.nextPage() );
  if (sensor_time > millis())  sensor_time = millis();  
    
  if(millis() - sensor_time > INTERVAL_SENSOR)  
  {  
    getSensorData();                                  
    sensor_time = millis();
  }  


    
  if (net_time1 > millis())  net_time1 = millis();
  
  if (millis() - net_time1 > INTERVAL_NET)     
  {                                         
    downloadData();
    delay(3000);
    updateSensorData();
    net_time1 = millis();
  }
  
}

void getSensorData(){  
    sensor_tem = TempMonitor.getTemperature();  
    sensor_hum = TempMonitor.getHumidity();   
    //获取光照   
    delay(1000);
    dtostrf(sensor_tem, 2, 1, sensor_tem_c);
    dtostrf(sensor_hum, 2, 1, sensor_hum_c);
}
void updateSensorData() {
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接，如果失败，不能发送该数据
    Serial.print("create tcp ok\r\n");

jsonToSend="{\"Temperature\":";
    dtostrf(sensor_tem,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+=",\"Humidity\":";
    dtostrf(sensor_hum,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+="}";



    postString="POST /devices/";
    postString+=DEVICEID;
    postString+="/datapoints HTTP/1.1";
    postString+="\r\n";
    postString+="api-key:";
    postString+=apiKey;
    postString+="\r\n";
    postString+="Host:api.heclouds.com\r\n";
    postString+="Connection:close\r\n";
    postString+="Content-Length:";
    postString+=jsonToSend.length();
    postString+="\r\n";
    postString+="\r\n";
    postString+=jsonToSend;
    postString+="\r\n";
    postString+="\r\n";
    postString+="\r\n";
    
    
  const char *postArray = postString.c_str();                 //将str转化为char数组
  Serial.println(postArray);
  wifi.send((const uint8_t*)postArray, strlen(postArray));    //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)

  Serial.println("send success"); 
     if (wifi.releaseTCP()) {                                 //释放TCP连接
        Serial.print("release tcp ok\r\n");
        } 
     else {
        Serial.print("release tcp err\r\n");
        }
      postArray = NULL;                                       //清空数组，等待下次传输数据
  
  } else {
    Serial.print("create tcp err\r\n");
  }
}
char downloadData()
{
     if (wifi.createTCP(HOST_NAME, HOST_PORT)) 
  { 
    Serial.print("create tcp ok\r\n");
    getString = "GET /devices/";      
    getString += DEVICEID;  //设备ID号  
    getString +="/datastreams/Light "; 
    getString += "HTTP/1.1";  
    getString += "\r\n"; 
    getString += "api-key:";  
    getString += apiKey; 
    getString += "\r\n"; 
    getString +="Host:api.heclouds.com\r\n"; 
    getString +="Connection:close\r\n\r\n";

    const char *getArray =getString.c_str();
    wifi.send((const uint8_t*)getArray,strlen(getArray));
    Serial.println(F("Send Success!"));
    
    Serial.print("\r\n");
  char buffer[300];
  uint32_t len = wifi.recv(buffer,sizeof(buffer),1000);
  if(len>0)
  {
    Serial.print("Received:[");
    for(uint32_t i =0;i<len;i++)
    {
      Serial.print(buffer[i]);  
    }
    Serial.print("]"); 
    Serial.print("\r\n"); 
  }
  else
  {
    Serial.print("Recv failed");
    Serial.print("\r\n");  
  }
  for(uint32_t j=0;j<len;j++)
  {
    if(buffer[j]=='v'&&buffer[j+1]=='a'&&buffer[j+2]=='l')
    {
      Serial.print(buffer[j+8]);
      Serial.print("\r\n");
      if(buffer[j+8]=='2')
      {
         for (int i = 0; i < 2; i++) 
         {
//       strip.setPixelColor(i, strip.Color(255, 0, 0)); 
//       strip.show();
         }
       delay(1000); 
       }  
    } 
   }
  wifi.releaseTCP();
  if(wifi.releaseTCP())
  {
    Serial.print("release tcp ok");
    Serial.print("\r\n");  
  }
  else
  {
    Serial.print("release tcp err") ;
    Serial.print("\r\n");  
  }
    getArray = NULL;
    Serial.println(freeRam());
    while(true)break;
}
    else{
    Serial.print("create tcp failed\r\n");
    Serial.print("\r\n");
    }
}
int freeRam() { 
  extern int __heap_start, *__brkval;  
  int v;  
  return (int) &v - (__brkval == 0 ? (int)&__heap_start :(int) __brkval);
  }
  char* getdevice()
{
    char* id = new char[9];
    char* inf = new char[200];
    if (wifi.createTCP(HOST_NAME, HOST_PORT)) 
    { 
    Serial.print("create tcp ok\r\n");
    jsonToRegister="{\"sn\":";
    jsonToRegister+="\"422\"";
    jsonToRegister+=",\"title\":";
    jsonToRegister+="\"hanger\"}";
    postRegister="POST /register_de?register_code=93Hejj9zQF3FtqPy HTTP/1.1";
    postRegister+="\r\n";
    postRegister+="Host:api.heclouds.com\r\n";
    postRegister+="Connection:keep-alive\r\n";
    postRegister+="Content-Length:";
    postRegister+=jsonToRegister.length();
    postRegister+="\r\n";
    postRegister+="\r\n";
    postRegister+=jsonToRegister;
    postRegister+="\r\n";
    postRegister+="\r\n";
    postRegister+="\r\n";
    const char *postArray = postRegister.c_str(); 
    Serial.println(postArray);
    wifi.send((const uint8_t*)postArray, strlen(postArray));
    Serial.println("send success");
     uint32_t len = wifi.recv(inf,sizeof(inf),1000);
     if(len>0)
     {
     Serial.print("Received:[");
     for(uint32_t i =0;i<len;i++)
     {
      Serial.print(inf[i]);  
     }
     Serial.print("]"); 
     Serial.print("\r\n"); 
     }
    else
    {
    Serial.print("Recv failed");
    Serial.print("\r\n");  
    }
    for(uint32_t i=0;i<len;i++)
    {
      if(inf[i]=='d'&&inf[i+1]=='e'&&inf[i+2]=='v'&&inf[i+3]=='i')
      {
        Serial.print("\r\n");
        for(uint32_t j=0;j<9;j++)
        {
          id[j]=inf[i+j+12];  
        }  
      }  
    }
    return id;
    if (wifi.releaseTCP()) 
    { 
    Serial.print("release tcp ok\r\n");
    } 
    else 
    {
    Serial.print("release tcp err\r\n");
    }
    postArray = NULL;
    } 
    else 
    {
    Serial.print("create tcp err\r\n");
    }
    delay(3000);
    delete[]id;
    delete[]inf;
}

