# wt32-eth01-web
web server for WT32-ETH01 with api port control

ТЗ:
1. Сетевое устройство получает по DHCP настройки сети IPv4
   
2. Все свои настройки хранит в энергонезависимой памяти
   
3. При старте отправляет в интерфейс UART:
   * свой серийный номер ( формируется автоматически при первом старте, менять нельзя , далее по тексту для примера 6812005)
   * MAC адрес
   * IPv4 адрес ( далее по тексту для примера 192.168.1.2 )
   * адрес Сервера уведомлений ( далее по тексту для примера 192.168.1.1 )
   * имя скрипта на Сервере уведомений ( далее по тексту для примера _in.php )
   * пароль для конфигурирования этого устройства через web интерфейс ( формируется автоматически при первом старте, менять нельзя )
   * пароль для управления портами через web интерфейс ( формируется автоматически при первом старте, менять можно, далее по тексту для примера 1234567 )
     
4. При изменении состояния портов ввода, устройство отправляет сообщение на сервер уведомлений

5. Настройка и управление портами осуществляется посредством Web сервера устройства
   * http://192.168.1.2/ - управление портами вывода с соответствующим паролем
   * http://192.168.1.2/Setup - управление настройками Сервера уведомлений, имени скрипта сервера уведомлений, пароля управления портами

6. Управление портами напрямую возможно через запрос типа http://192.168.1.2/SetPort?port_secret=1234567&out4=0&out2=1 , при этом парамерт port_secret явзяется обязательным

7. Возможно получить состояние портов вывода и ввода запросом типа http://192.168.1.2/GetPort , ответом будет JSON вида {"serial_number":"6812005","random_number":"1085","out2":"1","out4":"0","in12":"1","in14":"1"}

8. Сервер уведомлений получает http GET строку типа http://192.168.1.1/_in.php?sn=6812005&port=12&state=0
     
Тестовым (целевым) устройством выбрано WT32-ETH01

Питание желательно осуществлять через съемник PoE - 5v

IO2 и IO4 - порты вывода, контрольно - светодиоды ( выше в запросах фигурируют названия out2 и out4 )

IO12 и IO14 - порты ввода, контрольно - конопки через подтяжку питания ( выше в запросах фигурируют названия in12 и in14 )




