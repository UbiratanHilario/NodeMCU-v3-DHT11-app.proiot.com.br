/*
**  Ubiratan Hilario
**  Project with NodeMCU v3 to get the Temperature and the humidity from DHT11 and send to Serial, Display I2C (0x27) and https://app.proiot.com.br/
**  Completed: 24/05/2022

  #ifdef DEBUG_ESP_HTTP_CLIENT
  #ifdef DEBUG_ESP_PORT
  #define DEBUG_HTTPCLIENT(fmt, ...) DEBUG_ESP_PORT.printf_P( (PGM_P)PSTR(fmt), ## __VA_ARGS__ )
  #endif
  #endif

  #ifndef DEBUG_HTTPCLIENT
  #define DEBUG_HTTPCLIENT(...) do { (void)0; } while (0)
  #endif

  #define HTTPCLIENT_DEFAULT_TCP_TIMEOUT (30000)
*/

#include <Arduino.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
// Biblioteca https://github.com/arduino-libraries/NTPClient
#include <ESP8266WiFi.h>
// Biblioteca https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
// Biblioteca https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi


LiquidCrystal_I2C lcd(0x27, 16, 2); // FUNÇÃO DO TIPO "LiquidCrystal_I2C"


const char *ssid = "UBH2";            // nome do seu roteador WIFI (SSID)
const char *password = "36512519bac"; // senha do roteador WIFI
WiFiUDP ntpUDP;

// Configurações do Servidor NTP
const char *servidorNTP = "b.st1.ntp.br"; // Servidor NTP para pesquisar a hora
const int fusoHorario = -10800;           // Fuso horário em segundos (-10800 seg)
const int taxaDeAtualizacao = 60000;      // Taxa de atualização do servidor NTP em milisegundo
NTPClient timeClient(ntpUDP, servidorNTP, fusoHorario, taxaDeAtualizacao);
int hour = 0;
int minute = 0;
int minutos = 0;
// --------------------------------------------------------------------------------------
// DHT11 Sensor
#define DHTTYPE DHT11 // DHT 11
const int pino_dht = 0;
// Initialize DHT sensor.
DHT dht(pino_dht, DHTTYPE);
float Temperature;
float Humidity;
// --------------------------------------------------------------------------------------
WiFiClient client;
HTTPClient http;
// definicao das variaveis do servidor
const String API_URL = "things.conn.proiot.network";
const uint16_t PORTA = 80;
// todas as strings abaixo devem ser alteradas de acordo com as configracoes de seu projeto
const char *auth = "W8WZGsRdTP9wqRcVAT1Yw9Tfx4hRZ73";
const String ROTULO = "nodemcudht11";
const String DEVICE = "7eebc10f33be1733"; // 7e eb c1 0f 33 be 17 33
const String ALIAS = "alias";
const String ALIAS2 = "alias2";
const int ESPERA = 5;  // tempo de espera entre o envio das variaves em (s)
const long ATRASO = 5; // tempo de intervalo entre os envios de valores (padrao 20 min)
// --------------------------------------------------------------------------------------
void sendData();
void handleError(int httpCode, String message);
String getPayload();
// --------------------------------------------------------------------------------------
void handleError(int httpCode, String message)
{
  Serial.print("Code: " + String(httpCode));
  Serial.print(" | code msg: " + http.errorToString(httpCode));
  Serial.println(" | Message: " + message);
}
// --------------------------------------------------------------------------------------
String ip;
void setup()
{
  lcd.init();                    // INICIALIZA O DISPLAY LCD
  lcd.clear();                   // LIMPA O DISPLAY LCD
  lcd.backlight();               // HABILITA O BACKLIGHT (LUZ DE FUNDO)
  lcd.setCursor(0, 0);           // SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 1)
  lcd.print("Connectando...");   // ESCREVE O TEXTO NA PRIMEIRA LINHA DO DISPLAY LCD
  lcd.setCursor(0, 1);           // SETA A POSIÇÃO EM QUE O CURSOR RECEBE O TEXTO A SER MOSTRADO(LINHA 2)
  lcd.print("NODEMCU - OFFICE"); // ESCREVE O TEXTO NA SEGUNDA LINHA DO DISPLAY LCD

  // --------------------------------------------------------------------------------------
  Serial.begin(9600); // print no Serial Monitor da IDE
  Serial.println("Scanning I2C device...");
  Wire.begin();
  int count = 0;
  for (byte i = 0; i < 50; i++)
  {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0)
    {
      Serial.print("Address found->");
      Serial.print(" (0x");
      Serial.print(i, HEX);
      Serial.println(")");
      count++;
    }
    Serial.print("Found ");
    Serial.print(count, DEC);
    Serial.println(" device");
  }
  // --------------------------------------------------------------------------------------

  WiFi.begin(ssid, password);           // acessando a rede WIFI
  while (WiFi.status() != WL_CONNECTED) // aguardando a conexão WEB
  {
    delay(500); // aguarda 0,5 segundos
    Serial.print(".");
  }
  timeClient.begin();
  pinMode(pino_dht, INPUT);
  dht.begin();
  ip = WiFi.localIP().toString();
  Serial.println(ip);

  delay(2000);
}
// --------------------------------------------------------------------------------------
void loop()
{
  lcd.clear();
  timeClient.update();                           // atualiza o relogio
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity();       // Gets the values of the humidity
  if (isnan(Temperature) || isnan(Humidity))
  {
    Serial.println("Falha na leitura do Sensor DHT!");
  }
  else
  {
    Serial.println(timeClient.getFormattedTime() + ": Temperatura: " + (String)Temperature + " Unidade: " + (String)Humidity);

    lcd.setCursor(0, 0); // SETA A POSIÇÃO EM QUE O CURSOR INCIALIZA(LINHA 1)
    lcd.print(ip);       // ESCREVE O TEXTO NA PRIMEIRA LINHA DO DISPLAY LCD
    lcd.setCursor(12, 0);
    if (timeClient.getHours() < 10)
    {
      if (timeClient.getMinutes() < 10)
      {
        lcd.print("0" + (String)timeClient.getHours() + "0" + (String)timeClient.getMinutes());
      }
      else
      {
        lcd.print("0" + (String)timeClient.getHours() + (String)timeClient.getMinutes());
      }
    }
    else
    {
      if (timeClient.getMinutes() < 10)
      {
        lcd.print((String)timeClient.getHours() + "0" + (String)timeClient.getMinutes());
      }
      else
      {
        lcd.print((String)timeClient.getHours() + (String)timeClient.getMinutes());
      }
    }
    // --------------------------------------------------------------------------------------

    minutos = minute + ESPERA;
    if (minutos > 59)
    {
      // 58 + 5 = 63 - 60 = 3
      minutos = minutos - 60;
    }
    if (minute < 10)
    {
      lcd.setCursor(0, 1);
      lcd.print((String)Temperature + (char)223 + " " + (String)Humidity + "% 0" + (String)(minutos)); // ESCREVE O TEXTO NA SEGUNDA LINHA DO DISPL
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print((String)Temperature + (char)223 + " " + (String)Humidity + "% " + (String)(minutos)); // ESCREVE O TEXTO NA SEGUNDA LINHA DO DISPL
    }
    if (timeClient.getHours() < 10)
    {
      if (timeClient.getMinutes() < 10)
      {
        lcd.setCursor(0, 1);
        lcd.print("0" + (String)Temperature + (char)223 + " " + (String)Humidity + "% 0" + (String)(minutos)); // ESCREVE O TEXTO NA SEGUNDA LINHA DO DISPL
      }
      else
      {
        lcd.setCursor(0, 1);
        lcd.print("0" + (String)Temperature + (char)223 + " " + (String)Humidity + "% " + (String)(minutos)); // ESCREVE O TEXTO NA SEGUNDA LINHA DO DISPL
      }
    }
    else
    {
      if (timeClient.getMinutes() < 10)
      {
        lcd.setCursor(0, 1);
        lcd.print((String)Temperature + (char)223 + " " + (String)Humidity + "% 0" + (String)(minutos)); // ESCREVE O TEXTO NA SEGUNDA LINHA DO DISPL
      }
      else
      {
        lcd.setCursor(0, 1);
        lcd.print((String)Temperature + (char)223 + " " + (String)Humidity + "% " + (String)(minutos)); // ESCREVE O TEXTO NA SEGUNDA LINHA DO DISPL
      }
    }
  }
  // --------------------------------------------------------------------------------------
  if (timeClient.isTimeSet())
  {
    if ((hour == timeClient.getHours() && minutos <= timeClient.getMinutes()) || (hour == 0 && minute == 0))
    {
      Serial.println("Enviando dados.");
      sendData(); // envia os dados para a plataforma
    }

    // mensagem e reinicializacao deconexao com o servidor
    // Serial.println("\n\n");

    // tempo de intervalo entre as atualizacoes de valores
    hour = timeClient.getHours();
    minute = timeClient.getMinutes();
  }
  delay(2000);
}
// --------------------------------------------------------------------------------------
String getPayload()
{
  // conversao dos valores inteiros lidos para as strings
  String leitura_temperatura = String(Temperature, 2);
  String leitura_umidade = String(Humidity, 2);

  StaticJsonDocument<256> doc;

  JsonObject data = doc.to<JsonObject>();

  data["alias"] = leitura_temperatura;
  data["alias2"] = leitura_umidade;

  String payload;
  serializeJsonPretty(data, payload);
  //serializeJson(doc, payload);

  return payload;
}
// --------------------------------------------------------------------------------------
void sendData()
{
  String uri = "http://" + API_URL + "/" + DEVICE;
  client.connect(API_URL, PORTA);
  Serial.println(client.status());

  Serial.println((String)client + " " + uri + " " + PORTA + " " + DEVICE);
  http.begin(client, uri);
  http.connected();
  http.addHeader("Content-Type", "application/json", true, false);
  http.addHeader("Authorization", auth);
  http.setUserAgent(ROTULO);
  http.addHeader("Accept-Encoding", "application/json; charset=utf-8");
  http.addHeader("Accept", "*/*");
  http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  http.addHeader("Connection", "keep-alive");
  http.setTimeout(30000);
  http.setReuse(true);
  int httpCode = http.POST(getPayload());
  Serial.println(getPayload());


  Serial.println("Response code: " + (String)httpCode);
  Serial.println(http.getString());
  if (httpCode != HTTP_CODE_NO_CONTENT)
  {
    String response = http.getString();
    Serial.println("ERROR");
    handleError(httpCode, response);
    http.end();
    return;
  }
  client.stop();
}
// --------------------------------------------------------------------------------------
