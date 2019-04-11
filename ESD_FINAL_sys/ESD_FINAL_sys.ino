#include<ESP8266WiFi.h>
#include<ESP8266WebServer.h>
#include<EEPROM.h>
#include<ArduinoJson.h>


ESP8266WebServer server ;

bool setupStatus ;
byte setupStatusPos = 0;
String defualtPassword = "wifilockpass123";

String newPassword ;
byte passswordLocation = 80;



char* ssid = "SSID";
char* password = "PASSWORD";




void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("IOT LOCK");
  EEPROM.begin(512);
  EEPROM.write(0,0);
  EEPROM.commit();
  pinMode(D0,OUTPUT);
  pinMode(D1,OUTPUT);
  setupStatus = EEPROM.read(0);
  Serial.println(setupStatus);
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("*");
    delay(500);
  }
  Serial.println("IP ADDRESS :");
  Serial.println(WiFi.localIP());

  Serial.println("ping ");
  
  server.on("/intial",HTTP_OPTIONS,inititalSetupOptions);
  server.on("/intial",HTTP_POST,inititalSetup);
   
  server.on("/unlock",HTTP_OPTIONS,unlockOptions);
  server.on("/unlock",HTTP_POST,unlock);
  
  server.on("/update_p",HTTP_OPTIONS,update_Options);
  server.on("/update_p",HTTP_POST,update_);
  
  server.on("/reset",HTTP_OPTIONS,resetOptions);
  server.on("/reset",HTTP_POST,reset_);
  
  server.begin();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();

}

void inititalSetup(){
  server.sendHeader("Access-Control-Allow-Origin","*");
  if(!setupStatus){
      Serial.println("Intial setup");
      String data =  server.arg("plain");
      StaticJsonBuffer<500> jsonbuffer;
      JsonObject& jObject = jsonbuffer.parseObject(data);
      String setupPassword_T = jObject["setupPassword"];
      String newPassword_T = jObject["currentPassword"];
      Serial.println(setupPassword_T);
      Serial.println(newPassword_T);
      if(setupPassword_T == defualtPassword){
        Serial.println("PASSWORD MATCH");
        writeString(80,newPassword_T);
        Serial.println(read_String(80));
        EEPROM.write(setupStatusPos,true);
        setupStatus = true;
        EEPROM.commit();
        server.send(200,"OK");

      }
        else if(setupPassword_T != defualtPassword ){
        Serial.println("ERROR ... Password does not match");
        server.send(401,"Unauthorised");
      }
      else{
        Serial.println("UNKOWN ERROR");
        server.send(400,"!OK");
      }
  }
  else{
    server.send(204,"");
  }


}
void inititalSetupOptions(){
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(204,"");
}

void unlock(){
  
  server.sendHeader("Access-Control-Allow-Origin","*");
  Serial.println("test123");
  newPassword = read_String(80);
  Serial.println(newPassword);
  String data =  server.arg("plain");
  StaticJsonBuffer<500> jsonbuffer ;
  JsonObject &jObject = jsonbuffer.parseObject(data);

  String password_T = jObject["password"];
  Serial.println(password_T);
  if(password_T == newPassword){
    Serial.println("Password match");
    doorUnlock();
    server.send(200,"");

  }
  else if(password_T != newPassword){
    Serial.println("ERROR .... password inccorect");
    server.send(401,"");

  }
  else{
    Serial.println("UNKNOWN ERROR");
    server.send(204,"");
  }
}
void unlockOptions(){
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(204,"");
}


void doorUnlock(){
  digitalWrite(D0,LOW);
  digitalWrite(D1,LOW);
  digitalWrite(D0,HIGH);
  digitalWrite(D1,LOW);  
  delay(5000);
  digitalWrite(D0,LOW);
  digitalWrite(D1,LOW);
  
  
}

void update_(){
  newPassword = read_String(80);
  String data =  server.arg("plain");
  StaticJsonBuffer<500> jsonbuffer ;
  JsonObject &jObject = jsonbuffer.parseObject(data);
  String password_T = jObject["currentPassword"];
  String newPassword_T = jObject["newPassword"];

  if(newPassword == password_T){
    writeString(80,newPassword_T);
    EEPROM.commit();
    server.send(204,"");
  }
  else {
    return;
  }
}

void update_Options(){
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(204,"");
}

void reset_(){
  server.sendHeader("Access-Control-Allow-Origin","*");
  String pasword = readString(80);
  StaticJsonBuffer <500> jsonbuffer;
  JsonObject Jobject = jsonbuffer.parseObject(server.args("plain"));
  String recivedPassword = Jobject["currentPassword "];
  if(password == recivedPassword){
    Serial.println("Reseting.. ");
    for (int i = 0; i < 512; i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();
    server.send(200,"reset");
  }
  else{
    server.send(401,"Incorrect Pasword");
 
  
}
void resetOptions(){
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(204,"");
}

void writeString(char add,String data)
{
  int _size = data.length();
  int i;
  for(i=0;i<_size;i++)
  {
    EEPROM.write(add+i,data[i]);
  }
  EEPROM.write(add+_size,'\0'); 
  EEPROM.commit();
}

String read_String(char add)
{
  int i;
  char data[100]; //Max 100 Bytes
  int len=0;
  unsigned char k;
  k=EEPROM.read(add);
  while(k != '\0' && len<500)   
  {    
    k=EEPROM.read(add+len);
    data[len]=k;
    len++;
  }
  data[len]='\0';
  return String(data);
}
