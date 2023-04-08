/*
esp32通过wifi控制变频器
*/
#include <ArduinoWebsockets.h>
#include <WiFi.h>
#include <vector>
#include <ModbusMaster.h>
#include <FastAccelStepper.h>
#include <EEPROM.h>
#include <HTTPUpdate.h>

using namespace websockets;
WebsocketsServer server;
WebsocketsClient client;
WebsocketsClient *rule_client;
ModbusMaster freq_changer_node;
/*
  读取变频器频率设置
*/

int freq_setting_read()
{
  uint8_t result = freq_changer_node.readHoldingRegisters(0x3000, 1);

  // do something with data if read is successful
  if (result == freq_changer_node.ku8MBSuccess)
  {
    return freq_changer_node.getResponseBuffer(0);
  }
  return 0;
}

/*
  读取变频器当前频率
*/

int freq_read(){
  uint8_t result = freq_changer_node.readHoldingRegisters(0x3001, 1);

  // do something with data if read is successful
  if (result == freq_changer_node.ku8MBSuccess)
  {
    return freq_changer_node.getResponseBuffer(0);
  }
  return 0;
}

/*
  读取变频器当前状态
*/
int master_status_read(){
  uint8_t result = freq_changer_node.readHoldingRegisters(0x1001, 1);

  // do something with data if read is successful
  if (result == freq_changer_node.ku8MBSuccess)
  {
    return freq_changer_node.getResponseBuffer(0);
  }
  return 0;
}

/*
  设置变频器频率
  :param p: 5000代表50.00%
*/
void ferq_set(int p){
  freq_changer_node.writeSingleRegister(0x2000,p);
}

/*
  启动主轴
  :param d: 方向 0 正转，1 反转
*/
void start_master(int d){
  if (d == 0){//正转启动
    freq_changer_node.writeSingleRegister(0x1000,1);
  }
  else{
    freq_changer_node.writeSingleRegister(0x1000,2);
  }
}

/*
  停止主轴
*/
void stop_master(){
  freq_changer_node.writeSingleRegister(0x1000,5);
}

/*
  主轴点动
  :param d: 方向 0 正转，1 反转
*/
void startstop_master(int d){
  if (d == 0){//正转启动
    freq_changer_node.writeSingleRegister(0x1000,3);
  }
  else{
    freq_changer_node.writeSingleRegister(0x1000,4);
  }
}
//所有连接的客户端，实际上我只需要一个，应该踢掉其他客户端
std::vector<WebsocketsClient> allClients;
void pollAllClients() {
  for (auto& client : allClients) {
    client.poll();
  }
}
//初始化设置
void setup(){
    Serial.begin(115200);
    //连接变频器串口
    Serial2.begin(19200, SERIAL_8N1, 16, 17);
    freq_changer_node.begin(1, Serial2);//设置场站地址为1
    //Begin a soft AP
    IPAddress AP_local_ip(192,168,4,1);          //IP地址
    IPAddress AP_gateway(192,168,4,1);           //网关地址
    IPAddress AP_subnet(255,255,255,0);       //子网掩码
    WiFi.softAPConfig(AP_local_ip, AP_gateway, AP_subnet);
    WiFi.softAP("ZFocusAP", "txa990223");
    //等待AP初始化完成
    for (int i = 0; i < 15 && WiFi.status() != 255; i++) {
        Serial.print(".");
        Serial.print(WiFi.status());
        delay(1000);
    }
    Serial.print("主机IP:");
    Serial.println(WiFi.softAPIP());
    server.listen(8088);
    Serial.print("Is server live? ");
    Serial.println(server.available());
}
//ota升级开始
void update_start(){
  Serial.println("开始升级");
}
//ota升级完成
void update_finished(){
  ESP.restart();
}
//ota升级进度
void update_progress(int cur, int total){
  Serial.printf("Progress: %u%%\r", (cur / (total / 100)));
}
//ota升级错误
void update_error(int err){
  Serial.printf("Error[%u]: ", err);
}
void otaUpdate(){
  WiFiClient updateClient;
  httpUpdate.onStart(update_start);//当升级开始时
  httpUpdate.onEnd(update_finished);//当升级结束时
  httpUpdate.onProgress(update_progress);//当升级中
  httpUpdate.onError(update_error);//当升级失败时
  t_httpUpdate_return ret = httpUpdate.update(updateClient, "http://192.168.4.111/zfocus_freq.bin");
  switch(ret) {
    case HTTP_UPDATE_FAILED:      //当升级失败
        Serial.println("Update failed.");
        break;
    case HTTP_UPDATE_NO_UPDATES:  //当无升级
        Serial.println("NO Updates.");
        break;
    case HTTP_UPDATE_OK:         //当升级成功
        Serial.println("Update ok.");
        break;
  }
}
String status(){
  return "{'action': 'status', 'data':{'masterStatus': "+String(master_status_read())+",'ferq': "+String(freq_read())+",'ferq_set': "+String(freq_setting_read())+"}}";
}
void onMessageCallback(WebsocketsClient &m_client, WebsocketsMessage message) {
  Serial.print("Got Message: ");
  Serial.println(message.data());
  const WSInterfaceString cmd = message.data().substring(0,4);
  if(cmd == "1001"){//getStatus发送设备信息
    m_client.send(status()); 
  }
  else if(cmd == "5000"){//start_master 0
    start_master(0);
  }
  else if(cmd == "5001"){//start_master 1
    start_master(1);
  }
  else if (cmd == "5002"){//stop master
    stop_master();
  }
  else if (cmd == "5003"){//start stop master
    startstop_master(0);
  }
  else if(cmd == "5004"){//ferq_set
    int ferq =  message.data().substring(4).toInt();
    ferq_set(ferq);
  }
  else if(cmd == "9999"){//自己的ota升级
    otaUpdate();
  }
}
void loop() {
  if (server.available()) {
    // if there is a client that wants to connect
    if (server.poll()) {
      //accept the connection and register callback
      Serial.println("Accept a new client!");
      WebsocketsClient client = server.accept();
      client.onMessage(onMessageCallback);
      //client.onEvent(onEventsCallback);
      // store it for later use
      allClients.push_back(client);
    }
    // check for updates in all clients
    pollAllClients();
  }
}