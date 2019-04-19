/*
 * M5Stack_IRcube v0.0.1 
 *
 */
#include <M5Stack.h>
#include <M5StackUpdater.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <WiFiManager.h> // https://github.com/Brunez3BD/WIFIMANAGER-ESP32
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <aJSON.h>       // https://github.com/interactive-matter/aJson  https://gitlab.com/xarduino/lightsw/blob/master/patch/ajson-void-flush.patch
#include <IRremote.h>    // https://github.com/SensorsIot/Arduino-IRremote
#include <M5TreeView.h>  // https://github.com/lovyan03/M5Stack_TreeView
#include <Preferences.h>

M5TreeView treeView; //TreeView
typedef std::vector<MenuItem*> vmi;
String ipAddress;

WebServer webServer(80);
byte SEND_PIN = 21; // GRAYのGNDの隣：21、I2C GROVE IR：21   
IRsend irsend(SEND_PIN);
int deviceNumberTV = 0;
Preferences preferences;

//IRデータ処理 ---------------------------------------
#define IR_DATA "[{\"name\":\"TOSHIBA\", \"format\":\"nec\", \"data\":{\"custom\":\"02\",\"pow\":[\"48\"],\"cn\":[\"D8\",\"F8\"],\"vo\":[\"58\",\"78\"],\"mu\":[\"08\"],\"inputSel\":[\"F0\"]}},{\"name\":\"HITACHI\", \"format\":\"nec\", \"data\":{\"custom\":\"0A\",\"pow\":[\"E8\"], \"cn\":[\"98\",\"18\"], \"vo\":[\"48\",\"A8\"], \"mu\":[\"D0\"], \"inputSel\":[\"90\"] } },{\"name\":\"NEC\", \"format\":\"nec\", \"data\":{\"custom\":\"18\",\"pow\":[\"10\"], \"cn\":[\"00\",\"80\"], \"vo\":[\"40\",\"C0\"], \"mu\":[\"90\"], \"inputSel\":[\"50\"] } },{\"name\":\"Panasonic\", \"format\":\"aeha\", \"data\":{\"custom\":\"40040100\",\"pow\":[\"BCBD\"], \"cn\":[\"2C2D\",\"ACAD\"], \"vo\":[\"0405\",\"8485\"], \"mu\":[\"9C9D\"] , \"inputSel\":[\"A0A1\"] } },{\"name\":\"PIONEER\", \"format\":\"nec\", \"data\":{\"custom\":\"55\",\"pow\":[\"38\"], \"cn\":[\"08\",\"88\"], \"vo\":[\"50\",\"D0\"], \"mu\":[\"92\"], \"inputSel\":[\"C8\"]} },{\"name\":\"SHARP\", \"format\":\"aeha\", \"data\":{\"custom\":\"555AF148\",\"pow\":[\"688B\"], \"cn\":[\"8885\",\"4889\"], \"vo\":[\"288F\",\"A887\"], \"mu\":[\"E883\"] , \"inputSel\":[\"C881\"] } },{\"name\":\"SONY\", \"format\":\"sony\", \"data\":{\"custom\":\"555AF148\",\"pow\":[\"A900\"], \"cn\":[\"8900\",\"0900\"], \"vo\":[\"4900\",\"C900\"], \"mu\":[\"2900\"] , \"inputSel\":[\"A500\"] } }]"

typedef struct {
  String devicename, format, custom, power, chUp, chDn, voUp, voDn, mute, input;
} IRdata;

String reverse(String str){
     const char* b0 = ("0x" + str).c_str(); //連結してcharに
     long b1 = strtol(b0, NULL, 16); // longに
     uint8_t  rev = (~((uint8_t)b1)) ;
     String revSt = String(rev, HEX);
     if(rev < 16){
      revSt = "0" + revSt; //1ケタなら頭に0を足す
     }
     return revSt;
}

IRdata getIR(int num){
   char json[] = IR_DATA;
   aJsonObject* root = aJson.parse(json);
   if (root == NULL) {
     Serial.println("aJson.parse() failed");
     //return;
   }
   aJsonObject* obj = aJson.getArrayItem(root, num);
   String deviceame = aJson.getObjectItem(obj, "name")->valuestring;
   String format = aJson.getObjectItem(obj, "format")->valuestring;
   aJsonObject* data = aJson.getObjectItem(obj, "data");
   aJsonObject* tmp;
   String custom = aJson.getObjectItem(data, "custom")->valuestring;
   tmp = aJson.getObjectItem(data, "pow");
   String power = aJson.getArrayItem(tmp, 0)->valuestring;
   tmp = aJson.getObjectItem(data, "cn");
   String chUp = aJson.getArrayItem(tmp, 0)->valuestring;
   String chDn = aJson.getArrayItem(tmp, 1)->valuestring;
   tmp = aJson.getObjectItem(data, "vo");
   String  voUp = aJson.getArrayItem(tmp, 0)->valuestring;
   String  voDn = aJson.getArrayItem(tmp, 1)->valuestring;
   tmp = aJson.getObjectItem(data, "mu");
   String mute = aJson.getArrayItem(tmp, 0)->valuestring;
   tmp = aJson.getObjectItem(data, "inputSel");
   String input = aJson.getArrayItem(tmp, 0)->valuestring;

   if(format == "nec"){
     if(custom.length() < 3){
       custom += reverse(custom); 
       power  += reverse(power);
       chUp   += reverse(chUp);
       chDn   += reverse(chDn);
       voUp   += reverse(voUp);
       voDn   += reverse(voDn);
       mute   += reverse(mute);
       input  += reverse(input);
     }
   }
   aJson.deleteItem(root);
   return (IRdata){deviceame, format, custom, power, chUp, chDn, voUp, voDn, mute, input};  
}

int getIRcount(){
  char json[] = IR_DATA;
  aJsonObject* root = aJson.parse(json);
  int count = aJson.getArraySize(root);
  aJson.deleteItem(root);
  return count;
}
// /IRデータ処理 ---------------------------------------

void handleIndex(){
  Serial.println("handleIndex");
  String s = "<html lang=\"en\"><head><meta charset=\"utf-8\"/><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>M5Stack IRcube</title><link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.7/css/bootstrap.min.css\" /><script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery/3.2.1/jquery.min.js\"></script><script src=\"https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/3.3.7/js/bootstrap.min.js\"></script></head><body><div class=\"container\"><div class=\"row\"><div class=\"col-md-12\"><h1>M5Stack IRcube <small>";
  //s += localName;
  s += "m5ircube";
  s += ".local</small></h1><p>IP address: ";
  s += String(WiFi.localIP()[0]) + "." + String(WiFi.localIP()[1]) + "." + String(WiFi.localIP()[2]) + "." + String(WiFi.localIP()[3]);
  s += "</p><hr><div class=\"form-group\">";
  s += "<textarea class=\"form-control\" id=\"message\" rows=\"10\"></textarea></div>";
  s += "<div class=\"row\"> <button class=\"btn btn-success\" id=\"post\">POST</button> ";
  s += " <button class=\"btn btn-danger\" id=\"LEDON\">LED ON</button> ";
  s += " <button class=\"btn btn-info\" id=\"LEDOFF\">LED OFF</button>";
  //無線リセット
  s += " <button class=\"btn\" id=\"WIFIRESET\">WiFi reset</button> </div>";
  
  s += "</div>";
  s += "<script>var xhr = new XMLHttpRequest();var textarea = document.getElementById(\"message\");";
  //s += "document.getElementById(\"get\").addEventListener(\"click\", function () {xhr.open('GET', '/messages', true);xhr.setRequestHeader('X-Requested-With', 'curl');xhr.onreadystatechange = function() {if(xhr.readyState == 4) {textarea.value =xhr.responseText;}};xhr.send(null);});";
  s += "document.getElementById(\"post\").addEventListener(\"click\", function () {data = textarea.value;xhr.open('POST', '/messages', true);xhr.onreadystatechange = function() {if(xhr.readyState == 4) {alert(xhr.responseText);}};xhr.setRequestHeader('Content-Type', 'application/json');xhr.setRequestHeader('X-Requested-With', 'curl');xhr.send(data);});";

  //LED ON
  s += "document.getElementById(\"LEDON\").addEventListener(\"click\", function () {xhr.open('get', '/ledon', true);xhr.setRequestHeader('X-Requested-With', 'curl');xhr.onreadystatechange = function() {if(xhr.readyState == 4) {textarea.value =xhr.responseText;}};xhr.send(null);});";
  //LED OFF
  s += "document.getElementById(\"LEDOFF\").addEventListener(\"click\", function () {xhr.open('get', '/ledoff', true);xhr.setRequestHeader('X-Requested-With', 'curl');xhr.onreadystatechange = function() {if(xhr.readyState == 4) {textarea.value =xhr.responseText;}};xhr.send(null);});";
  //OnDemandAp Wifi設定 wifi reset
  s += "document.getElementById(\"WIFIRESET\").addEventListener(\"click\", function () {var result=confirm('Reset WiFi settings?');if(result){xhr.open('get', '/wifireset', true);xhr.setRequestHeader('X-Requested-With', 'curl');xhr.onreadystatechange = function() {if(xhr.readyState == 4) {textarea.value =xhr.responseText;}};xhr.send(null);}});";

  s += "</script></body></html>";
  webServer.send(200, "text/html", s);  
}

void handleNotFound() {
  Serial.println(webServer.uri().c_str());
  Serial.println("404");
  webServer.send(404, "text/plain", "Not found.");
}

void handleMessages() {
  Serial.println(webServer.uri().c_str()); //add

  if (webServer.method() == HTTP_POST) {
    //irrecv.disableIRIn();  //
    String req = webServer.arg(0);
    char json[req.length() + 1];
    req.toCharArray(json, req.length() + 1);
    Serial.println(json); //受信データ確認
    aJsonObject* root = aJson.parse(json);
    if (root != NULL) {
      aJsonObject* freq = aJson.getObjectItem(root, "freq");
      aJsonObject* data = aJson.getObjectItem(root, "data");
      if (freq != NULL && data != NULL) {
        const uint16_t d_size = aJson.getArraySize(data);
        //uint16_t rawData[d_size];
        unsigned int  rawData[d_size]; //型変更
        for (int i = 0; i < d_size; i++) {
          aJsonObject* d_int = aJson.getArrayItem(data, i);
          rawData[i] = d_int->valueint;
        }
        irsend.sendRaw(rawData, d_size, (uint16_t)freq->valueint);
        //irrecv.enableIRIn(); //
        req = "";
        aJson.deleteItem(root);
        webServer.sendHeader("Access-Control-Allow-Origin", "*");
        webServer.send(200, "text/plain", "ok");
      } else {
        webServer.send(400, "text/plain", "Invalid JSON format");
      }
    } else {
      webServer.send(400, "text/plain", "Request body is empty");
    }
  }
}

void handleLedOn() {
  Serial.println("LED ON");
  M5.Lcd.fillRect(220,20,100,100, RED); //
  webServer.send(200, "text/plain", "ok");
}
void handleLedOff() {
  Serial.println("LED OFF");
  M5.Lcd.fillRect(220,20,100,100, BLACK); //
  webServer.send(200, "text/plain", "ok");
}

void handleWiFiReset() { //WiFi reset
  Serial.println("reset WiFi settings");
  webServer.send(200, "text/plain", "reset WiFi settings...");

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,4);
  M5.Lcd.println("Reset WiFi settings");

  WiFi.disconnect(true);   // still not erasing the ssid/pw. Will happily reconnect on next start
  WiFi.begin("0","0");       // adding this effectively seems to erase the previous stored SSID/PW
  ESP.restart();
  delay(1000);
}

void printDeviceName(String e){
      M5.Lcd.fillRect(220,190,100,12,0); //
      M5.Lcd.setTextColor(0xffff,0);
      M5.Lcd.setTextSize(1);
      M5.Lcd.drawString(e, 220, 190, 1);
}

void sendIR(String format, String custom, String command){
  if (format == "nec"){
      unsigned long data = strtol(("0x" + custom + command).c_str() , NULL, 16);
      irsend.sendNEC(data, 32);
      Serial.print("sendIR: nec / ");
      Serial.println(custom + " " + command);
  } else if(format == "aeha"){
      unsigned int address = strtol(("0x" + custom).c_str(), NULL, 16); //
      unsigned long data = strtol(("0x" + command).c_str() , NULL, 16);
      Serial.println(address);
      Serial.println(data);
      irsend.sendPanasonic(address, data); //int, long
      Serial.print("sendIR: aeha / ");
      Serial.println(custom + " " + command);
  } else if(format == "sony"){
      unsigned long data = strtol(("0x" + custom + command).c_str() , NULL, 16);
      irsend.sendSony(data, 48);
      Serial.print("sendIR: sony / ");
      Serial.println(custom + " " + command);
  } else {
      Serial.println("sendIR: ???");
  }
  delay(300);
}


void CallBackRemote(MenuItem* mi){
  if (mi != NULL) {
    M5.Lcd.fillRect(0,0,220,8,0);//(0,0,320,8,0)
    M5.Lcd.setTextColor(0xffff,0); //0xff00,0
    M5.Lcd.setTextSize(1);
    //M5.Lcd.drawString("menu:" + mi->title + " / tag:" + mi->tag, 15, 0, 1);
    Serial.println("menu:" + mi->title + " / tag:" + mi->tag);

    if(mi->tag > 1000 && mi->tag < 1900){
      //Serial.println("TV control");
      IRdata ir = getIR(deviceNumberTV);
      String format  = ir.format;
      String custom  = ir.custom;
      String command;

      switch (mi->tag){
        case 1101:
          command = ir.power;          break;
        case 1201:
          command = ir.voUp;          break;
        case 1202:
          command = ir.voDn;          break;
        case 1203:
          command = ir.chUp;          break;
        case 1204:
          command = ir.chDn;          break;
        case 1102:
          command = ir.mute;          break;
        case 1103:
          command = ir.input;         break;
        default:
          command ="";
      }
      if(command){
        sendIR(format, custom, command);
      }
    }  // /tv

    if(mi->tag >= 1900 && mi->tag < 2000){ //TV選択
      deviceNumberTV = mi->tag - 1900;
      if(deviceNumberTV > getIRcount()){
        deviceNumberTV = 0;
      }
      printDeviceName(mi->title); //TV名 画面表示 右下
      //EEPROM書き込み
      preferences.begin("m5-ir", false);
      preferences.putInt("tv", deviceNumberTV);
      preferences.end();
    }

    if(mi->tag == 9001){ // DIGA DISP
      sendIR("aeha", "4004", "0D00FAF7");
    }
    if(mi->tag == 9002){ // DIGA BACK
      sendIR("aeha", "4004", "0D00818C");
    }
  } //

}
// ---------------------------------------------------------------
void setup() {  
  M5.begin();

  //SD-Updater
  if(digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }

  dacWrite(25, 0); // Speaker OFF
  M5.Lcd.fillScreen(BLACK);

  Serial.begin(115200);
  Serial.println("\n Starting");

  WiFiManager wifiManager; 

  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.println("M5Stack IRcube");
  M5.Lcd.println("\nIf you want to setup WiFi, \nPress Button C \nand Reset!!!");

  if(M5.BtnC.read()){ //起動時ボタンCが押されてたら
      M5.Lcd.fillScreen(BLACK); //clear
      M5.Lcd.setTextSize(2);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.println("AP mode: \nOnDemandAP\n");
      M5.Lcd.println("connect to WiFi network: \nM5Stack-AP");
      M5.Lcd.println("\nand");
      M5.Lcd.println("open\n");
      M5.Lcd.println("IP address: \n192.168.4.1");

      while(M5.BtnC.read());

      //起動時のボタンでWiFi設定起動 WiFiManager
      if (!wifiManager.startConfigPortal("M5Stack-AP")) {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
      }
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(2000);

  } else {
      //wifiManager.autoConnect("M5Stack-AP"); //AutoConnectAP 差し替え
      Serial.println("WiFi.begin()");
      WiFi.begin();
      M5.Lcd.setCursor(0,100);
      int i = 0;
      while (WiFi.status() != WL_CONNECTED) { //接続までしばらく待つ
        if(i > 20){break;}
        delay(500); 
        i++;
        Serial.print(".");
        M5.Lcd.print(".");
      }
        Serial.println("!");
        M5.Lcd.println("! ");
  }

  IPAddress ipadr = WiFi.localIP();
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("connected.");
    M5.Lcd.println("connected.");
    Serial.println(WiFi.SSID());
  } else {
    Serial.println("not connected!!!");
    M5.Lcd.println("not connected!!!");
    WiFi.disconnect(true); //つながらなかったらWiFi OFF
  }
  delay(1000);
  M5.Lcd.fillScreen(BLACK); //clear

  //IP Adress 表示 右下
  ipAddress = (String)ipadr[0] + "." + (String)ipadr[1] + "." + (String)ipadr[2] + "." + (String)ipadr[3];
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(220,210);
  M5.Lcd.print(ipAddress);

  //mDNSの設定
  if (MDNS.begin("m5ircube")) {
      //MDNS.addService("http", "tcp", 80);
      MDNS.addService("m5ircube", "tcp", 80);
      Serial.println("mDNS responder started");
  }

  // ウェブサーバーの設定
  webServer.on("/", handleIndex); // root
  webServer.on("/messages", handleMessages);  //messages
  webServer.on("/ledon", handleLedOn);  //LED ON  
  webServer.on("/ledoff"    , handleLedOff);  //LED OFF
  webServer.on("/wifireset", handleWiFiReset); //設定やり直し
  webServer.on("/favicon.ico", handleNotFound); //faviconなし

  webServer.onNotFound(handleNotFound);  
  webServer.begin();
  Serial.println("Web server started");

  //treeView
  treeView.clientRect.x = 2;   //2
  treeView.clientRect.y = 16;  //16
  treeView.clientRect.w = 216; //316
  treeView.clientRect.h = 200; //200
  treeView.itemWidth    = 200; //220
  
  treeView.useFACES       = true;
  treeView.useCardKB      = true;
  treeView.useJoyStick    = true;
  treeView.usePLUSEncoder = true;
  //treeView.clientRect.y = 10;
  //treeView.clientRect.h = 216; //◆216
  
  treeView.setItems(vmi
               { new MenuItem( "Power", 1101, CallBackRemote)
               , new MenuItem("VOL / CH", vmi
                   { new MenuItem("VOL UP",   1201, CallBackRemote)
                   , new MenuItem("VOL DOWN", 1202, CallBackRemote)
                   , new MenuItem("CH UP",    1203, CallBackRemote)
                   , new MenuItem("CH DOWN",  1204, CallBackRemote)
                   } )
               , new MenuItem( "MUTE",        1102, CallBackRemote) 
               , new MenuItem( "INPUT",       1103, CallBackRemote) 
               , new MenuItem( "SELECT TV",  vmi
                   { new MenuItem("TOSHIBA",   1900, CallBackRemote)
                   //, new MenuItem("FUJITSU",   1901, CallBackRemote)
                   , new MenuItem("HITACHI",   1901, CallBackRemote)
                   , new MenuItem("NEC",       1902, CallBackRemote)
                   , new MenuItem("Panasonic", 1903, CallBackRemote)
                   , new MenuItem("PIONEER",   1904, CallBackRemote)
                   //, new MenuItem("SANYO",     1906, CallBackRemote)
                   , new MenuItem("SHARP",     1905, CallBackRemote)
                   , new MenuItem("SONY",      1906, CallBackRemote)
                   } )
               , new MenuItem( "----------") 
               , new MenuItem( "DIGA DISP",   9001, CallBackRemote) 
               , new MenuItem( "DIGA BACK",   9002, CallBackRemote) 
               });
  treeView.begin();

  //左上 タイトル
  M5.Lcd.drawString("M5Stack IRcube", 5, 0, 1);


  //不揮発メモリ設定を読み取り EEPROM読み込み
  preferences.begin("m5-ir", false);
  int val = preferences.getInt("tv", 0);
  Serial.print("tv: ");
  Serial.println(val);
  if(val > getIRcount()){
    val = 0;
  }

  //JSONからデバイス名を取得して表示  右下
  deviceNumberTV = val;
  IRdata ir = getIR(deviceNumberTV);
  String devicename  = ir.devicename;
  Serial.print("setup devicename= ");
  Serial.println(devicename);
  printDeviceName(devicename);
  preferences.end();

} //setup()

void loop() {
  treeView.update();
  //delay(5); 
  webServer.handleClient();
  //delay(5); 
}
