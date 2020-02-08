/**
 * www.arduinona.com
 * ส่งสัญญาณรีโมทไปยังแอร์ด้วย ESP8266 และ blynk
 */


#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Panasonic.h>
#include <BlynkSimpleEsp8266.h>

#include "config.h"

BLYNK_WRITE(V1)
{
  if(param.asInt()) {
    Serial.println("Req on recv");
    req = REQ_ON_AC;
  }
}
BLYNK_WRITE(V2)
{
  if(param.asInt()) {
    Serial.println("Req off recv");
    req = REQ_OFF_AC;
  }
}
BLYNK_WRITE(V4)
{
  req = REQ_WING_AC;
  wing_ac = (wing_ac_en)param.asInt();
  Serial.println("Set wing " + String(param.asString()));
}
BLYNK_WRITE(V3)
{
  req = REQ_FAN_AC;
  fan_ac = (fan_ac_en)param.asInt();
}
BLYNK_WRITE(V5)
{
  Serial.println("Set fan auto");
  req = REQ_FAN_AC;
  fan_ac = fAUTO;
}
BLYNK_WRITE(V6)
{
  Serial.println("Set wing auto");
  req = REQ_WING_AC;
  wing_ac = wAUTO;
}
BLYNK_WRITE(V0)
{
  temp_ac = param.asInt();
  Serial.println("Req temp = " + String(temp_ac));
  req = REQ_TEMP_AC;
}

BLYNK_WRITE(V7)
{
  quiet_ac = param.asInt();
  Serial.println("Req quiet ac = "+String(quiet_ac));
  req = REQ_QUIET_AC;
}

BlynkTimer timer;  //เรียกใช้การตั้งเวลาของ Blynk




IRPanasonicAc ac(kIrLed);  // Set the GPIO used for sending messages.

void printState() {
  // Display the settings.
  Serial.println("Panasonic A/C remote is in the following state:");
  Serial.printf("  %s\n", ac.toString().c_str());
  // Display the encoded IR sequence.
  unsigned char* ir_code = ac.getRaw();
  Serial.print("IR Code: 0x");
  for (uint8_t i = 0; i < kPanasonicAcStateLength; i++)
    Serial.printf("%02X", ir_code[i]);
  Serial.println();
}

void setup() {
  WiFi.mode(WIFI_STA);
//    WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
      delay(500);
      Serial.print(F("."));
  }



  Blynk.config(auth);
  

  timer.setInterval(30000L, reconnecting);  //Function reconnect

  //ตั้งเวลาส่งข้อมูลให้ Blynk Server ทุกๆ 30 วินาที
  reconnecting();
    

  IPAddress ip = WiFi.localIP();
  Serial.println(F("WiFi connected"));
  Serial.println("");
  Serial.println(ip);



    
  ac.begin();
  Serial.begin(115200);
  delay(200);

  // Set up what we want to send. See ir_Panasonic.cpp for all the options.
  Serial.println("Default state of the remote.");
  printState();
  Serial.println("Setting desired state for A/C.");
  ac.setModel(kPanasonicRkr);
  ac.on();
  ac.setFan(kPanasonicAcFanAuto);
  ac.setMode(kPanasonicAcCool);
  ac.setTemp(temp_ac);
  ac.setSwingVertical(kPanasonicAcSwingVAuto);
  ac.setSwingHorizontal(kPanasonicAcSwingHAuto);
}

void loop() {



  /**
   * จำแนกข้อความที่รีเควสมา
   */
    
      switch(req) {
        case REQ_NONE: break;
        case REQ_ON_AC:   ac.on();   break;
        case REQ_OFF_AC:  ac.off();  break;
        case REQ_TEMP_AC: ac.setTemp(temp_ac); ac.on(); break;
        case REQ_WING_AC: {
          switch(wing_ac) {
            case wHIGHEST : ac.setSwingVertical(kPanasonicAcSwingVHighest );break;
            case wHIGH :    ac.setSwingVertical(kPanasonicAcSwingVHigh );   break;
            case wMIDDLE :  ac.setSwingVertical(kPanasonicAcSwingVMiddle ); break;
            case wLOW :     ac.setSwingVertical(kPanasonicAcSwingVLow );    break;
            case wLOWEST :  ac.setSwingVertical(kPanasonicAcSwingVLowest ); break;
            case wAUTO :    ac.setSwingVertical(kPanasonicAcSwingVAuto);    break;
          }
          break;
        }
        case REQ_FAN_AC: {
          switch(fan_ac) {
            case fMIN :  ac.setFan(kPanasonicAcFanMin  ); break;
            case fMED :  ac.setFan(kPanasonicAcFanMed  ); break;
            case fMAX :  ac.setFan(kPanasonicAcFanMax  ); break;
            case fAUTO : ac.setFan(kPanasonicAcFanAuto  );break;
          }
          break;
        }
        case REQ_QUIET_AC:  ac.setQuiet(quiet_ac); break; 
      }


  /**
   * ส่งข้อมูลออกไปเมื่อมีการรีเควสจากมือถือ
   */
      if(millis() - last_retry > 500 && req!=REQ_NONE ) {
        last_retry = millis();
        Serial.println("Sending IR command to A/C ...");
        ac.send();
        printState();
        retry ++;
        if(retry >= 2) {
          retry = 0;
          req = REQ_NONE;
        }
      }


  /**
   * ส่งข้อมูลกลับไปยัง blynk server ให้แสดงสถานะค่าล่าสุดว่าเป็นอย่างไร
   */
      if(millis() - last_report > 2000) {
        last_report = millis();

        static bool heartbeat = false;
        Blynk.virtualWrite(0, temp_ac);  
        Blynk.virtualWrite(1, ac.getPower());  
        Blynk.virtualWrite(2, !ac.getPower());  
        if(fan_ac != fAUTO)  Blynk.virtualWrite(3, (int)fan_ac); 
        if(wing_ac != wAUTO) Blynk.virtualWrite(4, (int)wing_ac); 
        Blynk.virtualWrite(5, fan_ac == fAUTO); 
        Blynk.virtualWrite(6, wing_ac == wAUTO);  
        Blynk.virtualWrite(8, temp_ac);  
        Blynk.virtualWrite(9, heartbeat);  
        heartbeat = !heartbeat;
      }


  /**
   * อัพเดทเวลาของ timer blynk
   */
      timer.run();

  /**
   * เช็คว่า Server ทำงานอยู่หรือไม่ ถ้าใช่ก็ให้อัพเดทค่า
   */
      if (Blynk.connected())
      {
        Blynk.run();
      } else {
        delay(200);
        Serial.println("Offlne");
      }
  
}



void reconnecting()
{
  static int blynkIsDownCount = 0;
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
  }
}
