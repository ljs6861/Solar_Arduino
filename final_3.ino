
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2); // i2c address 0x3F
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#define USE_SERIAL Serial
#define DS3231_I2C_ADDRESS 104


WiFiServer server(80); //와이파이 포트설정

int B = 2 ;   // D9 (LED)
int G = 15;   // D10 (LED)
int R = 13;   // D11 (LED)

//-------모터------------

int stp1 = 14; //D5 1번모터 회전각 1번에 1.8'
int dir1 = 16;  //D2 1번모터 방향 HIGH 오른쪽 LOW 왼쪽
int stp2 = 0; //D8
int dir2 = 12; //D6

int a = 0;     //  gen counter
//검 스텝모터 드라이버 꼽는순서 위에서 부터
//초
//빨
//파
/* gt2 타이밍 벨트는 톱니간 간격이 2mm 이고, 타이밍 벨트 풀리는 이빨이 20개이므로
   마이크로스텝을 쓰지 않을 때 1회전 = 200스텝 = 20이빨 = 40mm 이동하게 된다.
   1스텝 = 0.2mm 이므로
   1/2 마이크로스텝을 적용하면 1스텝당 0.1mm 씩 움직인다.  */

//---------------

byte tMSB, tLSB;
byte seconds, minutes, hours, day, date, month, year;
char weekDay[4];
float temp3231;
int reqNotCount = 0 ; // 서버 전송에 실패한 시각 카운트

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 5 * 1000; // 업데이트 시간 설정, 5(*1000) 초

bool showdot = false;

String HTTP_SERVER = "http://192.168.43.85:3000/"; //전송할 http주소
//String HTTP_SERVER = "http://192.168.0.35:3000/"; //전송할 http주소

//String HTTP_SERVER = "http://192.168.1.5:3000/"; //전송할 http주소

ESP8266WiFiMulti WiFiMulti;

void setup() {


  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);

  pinMode(stp1, OUTPUT);
  pinMode(dir1, OUTPUT);
  pinMode(stp2, OUTPUT);
  pinMode(dir2, OUTPUT);
  Serial.begin(115200);
  USE_SERIAL.begin(115200);

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }


    WiFiMulti.addAP("AndroidHotspot6861", "987654321"); //와이파이 자동연결
  //WiFiMulti.addAP("BITSW", "bitcocom0228");
  //      WiFiMulti.addAP("SOLAR", "987654321");

  server.begin();//와이파이 서버 시작

}

String response = "";
String response2 = "";
String reqString = "";



void loop() {
  int value1 = analogRead(A0); //태양광 패널 전압값을 아날로그로 측정

  float val =  float(value1 / 1023.0 * 5.0) ; //아날로그값을 전압으로 변환

  USE_SERIAL.print(val);
  USE_SERIAL.print("\n");

  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("    ");
  lcd.print(val);       //전압값 lcd로 출력
  lcd.print(" v     ");




  int curSecond = String(seconds, DEC).toInt();
  //
  if (millis() > lastConnectionTime + postingInterval) {
    //
    get3231Date();
    //    // 시간변수로 부터 시간 값 가져와서 문자열로 변환
    String pDate = "20" + String(year, DEC) + "-"  ;

    // 월, 일 값이 1자리 일 경우 앞에 0 첨부
    String tmpString = String(month, DEC);
    pDate += (tmpString.length() < 2 ? "0" + tmpString : tmpString) + "-" ;

    tmpString = String(date, DEC);
    pDate += (tmpString.length() < 2 ? "0" + tmpString : tmpString) ;

    tmpString = String(hours, DEC);
    String pTime = (tmpString.length() < 2 ? "0" + tmpString : tmpString) + ":";

    tmpString = String(minutes, DEC);
    pTime += (tmpString.length() < 2 ? "0" + tmpString : tmpString) + ":"  ;

    tmpString = String(seconds, DEC);
    pTime += (tmpString.length() < 2 ? "0" + tmpString : tmpString) ;


    if ((WiFiMulti.run() == WL_CONNECTED)) { //와이파이 통신부분


      HTTPClient http;

      http.begin("http://api.openweathermap.org/data/2.5/weather?q=gwangju&APPID=83c62b5028f8bad78c5410a0efae908f");//날씨 api 파싱


      int httpCode = http.GET();


      if (httpCode > 0) {


        if (httpCode == HTTP_CODE_OK) {

          String payload = http.getString();
          //USE_SERIAL.println(payload);


          String payload2 = jsonParser2(payload, "sunrise\"",   ",\"name");
          String payload3 = jsonParser2(payload, "sunrise\"",   ",\"name");

          String sun1 = jsonParser2(payload2, "sunrise\"",   ",\"sunset");
          // USE_SERIAL.println(sun1);
          // 1509494400

          String sun2 = jsonParser3(payload3, "sunset\"",   ",\"id");
          // USE_SERIAL.println(sun2);

          int sunR = sun1.toInt();
          int sunS = sun2.toInt();
          //USE_SERIAL.println(sunR);
          //USE_SERIAL.println(sunS);

          USE_SERIAL.println("광주");

          int today1 = (sunR % 86400);
          int sunR_hour = (today1 / 3600) - 15;
          USE_SERIAL.print("일출:");
          USE_SERIAL.print(sunR_hour);
          USE_SERIAL.print("시");
          int sunR_min = (today1 % 3600) / 60;
          USE_SERIAL.print(sunR_min);
          USE_SERIAL.println("분");


          int today2 = (sunS % 86400);
          int sunS_hour = (today2 / 3600) + 9;
          USE_SERIAL.print("일몰:");
          USE_SERIAL.print(sunS_hour);
          USE_SERIAL.print("시");
          int sunS_min = (today2 % 3600) / 60;
          USE_SERIAL.print(sunS_min);
          USE_SERIAL.println("분");


          USE_SERIAL.print("현재 시각:");
          USE_SERIAL.print(hours);
          USE_SERIAL.print("시");
          USE_SERIAL.print(minutes);
          USE_SERIAL.println("분");

            //날씨
          String weather = jsonParser1(payload, "main\"",   ",\"description");
          USE_SERIAL.print("날씨: ");
          USE_SERIAL.println(weather);
          
            //온도
            String tmp = jsonParser4(payload, "temp\"",   ",\"pressure");
            int temp = tmp.toInt()-273;


            USE_SERIAL.print("온도: ");
          USE_SERIAL.println(temp);

          



          if ( sunR_hour <= hours  &&  hours <= sunS_hour ) {
            USE_SERIAL.println("낮입니다.");

            // 데이터를 서버로 보내고
            String request = HTTP_SERVER + "solar/Day/"
                             + weather
                             + "/"
                             + temp
                             + "/"
                             + pDate
                             + "/"
                             + pTime
                             + "/"
                             + String(val);

            http_Send(request);
            http_Send(String(val));


          } else {

            USE_SERIAL.println("밤입니다.");

            // 데이터를 서버로 보내고
            String request = HTTP_SERVER + "solar/Night/"
                             + weather
                             + "/"
                             + pDate
                             + "/"
                             + pTime
                             + "/"
                             + String(val);

            http_Send(request);
            http_Send(String(val));
          }

          delay(3500);


        }
        USE_SERIAL.println(WiFi.localIP());//아두이노 ip확인 포트포워드 ip에 써주면 됨

        http.end();
        //
        //
      }

      //
    }
    client();

    //----모터--------------------------------------------------------------------------------



  }


  if (val < 4.0) {


    step_1cycle();


  } else {


    analogWrite(B, 255);
    analogWrite(R, 0);

  }
  delay(5000);


}

//
void http_Send(String req) {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    HTTPClient http;
    USE_SERIAL.print("[HTTP] begin...\n");

    USE_SERIAL.println(req);
    http.begin(req); //HTTP

    USE_SERIAL.print("[HTTP] GET...\n");

    int httpCode = http.GET();
    USE_SERIAL.println(httpCode);

    http.end();
  }

}

//------------------------------------------------------
void step1_low() { //아래로 이동
  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("Low Moving");
  step2(HIGH, 27500);


}
void step1_lowclean() { //아래로 이동하면서 청소

  step(HIGH, 27500);

  delay(400);

}

void step1_high() { //위로 이동
  step2(LOW, 27500);
}

void step_stop() { //정지
  digitalWrite(stp1, LOW);
  digitalWrite(stp2, LOW);
}

void step_1cycle() { //위로 이동후 아래로 이동하면서 청소
  analogWrite(B, 0);
  analogWrite(R, 255);

  step(HIGH, 27500);
  delay(400);
  int value1 = analogRead(A0);
  float val =  float(value1 / 1023.0 * 5.0) ;

  lcd.setCursor(2, 1);//청소 후 발전량 표시
  lcd.print("-> ");
  lcd.print(val);
  lcd.print(" v     ");

  step1_high();

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("    ");
  lcd.print(val);
  lcd.print(" v     ");
}


void step(boolean dir, int steps)
{
  digitalWrite(dir1, dir);//모터 Y축 방향
  digitalWrite(dir2, HIGH);//모터 X축 방향 high 왼쪽으로 돌아감 아래쪽 내려가는거
  delay(50);
  lcd.setCursor(2, 1);
  lcd.print("Cleanning...");
  for (int i = 0; i < steps; i++) {
    digitalWrite(stp1, HIGH);
    digitalWrite(stp2, HIGH);
    delay(1);
    digitalWrite(stp1, LOW);
    digitalWrite(stp2, LOW);
    delay(1);
  }
}

void step2(boolean dir, int steps)
{
  digitalWrite(dir1, dir);
  delay(50);

  for (int i = 0; i < steps; i++) {
    digitalWrite(stp1, HIGH);
    delay(1);
    digitalWrite(stp1, LOW);
    delay(1);
  }
}

void client() {

  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();


  // Match the request
  int val;
  if (req.indexOf("/?pin=12") != -1) {
    step_1cycle();
  } else {
    Serial.println("invalid request");
    client.stop();
    return;
  }

  // Set GPIO2 according to the request
  //  digitalWrite(16, val);

  client.flush();
  delay(1);

}


//날씨
String jsonParser1(String payloadStr, String beforeStr, String afterStr) {

  return payloadStr.substring(payloadStr.indexOf(beforeStr) + 7, payloadStr.indexOf(afterStr) - 1);

}


//일출,일몰
String jsonParser2(String payloadStr, String beforeStr, String afterStr) {

  return payloadStr.substring(payloadStr.indexOf(beforeStr) + 5, payloadStr.indexOf(afterStr));

}

//일출, 일몰
String jsonParser3(String payloadStr, String beforeStr, String afterStr) {

  return payloadStr.substring(payloadStr.indexOf(beforeStr) + 8,  payloadStr.indexOf(afterStr) - 1);

}

//온도
String jsonParser4(String payloadStr, String beforeStr, String afterStr) {

  return payloadStr.substring(payloadStr.indexOf(beforeStr)+6,  payloadStr.indexOf(afterStr)-3);

}






