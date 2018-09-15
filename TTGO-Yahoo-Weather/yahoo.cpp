#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "yahoo.h"

#define YAHOO_URL_LOCATION(location) "GET /v1/public/yql?q=select%20*%20from%20weather.forecast%20where%20woeid%20in%20(select%20woeid%20from%20geo.places(1)%20where%20text%3D%22" #location "%22)&format=json&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys  HTTP/1.0"

WiFiClient http;
StaticJsonBuffer<8192> jsonBuffer;
static yahoo_api_t _result;

static void parseJson(JsonObject &root)
{
  JsonObject &query = root["query"];
  JsonObject &results = query["results"];
  JsonObject &channel = results["channel"];

  JsonObject &units = channel["units"];
  _result.units.distance = (const char *)units["distance"];
  _result.units.pressure = (const char *)units["pressure"];
  _result.units.speed = (const char *)units["speed"];
  _result.units.temperature = (const char *)units["temperature"];

  JsonObject &location = channel["location"];
  _result.location.city = (const char *)location["city"];
  _result.location.country = (const char *)location["country"];
  _result.location.region = (const char *)location["region"];

  JsonObject &wind = channel["wind"];
  _result.wind.chill = (const char *)wind["chill"];
  _result.wind.direction = (const char *)wind["direction"];
  _result.wind.speed = (const char *)wind["speed"];

  JsonObject &atmosphere = channel["atmosphere"];
  _result.atmosphere.humidity = (const char *)atmosphere["humidity"];
  _result.atmosphere.pressure = (const char *)atmosphere["pressure"];
  _result.atmosphere.rising = (const char *)atmosphere["rising"];
  _result.atmosphere.visibility = (const char *)atmosphere["visibility"];

  JsonObject &astronomy = channel["astronomy"];
  _result.astronomy.sunrise = (const char *)astronomy["sunrise"];
  _result.astronomy.sunset = (const char *)astronomy["sunset"];

  JsonObject &item = channel["item"];

  JsonObject &condition = item["condition"];
  _result.today.code  = condition["code"] == "320100" ? "48" : (const char *)condition["code"];
  _result.today.date = (const char *)condition["date"];
  _result.today.temp = (const char *)condition["temp"];
  _result.today.text = (const char *)condition["text"];

  JsonObject &forecast = item["forecast"];
  for (int i = 0; i < item["forecast"].size(); i++)
  {
    JsonObject &obj = item["forecast"][i];
    _result.forecast[i].code  = obj["code"] == "320100" ? "48" : (const char *)obj["code"];
    _result.forecast[i].date = (const char *)obj["date"];
    _result.forecast[i].day = (const char *)obj["day"];
    _result.forecast[i].text = (const char *)obj["text"];
  }
}

bool getWeather()
{
  int ret = 0;
  char status[32] = {0};

  if (!http.connect("query.yahooapis.com", 80))
  {
    Serial.println("Connection failed");
    http.stop();
    return false;
  }

  Serial.println("Connected!");

  // Set up your city, example YAHOO_URL_LOCATION(location), replace the city with the URL, and make a GET request
  http.println(YAHOO_URL_LOCATION(shenzhen));
  http.println("Host: query.yahooapis.com");
  http.println("Connection: close");

  if (http.println() == 0)
  {
    Serial.println("Failed to send request");
    http.stop();
    return false;
  }
  // Check HTTP status
  http.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.0 200 OK") != 0)
  {
    Serial.print("Unexpected response: ");
    Serial.println(status);
    http.stop();
    return false;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!http.find(endOfHeaders))
  {
    Serial.println("Invalid response");
    http.stop();
    return false;
  }

  jsonBuffer.clear();

  // Parsing json data returned by Yahoo
  JsonObject &root = jsonBuffer.parseObject(http);
  if (!root.success())
  {
    Serial.println("parse data fail");
    http.stop();
    return false;
  }
  Serial.println("parse data success");
  parseJson(root);

  http.stop();
  return true;
}


const yahoo_api_t &getWeatherInfo()
{
  return _result;
}
