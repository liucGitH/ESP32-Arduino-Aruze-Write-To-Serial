# ESP32-Arduino-Aruze-Write-To-Serial
This library help write data in cache to Serial port when message receive by WiFiClient(eg Aruze-ttk) is too long that can not save as a string or a list on ESP32 or Arduino.
When using Aruze-ttk to convert a string to a piece of sound, all data receive by WiFiClient would save as cache. However, if read one byte then write one byte by "Serial.write(http.read());",it loss some of the bytes. Then you would get a media file which wave look like below.

![image](https://github.com/liucGitH/ESP32-Arduino-Aruze-Write-To-Serial/assets/77263346/e4569085-279a-4fd9-8923-c48ce8c2350c)
However, it should be like this:
![image](https://github.com/liucGitH/ESP32-Arduino-Aruze-Write-To-Serial/assets/77263346/ad40b5e6-52e2-4210-bae6-6ec92b1a34b8)

The cpp file inherit <HTTPClient.h> library, adding a new method "cacheWriteSerial()" which directly write data in cache to Serial port without saving it to memory as a string or a list. An example is included in this repository.
