#include "WiFi.h"
//#include <HTTPClient.h>
#include "HTTPClient_custom.cpp"

// 设置 WiFi SSID 和密码
const char *ssid = "***********";
const char *password = "**********";

// 设置 Azure Speech To Text API 的认证密钥
const char *subscriptionKey = "*****************";
String token = "";
int rawData_len = 0;
uint8_t *rawData = NULL;
const char *host = "eastasia.stt.speech.microsoft.com"; // 设置请求网址
const int httpsPort = 443;                              // 设置请求端口号
String resourceUri = "https://eastasia.tts.speech.microsoft.com/cognitiveservices/v1";

// 设置发送的文字内容 Set text content
String content = "hello";

void get_token(const char *key)
{
  String resourceUri = "https://eastasia.api.cognitive.microsoft.com/sts/v1.0/issueToken";
  const char *subscriptionKey = key;
  HHPClient_Custom http;
  http.begin(resourceUri);
  http.addHeader("Ocp-Apim-Subscription-Key", subscriptionKey);
  http.addHeader("Content-Length", "0");
  int httpResponseCode = http.POST("");
  if (httpResponseCode == HTTP_CODE_OK)
  {
    token = http.getString();
  }
  else
  {
    Serial.println("get token fail");
  }

  http.end();
}

String get_post_data(String &s)
{
  String post_data = "<speak version=\"1.0\" xmlns=\"http://www.w3.org/2001/10/synthesis\" xml:lang=\"zh-CN\">";
  post_data += "<voice name=\"Microsoft Server Speech Text to Speech Voice (zh-CN, *********)\">";
  post_data += s;
  post_data += "</voice></speak>";
  return post_data;
}

void get_response(String &d)
{
  HHPClient_Custom http;
  http.begin(resourceUri);
  http.addHeader("Authorization", "****** " + token);
  http.addHeader("Content-Type", "application/ssml+xml");
  http.addHeader("X-Microsoft-OutputFormat", "audio-16khz-64kbitrate-mono-mp3");
  http.addHeader("User-Agen", "Azure_TTS_Example");
  int ResponseCode = http.POST(d);
  delay(1000);
  if(ResponseCode != HTTP_CODE_OK)
  {
    Serial.println(ResponseCode);
    Serial.println("post fail");
  }
  http.cacheWriteSerial(); // 通过该函数将缓存内容写入串口而不保存
  http.end();
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
  }
  get_token(subscriptionKey);
  String datatopost = get_post_data(content);
  get_response(datatopost);
  delay(5000);
}

void loop()
{
}
