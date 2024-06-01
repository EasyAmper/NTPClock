//    NTPClock by EASYAMPER 01.06.2024
//    обновление и отсчет времени с периодическим выводом в порт + день недели по UNIX
#define FAIL_TIME 500       // период вывода сообщения об ошибке синхронизации в порт
#define SYN_TIME 50000      // период синхронизации времени

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <UnixTime.h>     // для преобразования UNIX в дату (https://github.com/GyverLibs/UnixTime)

byte h, m, s, WD;
unsigned long t;
unsigned long syn;
unsigned long rec;

const char *ssid     = "Nastya";   // имя сети
const char *password = "32830545";   // пароль

UnixTime stamp(3);   // часовой пояс
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ru.pool.ntp.org", 10800, 60000);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
  timeClient.update();
  h = timeClient.getHours();   // первоначальная синхронизация
  m = timeClient.getMinutes();
  s = timeClient.getSeconds();
  stamp.getDateTime(timeClient.getEpochTime());   // получаем время UNIX, скармливаем библиотеке
  WD = stamp.dayOfWeek;    // забираем из нее дни недели
}

void loop() {
  // блок вывода и самостоятельного счёта времени
  if (millis() - t > 1000) {
    t = millis();
    Serial.println("internal time:");
    Serial.print(h);
    Serial.print(":");
    Serial.print(m);
    Serial.print(":");
    Serial.println(s);
    Serial.print("Week day: ");
    Serial.println(WD);

    s = s + 1;
    if (s == 60) {              // вычисление минут и часов по секундам
      s = 0;                    // в 1ой минуте 60 сек
      m = m + 1;
    }
    if (m == 60) {              // в 1ом часе 60 минут
      m = 0;
      h = h + 1;
    }
    if (h == 24) {             // день в среднем равен 24 часа
      h = 0;
      m = 0;                   //   сброс счётчика , можно добавить
      s = 0;                   //   счёт дней, добавив переменную
    }
  }
  // блок синхронизации (обновление времени)
  if ( WiFi.status() != WL_CONNECTED ) {   // блок обновления времени
    if (millis() - rec > FAIL_TIME) {
      rec = millis();
      Serial.println("synchronization failed !");
    }
  } else {
    if (millis() - syn > SYN_TIME) {
      syn = millis();
      timeClient.update();    // обновление времени
      h = timeClient.getHours();
      m = timeClient.getMinutes();
      s = timeClient.getSeconds();
      stamp.getDateTime(timeClient.getEpochTime());   // получаем время UNIX, скармливаем библиотеке
      WD = stamp.dayOfWeek;    // забираем из нее дни недели
      Serial.println("successful time synchronization");
    }
  }
}
