# Websocket控制变频器
该开源项目是一个基于WebSocket协议实现的远程控制变频器的工具，采用ESP32芯片作为控制核心。  
通过使用WebSocket协议和ESP32芯片，该工具可以在Web浏览器中实现远程控制变频器的功能，无需安装额外的客户端软件。  
![PCB](https://github.com/JinYing123/ESP32_Frequency/blob/main/PCB.png)
***
目前wifi模式是AP，修改下面这段代码可以换成连接路由
```c++
//AP模式
IPAddress AP_local_ip(192,168,4,1);          //IP地址
IPAddress AP_gateway(192,168,4,1);           //网关地址
IPAddress AP_subnet(255,255,255,0);       //子网掩码
WiFi.softAPConfig(AP_local_ip, AP_gateway, AP_subnet);
WiFi.softAP("ZFocusAP", "txa990223");
```
