#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WifiLocation.h>
#include <EEPROM.h>

const char* ssid     = "Nome rede wifi";
const char* password = "Senha rede wifi";
const char* host = "Dns dinâmico";
const char* googleApiKey = "Chave API Google";

int sensor_entrada = D0;
int sensor_conf_entrada = D1;
int sensor_saida = D5;
int sensor_conf_saida = D6;
int entrada = 0;
int saida = 0;
int lotacao = 0;
String coordenadas = "";
int contador_localizacao = 0;
int contador_viagem = 0;
String viagem = "";
byte idviagem;


WifiLocation location(googleApiKey);

void setup() {
  Serial.begin(9600);
  delay(10);
  pinMode(sensor_entrada, INPUT);
  pinMode(sensor_saida, INPUT);
  pinMode(sensor_conf_entrada, INPUT);
  pinMode(sensor_conf_saida, INPUT);

  Serial.println();
  Serial.println();
  Serial.print("Conectando com ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Conectando em ");
  Serial.println(host);

  EEPROM.begin(4);
  (EEPROM.get(0, idviagem));
  idviagem += 1;
  EEPROM.write(0, idviagem);
  EEPROM.commit();
  EEPROM.end();
}


void localizacao() {
  location_t loc = location.getGeoFromWiFi();

  Serial.println("Location request data");
  Serial.println(location.getSurroundingWiFiJson());
  Serial.println (String(loc.lat, 7) + ", " + String(loc.lon, 7));
  coordenadas = (String(loc.lat, 7) + "," + String(loc.lon, 7));
}

void entradinha() {

  if ((digitalRead(sensor_entrada)) && (digitalRead(sensor_conf_entrada)) == 1)
  {
    entrada += 1;
    banco();
    contador_viagem = 0;
    viagem = "Viagem%20em%20percurso";
  }
  else
  {
    contador_viagem += 1;
  }
}


void saidinha() {
  if ((digitalRead(sensor_saida)) && (digitalRead(sensor_conf_saida)) == 1)
  {
    saida += 1;
    banco();
    contador_viagem = 0;
    viagem = "Viagem%20em%20percurso";
  }
  else
  {
    contador_viagem += 1;
  }
}

void loop() {
  contador_localizacao += 1;
  Serial.print("Contador localização: ");
  Serial.println(contador_localizacao);
  Serial.print("Contador viagem: ");
  Serial.println(contador_viagem);
  Serial.print("idviagem: ");
  Serial.println(idviagem);
  if (contador_localizacao == 60) {
    localizacao();
    contador_localizacao = 0;
  }
  else {

  }

  EEPROM.begin(4);
  contador_viagem += 1;
  if (contador_viagem >= 900)
  {
    EEPROM.write(0, idviagem);
    EEPROM.commit();
    contador_viagem = 0;
    viagem = "Viagem%20finalizada";
    banco();
  }
  else {

  }
  entradinha();
  //Serial.println(entrada);
  saidinha();
  //Serial.println(saida);
  lotacao = entrada - saida;
  entrada = 0;
  saida = 0;
  //Serial.println(lotacao);
  EEPROM.end();
  /*Serial.println("----------------------------------------------------");
  Serial.print("Sensor de entrada: " );
  Serial.println(digitalRead(sensor_entrada));
  Serial.print("Sensor de confirmação entrada: " );
  Serial.println(digitalRead(sensor_conf_entrada));
  Serial.print("Sensor de saída: " );
  Serial.println(digitalRead(sensor_saida));
  Serial.print("Sensor de confirmação saída: " );
  Serial.println(digitalRead(sensor_conf_saida));
  */
  delay(3000);
}

void banco() {
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Falha na conexão");
    return;
  }

  String url = "/informacoes/salvar.php?";
  url += "entrada=";
  url += digitalRead(sensor_entrada);
  url += "&saida=";
  url += digitalRead(sensor_saida);
  url += "&lotacao=";
  url += lotacao;
  url += "&coordenadas=";
  url += coordenadas;
  url += "&viagem=";
  url += viagem;
  url += "&idviagem=";
  url += idviagem;

  Serial.print ("Requisitando URL: ");
  Serial.println(url);


  // This will send a string to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");



  // wait for data to be available
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    //Serial.print(line);
    if (line.indexOf("salvo_com_sucesso") != -1) {
      Serial.println();
      Serial.println("Dados foram salvos com sucesso");

    } else if (line.indexOf("erro_ao_salvar") != -1) {
      Serial.println();
      Serial.println("Ocorreu um erro ao tentar salvar");
    }
  }

  // Close the connection
  Serial.println();
  Serial.println("Conexao fechada");
  client.stop();

  delay(500); // execute once every 5 minutes, don't flood remote service
}