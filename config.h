// copy this file to wifikeys.h and edit
const char *ssid =     "Kanuengnit_kiss2G";         // Put your SSID here
const char *password = "3213213213";         // Put your PASSWORD here
char auth[] = "UjMtGyuOL3RQJxQC62njHuZjmw-B7TqG";
#define BLYNK_PRINT Serial

IPAddress local_IP(192, 168, 1, 44);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 254, 0);

const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).

typedef enum {
  wLOWEST=0,
  wLOW,
  wMIDDLE,
  wHIGH,
  wHIGHEST,
  wAUTO
}wing_ac_en;


typedef enum {
  fMIN=0,
  fMED,
  fMAX,
  fAUTO
}fan_ac_en;


int temp_ac = 25; // 
wing_ac_en wing_ac = wAUTO;   // 0=highest, 1=high, 2=middle, 3=low, 4=lowest, 5=auto
fan_ac_en  fan_ac  = fAUTO;   // 0=min, 1=med, 2=max, 3=auto
bool quiet_ac = false;

typedef enum{
  REQ_NONE =0,
  REQ_ON_AC,
  REQ_OFF_AC,
  REQ_TEMP_AC,
  REQ_WING_AC,
  REQ_FAN_AC,
  REQ_QUIET_AC
}req_en;

req_en req;
int retry = 0;
unsigned long last_retry=0;
unsigned long last_report=0;
