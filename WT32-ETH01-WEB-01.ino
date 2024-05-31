/* NetNode by d71

   Web сервер WT32-ETH0
   получает ip по dhcp
   на / отвечает страницей с чтением и установкой статуса порта
   на /Setup отвечает страницей настройки параметров
   настройки хранит в постоянной памяти
   инфу при старте выводит на Serial скорость 9600
   тыкается в сервер и пишет в скрипт сервера port и его state

  на IO15 подана 1 - с этоё ноги снимается питание для подтяжки кнопок IN12 и IN14


  а Arduino IDE целевая плата ESP32 Dev Module

*/


#include <ETH.h> // quote to use ETH
#include <WiFi.h>
#include <WebServer.h> // Introduce corresponding libraries
#include <Preferences.h> //EEPROM

#define ETH_ADDR        1
#define ETH_POWER_PIN   16//-1 //16 // Do not use it, it can cause conflict during the software reset.
#define ETH_POWER_PIN_ALTERNATIVE 16 //17
#define ETH_MDC_PIN    23
#define ETH_MDIO_PIN   18
#define ETH_TYPE       ETH_PHY_LAN8720
#define ETH_CLK_MODE    ETH_CLOCK_GPIO17_OUT // ETH_CLOCK_GPIO0_IN

IPAddress local_ip(192, 168, 1, 112);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);
IPAddress dns2 = (uint32_t)0x00000000;

const String build = "1.20240531.3";

//for preferens
Preferences Pref;
String remote_server, remote_script, serial_number, password ,port_secret; //настройки удаленного сервера
int out_pin2, out_pin4; //настройки пинов


int in_pin12,in_pin14,old_in_pin12,old_in_pin14;

static bool eth_connected = false;

WebServer server(80); // Declare the WebServer object

// Initialize the Web client object
WiFiClient client;


//ports

//out
int OUT2 = 2;
int OUT4 = 4;
int OUT15 = 15;

//in
int IN12 = 12;
int IN14 = 14;



void WebSetup() { //CallBack

  String html = R"EOF(
  
  <html>
  <head>
  <title>ESP32 NetNode</title>
    <script>

    window.onload = function(){
      getData();
    };
  
    function getData() {
      var xmlhttp;
        if (window.XMLHttpRequest) {
          xmlhttp = new XMLHttpRequest();
        }else{
          xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
        }
        
        xmlhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            obj =JSON.parse(this.responseText);
            document.getElementById("random_number").innerHTML = obj.random_number;
            document.getElementById("serial_number").innerHTML = obj.serial_number;
            document.getElementById("remote_server").value = obj.remote_server;
            document.getElementById("remote_script").value = obj.remote_script;
            document.getElementById("port_secret").value = "";
            document.getElementById("password").value = "";
          }
        };
        
      xmlhttp.open("GET", "GetData", true);
      xmlhttp.send();
    }

    function SetServer(){
          xmlhttp = new XMLHttpRequest();
          
          remote_server = document.getElementById("remote_server").value;
          remote_script = document.getElementById("remote_script").value;
          port_secret = document.getElementById("port_secret").value
          password = document.getElementById("password").value;
          new_password = document.getElementById("new_password").value;
          
          url="SetServer?remote_server="+remote_server+"&remote_script="+remote_script+"&port_secret="+port_secret+"&password="+password+"&new_password="+new_password;

          xmlhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
               getData();
            }
          };
          
          xmlhttp.open("GET", url, true);
          xmlhttp.send();
   
    }
         
    </script>
    </head>
  <body>
    <h2>WT32-ETH01 NetNode</h2>
    random out: <span id=random_number></span><br>
    SN: <span id=serial_number> x </span><br>
    
      <input type=button value="Reload Data" onclick="getData()"> <br><br>

      system password: <input id=password type=password name=password><br>
      new system password: <input id=new_password type=text name=new_password><br>
      remote server: <input id=remote_server type=text value='0.0.0.1' name=remote_server> <br>
      remote script: <input id=remote_script type=text value='data_in.php' name=remote_script> <br>
      port secret: <input id=port_secret type=text value='' name=port_secret> <br>
      <input type=button value=Save onclick="SetServer()" >
      
  </body>
  </html>

  )EOF";

  server.send(200, "text/html", html); //!!! Note that returning to the web page requires "text / html" !!!
}


void WebPort() {

  String html = R"EOF(
  
  <html>
  <head>
  <title>ESP32 NetNode Ports</title>
    <script>

    window.onload = function(){
      getPort();
    };
  
    function getPort() {
      var xmlhttp;
        if (window.XMLHttpRequest) {
          xmlhttp = new XMLHttpRequest();
        }else{
          xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
        }
        
        xmlhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            obj =JSON.parse(this.responseText);
            document.getElementById("random_number").innerHTML = obj.random_number;
            document.getElementById("serial_number").innerHTML = obj.serial_number;
            document.getElementById("out2").value = obj.out2;
            document.getElementById("out4").value = obj.out4;
            //document.getElementById("port_secret").value = "";
          }
        };
        
      xmlhttp.open("GET", "GetPort", true);
      xmlhttp.send();
    }

    function SetPort(){
          xmlhttp = new XMLHttpRequest();
          
          out2 = document.getElementById("out2").value;
          out4 = document.getElementById("out4").value;
          port_secret = document.getElementById("port_secret").value
          
          url="SetPort?out2="+out2+"&out4="+out4+"&port_secret="+port_secret;

          xmlhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
               getPort();
            }
          };
          
          xmlhttp.open("GET", url, true);
          xmlhttp.send();
   
    }
         
    </script>
    </head>
  <body>
    <h2>WT32-ETH01 NetNode Port</h2>
    random out: <span id=random_number></span><br>
    SN: <span id=serial_number> x </span><br>
    
      <input type=button value="Reload Port Status" onclick="getPort()"> <br><br>

      port secret: <input id=port_secret type=port_secret name=port_secret><br>
      port 02: <input id=out2 type=text value='' name=out2> <br>
      port 04: <input id=out4 type=text value='' name=out4> <br>
      <input type=button value=SetPort onclick="SetPort()" >
      
  </body>
  </html>

  )EOF";



  server.send(200, "text/html", html); //!!! Note that returning to the web page requires "text / html" !!!
}

void SetPort() { // Callback

  if (server.hasArg("port_secret")) {

    if (server.hasArg("out2")){
      out_pin2 = server.arg("out2").toInt();
    }

    if (server.hasArg("out4")){
      out_pin4 = server.arg("out4").toInt();
    }

    String get_port_secret = server.arg("port_secret");

    if (get_port_secret == port_secret) {

      SavePort();
      Serial.println( "Set Port: out2=" + String(out_pin2) + " out4=" + String(out_pin4) );
    } else {

      Serial.println("! Wrong port secret for Set Port state");
    }

  }

  String html = "ok";
  server.send(200, "text/html", html);
}




void SetServer() { // Callback

  if (server.hasArg("remote_server")) {
    remote_server = server.arg("remote_server");
    remote_script = server.arg("remote_script");
    port_secret = server.arg("port_secret");
    String get_password = server.arg("password");
    String new_password = server.arg("new_password");

    if (get_password == password) {

      if( new_password.length() > 3 && new_password.length() < 12 ){ //ну куда уж пароль меньше 4 знаков
        password = new_password;
        Serial.println("new setup password: " + password);
      }

      SaveServer();
      Serial.println("New Remote Server: " + remote_server + '/' + remote_script);
      Serial.println("New port secret: " + port_secret);
    } else {

      Serial.println("! Wrong password for save Server address");
    }

  }

  String html = "ok";
  server.send(200, "text/html", html);
}

void GetPort() { // Callback

  String random_number = String(random(10000));
  String message = "{\"serial_number\":\"" + serial_number + "\"," +
                    "\"random_number\":\"" + random_number + "\"," +
                    "\"out2\":\"" + String(out_pin2) + "\"," +
                   "\"out4\":\"" + String(out_pin4) + "\"," +
                   "\"in12\":\"" + String(in_pin12) + "\"," +
                   "\"in14\":\"" + String(in_pin14) + "\"}";
             
  server.send(200, "text/plain", message); // Send message back to page

  //Serial.println("Send JSON "+message);
}

void GetData() { // Callback

  String random_number = String(random(10000));
  String message = "{\"serial_number\":\"" + serial_number + "\"," +
                   "\"remote_server\":\"" + remote_server + "\"," +
                   "\"remote_script\":\"" + remote_script + "\"," +
                   "\"random_number\":\"" + random_number + "\"}";
  server.send(200, "text/plain", message); // Send message back to page
}

void SavePort() {

  Pref.begin("Settings", false);
    Pref.putInt("out_pin2", out_pin2);
    Pref.putInt("out_pin4", out_pin4);
  Pref.end();

  digitalWrite(OUT2, out_pin2);
  digitalWrite(OUT4, out_pin4);
}


void SaveServer() {

  Pref.begin("Settings", false);
    Pref.putString("remote_server", remote_server);
    Pref.putString("remote_script", remote_script);
    Pref.putString("port_secret", port_secret);
    Pref.putString("password", password);
  Pref.end();
}

void httpRequest(int PortEvent, int state)
{

  if (remote_server == "0.0.0.0") {
    Serial.println("Set remote server addres other 0.0.0.0");
    Serial.println("");
    return;
  }

  Serial.println("try Request server " + remote_server);

  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection
  if (client.connect( remote_server.c_str(), 80)) {

    String s;
    s = "Connecting to ..."; s += remote_server;
    Serial.println(s);

    // send the HTTP PUT request
    s = "GET /" + remote_script + "?sn=" + serial_number + "&port=" + String(PortEvent) +
        "&state=" + String(state) + " HTTP/1.1";
    client.println(s);

    s = "Host: " + remote_server;
    client.println(s);
    client.println("Connection: close");
    client.println();

    Serial.println("Request end");
  } else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }

  Serial.println("");

}


void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

void ReadConfig() {

  Serial.println("Read config:");


  Pref.begin("Settings", false);
  remote_server = Pref.getString("remote_server");
  if (remote_server.length() == 0 ) {
    remote_server = "0.0.0.0";
    Pref.putString("remote_server", remote_server);
  }

  remote_script = Pref.getString("remote_script");
  if (remote_script.length() == 0 ) {
    remote_script = "_in.php";
    Pref.putString("remote_script", remote_script);
  }

  Serial.println("remote server: " + remote_server + "/" + remote_script);



  serial_number = Pref.getString("serial_number");
  if (serial_number.length() == 0 ) {
    serial_number = String(random(10000000));
    Pref.putString("serial_number", serial_number);
  }

  Serial.println("serial number: " + serial_number);



  password = Pref.getString("password");
  if (password.length() == 0 ) {
    //password = String(random(10000000));
    password = "12345678";
    Pref.putString("password", password);
  }

  Serial.println("Setup password: " + password);



  port_secret = Pref.getString("port_secret");
  if (port_secret.length() == 0 ) {
    //port_secret = String(random(10000000));
    port_secret = "12345678";
    Pref.putString("port_secret", port_secret);
  }

  Serial.println("Port secret: " + port_secret );

  
  out_pin2 = Pref.getInt("out_pin2");
  out_pin4 = Pref.getInt("out_pin4");

  //Write Port State
  digitalWrite(OUT2, out_pin2);
  digitalWrite(OUT4, out_pin4);

  Serial.println("Port status: pin_pin2=" + String(out_pin2) + " out_pin4=" + String(out_pin4) );
  Serial.println("");

  Pref.end();

}

void setup() {
  pinMode(ETH_POWER_PIN_ALTERNATIVE, OUTPUT);
  digitalWrite(ETH_POWER_PIN_ALTERNATIVE, HIGH);


  pinMode(OUT2, OUTPUT);
  pinMode(OUT4, OUTPUT);
  
  pinMode(OUT15, OUTPUT); //питание для кнопок
  digitalWrite(OUT15, HIGH);

  pinMode(IN12, INPUT);
  pinMode(IN14, INPUT);


  Serial.begin(9600);
  delay(1000);
  Serial.println("\r\n\r\n");

  WiFi.onEvent(WiFiEvent);

  ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE); // Enable ETH

  //ETH.config(local_ip, gateway, subnet, dns1, dns2); // Static IP, leave without this line to get IP via DHCP

  while (!((uint32_t)ETH.localIP())) {}; // Waiting for IP (leave this line group to get IP via DHCP)

  // web callback
  // принимаем ajax, отдаём json
  server.on("/", WebPort);
  server.on("/GetPort", HTTP_GET, GetPort);
  server.on("/GetData", HTTP_GET, GetData);
  server.on("/Setup", HTTP_GET, WebSetup);
  server.on("/SetServer", HTTP_GET, SetServer);
  server.on("/SetPort", HTTP_GET, SetPort);

  //start server
  server.begin();
  Serial.println("NetNode by d71 ver " + build);
  
  Serial.println("Local Web Server Started");
  
  Serial.print("MAC: ");
  Serial.println( ETH.macAddress() );
  
  Serial.print("IPv4: ");
  Serial.println( ETH.localIP() );
  
  Serial.println("\r\n");

  ReadConfig();

  //запоминаем состоние входов
  old_in_pin12 = digitalRead(IN12);
  old_in_pin14 = digitalRead(IN14);
}



void loop() {
  
  server.handleClient(); // Handling requests from clients



  in_pin12 = digitalRead(IN12);

  if (old_in_pin12 != in_pin12){

    Serial.println("IN12 change state=" + String(in_pin12));
    Serial.println("");
    old_in_pin12 = in_pin12;
    httpRequest(12, in_pin12);  
    delay(500);
  }


  
  in_pin14 = digitalRead(IN14);

  if (old_in_pin14 != in_pin14){

    Serial.println("IN14 change state=" + String(in_pin14));
    Serial.println("");
    old_in_pin14 = in_pin14;
    httpRequest(14, in_pin14);  
    delay(500);
  }

}
