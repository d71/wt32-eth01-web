# wt32-eth01-web
web server for WT32-ETH01 with api port control

ТЗ:
1. Сетевое устройство получает по DHCP настройки сети IPv4
2. Все свои настройки хранит в энергонезависимой памяти
3. При старте отправляет в интерфейс UART:
   свой серийный номер
   MAC адрес
   IPv4 адрес
   адрес Сервера уведомлений
   имя скрипта на Сервере уведомений
   пароль для конфигурирования этого устройства через web интерфейс
   пароль для управления портами через web интерфейс
4. При изменении состояния нескольких портов ввода, устройство отправляет сообщение на сервер уведомлений

Тестовым (целевым) устройством выбрано WT32-ETH01
Питание осуществляется через съемник PoE - 5в

IO2 и IO4 - порты вывода, контрольно - светодиоды
IO12 и IO14 - порты ввода, контрольно - конопки через подтяжку птания


