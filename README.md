# Websocket控制变频器
变频器通过RS485控制，ESP32提供Wifi。具体控制命令可以看onMessageCallback，也可以按自己的变频器修改控制函数。
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
