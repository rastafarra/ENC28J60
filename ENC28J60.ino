/* Дмитрий Осипов. http://www.youtube.com/user/d36073?feature=watch

   v.01 Управляем Arduino с Web страницы Webserver Ethernet ENC28J60 Enternet HR911105A Pin ON OF Relays.

   -----

   Что нам понадобится:

   1). ENC28J60 Ethernet LAN / Network Module.

http://www.ebay.com/sch/i.html?_from=R40&_sacat=0&_nkw=ENC28J60&rt=nc&LH_BIN=1

2). Библиотека / Library - "ethercard".

EtherCard is a driver for the ENC28J60 chip, compatible with Arduino IDE.

https://github.com/jcw/ethercard

или берем здесь. https://yadi.sk/d/R57sVoglbhTRN

3).

Скачать sketch.

v.01 Управляем Arduino с Web страницы Webserver Ethernet ENC28J60 Enternet HR911105A Pin ON OF Relays.

-----------------------

Подключаем Pins "ENC28J60 Module" к Arduino Uno.

VCC - 3.3V

GND - GND

SCK - Pin 13

SO - Pin 12

SI - Pin 11

CS - Pin 10 Можно выбрать любой.

Подключаем "ENC28J60 Module" например к Router, загружаем sketch, открываем страницу

в браузере например 192.168.1.222 , на странице можем включать выключат Pins / реле.

---------

Примечание: "ENC28J60 Module" питается от 3.3 volts, и потребляет по документации 250mA.

Arduino Uno Максимальный допустимый ток, получаемый с 3V3 контакта — 50 мА.

У меня прекрасно всё работает с 3V3 контакта Arduino Uno.

На всякий случай, предупреждаю.

------------------

 */

#include <EtherCard.h> // Подключаем скачанную библиотеку. https://yadi.sk/d/R57sVoglbhTRN

#include <EEPROM.h>

// MAC Address должен быть уникальным в вашей сети. Можно менять.

static byte mymac[] = {

	0x5A,0x5A,0x5A,0x5A,0x5A,0x5A };

// ip статический / постоянный Address нашей Web страницы.

static byte myip[] = {

	192,168,11,222 };

// Буфер, чем больше данных на Web странице, тем больше понадобится значения буфера.

byte Ethernet::buffer[900];

BufferFiller bfill;

// Массив задействованных номеров Pins Arduino, для управления например 8 реле.

int LedPins[] = {

	2,3,4,5,6,7,8,9};

// Массив для фиксации изменений.

boolean PinStatus[] = {

	false,false,false,false,false,false,false,false};

//-------------

const char http_OK[] PROGMEM =

"HTTP/1.0 200 OK\r\n"

"Content-Type: text/html; charset=utf-8\r\n"

"Pragma: no-cache\r\n\r\n";

const char http_Found[] PROGMEM =

"HTTP/1.0 302 Found\r\n"

"Location: /\r\n\r\n";

const char http_Unauthorized[] PROGMEM =

"HTTP/1.0 401 Unauthorized\r\n"

"Content-Type: text/html\r\n\r\n"

"<h1>401 Unauthorized</h1>";

//------------

// Делаем функцию для оформления нашей Web страницы.

void homePage()

{

	bfill.emit_p(PSTR("$F"

				"<title>Творчество имени Смоляра</title>"

        "<h1>ПКУ-10</h1>"

				"Канал: <a href=\"?ArduinoPIN1=$F\">$F</a><br />"),

			http_OK,

			PinStatus[1]?PSTR("off"):PSTR("on"),

			PinStatus[1]?PSTR("<font color=\"green\"><b>основной</b></font>"):PSTR("<font color=\"red\">резервный</font>"));

}

//------------------------

void setup()

{

	Serial.begin(9600);

	if (ether.begin(sizeof Ethernet::buffer, mymac,10) == 0) {};

//	if (!ether.dhcpSetup()) {};

	// Выводим в Serial монитор IP адрес который нам автоматический присвоил наш Router.

	// Динамический IP адрес, это не удобно, периодический наш IP адрес будет меняться.

	// Нам придётся каждый раз узнавать кой адрес у нашей страницы.

//	ether.printIp("My Router IP: ", ether.myip); // Выводим в Serial монитор IP адрес который нам присвоил Router.

	// Здесь мы подменяем наш динамический IP на статический / постоянный IP Address нашей Web страницы.

	// Теперь не важно какой IP адрес присвоит нам Router, автоматический будем менять его, например на "192.168.1.222".

	ether.staticSetup(myip);

//	ether.printIp("My SET IP: ", ether.myip); // Выводим в Serial монитор статический IP адрес.

	//-----

  byte state = EEPROM.read(1);

	for(int i = 0; i <= 7; i++)	{
		pinMode(LedPins[i],OUTPUT);

		PinStatus[i] = 1 == state ? true : false;
	}

  for (int i = 0; i <= 7; i++) 
    digitalWrite(LedPins[i],PinStatus[i+1]);

}

// --------------------------------------

void loop()

{

	delay(1); // Дёргаем микроконтроллер.

	word len = ether.packetReceive(); // check for ethernet packet / проверить ethernet пакеты.

	word pos = ether.packetLoop(len); // check for tcp packet / проверить TCP пакеты.

	if (pos) {

		bfill = ether.tcpOffset();

		char *data = (char *) Ethernet::buffer + pos;

		if (strncmp("GET /", data, 5) != 0) {

			bfill.emit_p(http_Unauthorized);

		}

		else {

			data += 5;

			if (data[0] == ' ') {

				homePage(); // Return home page Если обнаружено изменения на станице, запускаем функцию.

				for (int i = 0; i <= 7; i++)digitalWrite(LedPins[i],PinStatus[i+1]);

			}

			// "16" = количество символов "?ArduinoPIN1=on ".

			else if (strncmp("?ArduinoPIN1=on ", data, 16) == 0) {
        EEPROM.write(1, 1);

				PinStatus[1] = true;

				bfill.emit_p(http_Found);

			}

			else if (strncmp("?ArduinoPIN2=on ", data, 16) == 0) {

				PinStatus[2] = true;

				bfill.emit_p(http_Found);

			}

			else if (strncmp("?ArduinoPIN3=on ", data, 16) == 0) {

				PinStatus[3] = true;

				bfill.emit_p(http_Found);

			}

			else if (strncmp("?ArduinoPIN4=on ", data, 16) == 0) {

				PinStatus[4] = true;

				bfill.emit_p(http_Found);

			}

			else if (strncmp("?ArduinoPIN5=on ", data, 16) == 0) {

				PinStatus[5] = true;

				bfill.emit_p(http_Found);

			}

			else if (strncmp("?ArduinoPIN6=on ", data, 16) == 0) {

				PinStatus[6] = true;

				bfill.emit_p(http_Found);

			}

			else if (strncmp("?ArduinoPIN7=on ", data, 16) == 0) {

				PinStatus[7] = true;

				bfill.emit_p(http_Found);

			}

			else if (strncmp("?ArduinoPIN8=on ", data, 16) == 0) {

				PinStatus[8] = true;

				bfill.emit_p(http_Found);

			}

			//------------------------------------------------------

			else if (strncmp("?ArduinoPIN1=off ", data, 17) == 0) {
        EEPROM.write(1, 0); // off --- 1

				PinStatus[1] = false;

				bfill.emit_p(http_Found);

			}

			else if (strncmp("?ArduinoPIN2=off ", data, 17) == 0) {

				PinStatus[2] = false;

				bfill.emit_p(http_Found);

			}

			else if (strncmp("?ArduinoPIN3=off ", data, 17) == 0) {

				PinStatus[3] = false;

				bfill.emit_p(http_Found);

			}

			else if (strncmp("?ArduinoPIN4=off ", data, 17) == 0) {

				PinStatus[4] = false;

				bfill.emit_p(http_Found);

			}

			else if (strncmp("?ArduinoPIN5=off ", data, 17) == 0) {

				PinStatus[5] = false;

				bfill.emit_p(http_Found);

			}

			else if (strncmp("?ArduinoPIN6=off ", data, 17) == 0) {

				PinStatus[6] = false;

				bfill.emit_p(http_Found);

			}

			else if (strncmp("?ArduinoPIN7=off ", data, 17) == 0) {

				PinStatus[7] = false;

				bfill.emit_p(http_Found);

			}

			else if (strncmp("?ArduinoPIN8=off ", data, 17) == 0) {

				PinStatus[8] = false;

				bfill.emit_p(http_Found);

			}

			//---------------------------

			else {

				// Page not found

				bfill.emit_p(http_Unauthorized);

			}

		}

		ether.httpServerReply(bfill.position()); // send http response

	}

}
