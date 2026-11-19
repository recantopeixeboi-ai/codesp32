//carregado 01/10/2026 EM  23/05/2026  em 26/02/2026 ultimo garregamento

#include <qrcode_st7735.h>
#include <QRCodeGenerator.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <qrcode.h>
#include <Pushbutton.h>



// Configurações do Display ST7735
#define TFT_CS    5    // 5 
#define TFT_DC    15   // 15
#define TFT_RST   19   // 19
#define TFT_SDA   23   // 23
#define TFT_SCL   18   // 18
#define TFTMODEL INITR_144GREENTAB
/*
#define TFT_CS    5
#define TFT_DC    17
#define TFT_RST   0
#define TFT_SDA   23
#define TFT_SCL   18
#define TFTMODEL INITR_144GREENTAB
*/

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

QRcode_ST7735  qrcode(&tft); 

// Configurações do Botão e LED
#define BUTTON_PIN 13
#define CREDITO_PIN 12  //12   40f5
#define LED_PIN    2  //2
#define  buzzer   21
Pushbutton button(BUTTON_PIN);

// Configurações do WiFi
const char* ssid = "DELTA_FIBRA_EMPORIO";
const char* password = "82520519";

// Configurações do Mercado Pago
String accessToken = "APP_USR-869554720330327-02070-773151d2e61c73be0c63ff99d00ea4d-71134490";//monica
//String accessToken = "APP_USR-236461546714171-04131-17880727991cd28016603b281693e34-25560973";//james
String userId = "SEU_USER_ID_MERCADO_PAGO";
//APP_USR-4344905239967248-020614-9cf10fe900ddb3836f91e23f47d9746f-3184584858
// Variáveis globais
float valor = 0.00;
float credito = 0;
unsigned long ultimoIncremento = 0;
unsigned long ultimoQRCode = 0;
unsigned long ultimoStatusCheck = 0;
unsigned long tempoSeta = 0;
String qrCodeData = "";
String paymentId = "";
String status = "";
bool pagamentoAprovado = false;
bool mudaSeta = false;
int cont =1;
bool tempoqr = true ;
bool inicio = true;


// Estrutura para desenhar o QR Code no display


// Inicialização do Display
void setupDisplay() {
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setRotation(0);
  tft.setTextSize(1);
  tft.setCursor(20,10);
  tft.println("APERTE O BOTAO");
  tft.setCursor(0,30);
  tft.println("para adicionar Valor");
  tft.setCursor(20,50);
  tft.println(" cada aperto");
  tft.setCursor(0,70);
  tft.println("aumenta 0.50 centavos");
  tft.fillRoundRect(0,150,128,5,0,ST77XX_BLACK);
  
  
}

// Função para desenhar o QR Code no display
void drawQRCode() {
  qrcode.init();
  qrcode.create(qrCodeData);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 120);
  tft.println("R$: " + String(valor, 2));

  tft.setCursor(20, 140);
  tft.setTextSize(1);
  tft.println("APERTE O BOTAO");
  tft.setCursor(20, 150);
  
  tft.println("PARA CANCELAR");

  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.println("ESCANEIE O QRCODE");


  Serial.println("qrCodeData");
  Serial.println(qrCodeData);
  Serial.println("paymentId");
  Serial.println(paymentId);
  Serial.println("status");
  Serial.println(status);
  ultimoQRCode = millis();
  Serial.println(qrCodeData);
 
}

// Função para gerar QR Code de pagamento 9324
void gerarQRCode() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    //String url ="https://api.mercadopago.com";
    String url = "https://api.mercadopago.com/v1/payments";
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + accessToken);
    http.addHeader("Content-Type", "application/json");
    String idempotency = String(millis());
    http.addHeader("X-Idempotency-Key", idempotency);

    String payload = "{\"transaction_amount\":"+ String(valor) +",\"description\": \"Título do produto\",\"payment_method_id\": \"pix\",\"payer\":{\"email\": \"monaraavelino@gmail.com\",\"first_name\": \"maria\",\"last_name\": \"jose\"}}";
    //,\"identification\":{\"type\": \"CPF\",\"number\": \"96282878187\"},\"address\": {\"zip_code\": \"7447382\",\"street_name\": \"Av.oriente\",\"street_number\": \"303\",\"neighborhood\": \"morada do sol\",\"city\": \"goiania\",\"federal_unit\": \"go\"}}}";
    //"{\"transaction_amount\": 1.00, \"description\": \"Produto de Teste\", \"payment_method_id\": \"pix\"}";

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
      String response = http.getString();
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, response);
      qrCodeData = doc["point_of_interaction"]["transaction_data"]["qr_code"].as<String>();
      //doc["qr_data"].as<String>();
      paymentId = doc["id"].as<String>();
      
      tft.setCursor(30, 0);
      tft.println(paymentId);
      drawQRCode();
     // qrcode.create(qrCodeData);
    } else {
      tft.fillScreen(ST77XX_BLACK);
      tft.fillScreen(ST77XX_RED);
      tft.setTextSize(2);
      tft.setCursor(20, 50);
      tft.println("Erro ao gerar");
      tft.setCursor(20, 80);
      tft.println("QR Code");
    }
    http.end();
    
  }
  // liberaCredito();
  
}

// Função para verificar status do pagamento
void verificarStatusPagamento() {
  if (paymentId != "" && WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.mercadopago.com/v1/payments/" + paymentId;
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + accessToken);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, response);
      String status = doc["status"].as<String>();
      Serial.println(status);
        

      if (status == "approved") {
        pagamentoAprovado = true;
        digitalWrite(LED_PIN, HIGH);
        tft.fillScreen(ST77XX_GREEN);
        tft.setTextSize(2);
        tft.setCursor(10, 30);
        tft.setTextColor(ST77XX_BLACK);
        tft.println("PAGAMENTO");
        tft.setCursor(10, 60);
        tft.println("APROVADO!");
        liberaCredito();
        delay(9000);
      }
    }
    http.end();
  }
}


// notes in the melody:


// note durations: 4 = quarter note, 8 = eighth note, etc.:


void setup() {
  

  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(CREDITO_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(CREDITO_PIN, LOW);

   for(int i=0;i<3;i++){  
      tone(buzzer,700,200);
      delay(200);
      noTone(buzzer); 
      delay(200);  
    }
  
  WiFi.begin(ssid, password);
  tft.initR(INITR_BLACKTAB);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    //Serial.println("Conectando ao WiFi...");
    digitalWrite(LED_PIN, LOW);
    delay(1000);
      //tft.setRotation(2);
      tft.fillScreen(ST77XX_RED);
      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(2);
      tft.setCursor(24, 20);
      tft.println("  SEM");//semconexao"
      tft.setCursor(1, 60);
      tft.println(" INTERNETE");//" Ao WiFi"
      tft.setCursor(18, 100);
      tft.println("  USE");
      tft.setCursor(8, 130);
      tft.println("  MOEDAS");
    
  }
  Serial.println("Conectado ao WiFi");

  setupDisplay();
  drawArrowDown(); 
  limpar();
  seta();
  conectaWifi();
 // melodia();
 // liberaCredito();
  
}
void conectaWifi() {

 while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
     digitalWrite(LED_PIN, HIGH);
    delay(1000);
    //Serial.println("Conectando ao WiFi...");
    digitalWrite(LED_PIN, LOW);
    delay(1000);
      //tft.setRotation(2);
      tft.fillScreen(ST77XX_RED);
      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(2);
      tft.setCursor(24, 20);
      tft.println("  SEM");//semconexao"
      tft.setCursor(1, 60);
      tft.println(" INTERNETE");//" Ao WiFi"
      tft.setCursor(18, 100);
      tft.println("  USE");
      tft.setCursor(8, 130);
      tft.println("  MOEDAS");
  }
    if(WiFi.status() == WL_CONNECTED){
    //tft.fillScreen(ST77XX_WHITE);
    setupDisplay();
    digitalWrite(LED_PIN, HIGH);
  }

}

void loop() {
    
 // Serial.println(digitalRead(BUTTON_PIN));
 if(millis() - tempoSeta >= 500){
   mudaSeta = !mudaSeta;
   tempoSeta = millis();
  // Serial.println(mudaSeta);
   //cont ++;
  // Serial.println(cont);
   if(tempoqr == true && inicio == true ){ //valor <= 0
      if(mudaSeta){
        drawArrowDown(); 
      }else{
        limpar();
      }
    } 
  }

  void seta();
 /* if(valor <= 0){
 drawArrowDown(); 
 delay(500);
 limpar();
 delay(500);
  }else{
  tft.fillRect(0, 110, 125, 10, ST7735_BLACK);
   }*/
  // Verifica se o botão foi pressionado
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // Debounce
    if (digitalRead(BUTTON_PIN) == LOW  && tempoqr == true) {
      tone(buzzer,700,200);
      tft.fillRect(0, 110, 125, 10, ST7735_BLACK);
      valor += 0.50;
      inicio = false;
      qrCodeData = "";
      if(valor > 5){
        valor = 0.50;
      }
      credito = valor/0.25;
      // credito = valor/0.25;
     
      Serial.print("Valor atual: ");
      Serial.println(valor);
      Serial.print("credito atual: ");
      Serial.println(valor);
      digitalWrite(LED_PIN, LOW);
     // tft.fillScreen(ST77XX_BLACK);
      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(1);
      tft.setCursor(20, 100);
      tft.println("Valor atual:");
     // tft.setCursor(15, 100);
      //tft.println("R$:");
      tft.setTextSize(2);

      tft.fillRoundRect(0,120,120,36,2,ST77XX_BLACK);//36
      tft.setCursor(30, 120);
      //tft.println(String(valor, 2));
     // tft.println("R$: " + String(credito, 2));
       tft.println(valor);
      ultimoIncremento = millis();
      while (digitalRead(BUTTON_PIN) == LOW); // Aguarda soltar o botão
    
   }
   
  }

  // Gera QR Code 10 segundos após o último incremento //10000
  if (millis() - ultimoIncremento >= 5000 && valor > 0 && qrCodeData == ""  ) {
    //gerarQRCode();
    gerar();
    //liberaCredito();
    
    ultimoQRCode = millis();
    tempoqr = false;

  }

  // Verifica status do pagamento 30 segundos após gerar QR Code
  if (millis() - ultimoQRCode >= 20000 && qrCodeData != "" && !pagamentoAprovado) {
    verificarStatusPagamento();
  }

  // Limpa a tela após pagamento aprovado
  if (pagamentoAprovado && millis() - ultimoQRCode >= 6000) {
    valor = 0.00;
    qrCodeData = "";
    paymentId = "";
    pagamentoAprovado = false;
    tempoqr = true;
    inicio = true;
    digitalWrite(LED_PIN, LOW);
    setupDisplay();
    
  }
   // voltar para a tela inivcial
  if(qrCodeData != "" && millis() - ultimoQRCode >= 180000 && valor >0 && tempoqr == false){
    //credito = valor;
    valor = 0.00;
    qrCodeData = "";
    paymentId = "";
    pagamentoAprovado = false;
    tempoqr = true;
    inicio = true;
    tft.fillScreen(ST77XX_BLACK);
    setupDisplay();  
  }
  /*
  if(millis() - ultimoQRCode >= 20000 && qrCodeData != ""){
    // tft.fillRoundRect(0,5,128,5,0,ST77XX_WHITE);
     tft.setTextColor(ST77XX_RED);
     tft.setCursor(1, 150);
     tft.println("Aguardando pagamento...");
    }*/
  
  if (WiFi.status() != WL_CONNECTED){
    conectaWifi();

  }

    if (digitalRead(BUTTON_PIN) == LOW && tempoqr == false && qrCodeData != "") {
      valor = 0;
      delay(50);
      qrCodeData = "";
      tempoqr = true;
      inicio = true;
      tft.fillScreen(ST77XX_BLACK);
      setupDisplay();
   
    
  }   
}



void limpar (){
 tft.fillRect(0, 110, 125, 45, ST7735_BLACK);
 uint16_t color = ST7735_WHITE ;

  tft.fillRect(50, 110, 30, 10, color);

  tft.fillTriangle(34, 120, // Ponto 1 (esquerda superior do triângulo)54
                   94, 120, // Ponto 2 (direita superior do triângulo)74
                   64, 140,   // Ponto 3 (ponta inferior)
                   ST7735_WHITE);

  
  tft.fillRoundRect(40, 155, 50, 10, 4, ST7735_BLUE);
}

void drawArrowDown() {  
  tft.fillRect(0, 110, 125, 45, ST7735_BLACK);
  int size = 20; // Tamanho da seta (altura e largura aproximadas do corpo)
  int headSize = 20; // Tamanho da cabeça do triângulo
  int x = 64;
  int y = 100;
  uint16_t color = ST7735_WHITE ;
  
  tft.fillRect(50, 125, 30, 10, color);

  tft.fillTriangle(34, 135, // Ponto 1 (esquerda superior do triângulo)54
                   94, 135, // Ponto 2 (direita superior do triângulo)74
                   64,  155,   // Ponto 3 (ponta inferior)
                   ST7735_WHITE);  

 // tft.fillRect(0, 110, 125, 45, ST7735_BLACK);
  tft.fillRoundRect(40, 155, 50, 10, 4, ST7735_BLUE);                                
}
void seta(){}
void liberaCredito(){
 
  while(credito>0){
    digitalWrite(LED_PIN,HIGH);
    digitalWrite(CREDITO_PIN,HIGH);
    tone(buzzer,900,200);
    delay(100);
    noTone(buzzer);
    digitalWrite(LED_PIN,LOW);
    digitalWrite(CREDITO_PIN,LOW);
    delay(100);
    credito--;
    Serial.println("Passeio aquir");
   }
  for(int i=0;i<3;i++){  

    tone(buzzer,700,200);
    delay(100);
    noTone(buzzer); 
    delay(100);    
  
  } 


   
}
void gerar(){
  qrCodeData = "00020126360014br.gov.bcb.pix0114+556298611353152040000530398654040.505802BR5925MONICAADRANAAVELINASOBRIN6006Goinia62250521mpqrinter18089786797163045449";
  qrcode.init();
  qrcode.create(qrCodeData);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 120);
  tft.println("R$: " + String(valor, 2));

  tft.setCursor(20, 140);
  tft.setTextSize(1);
  tft.println("APERTE O BOTAO");
  tft.setCursor(20, 150);
  
  tft.println("PARA CANCELAR");

  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.println("ESCANEIE O QRCODE");


  Serial.println("qrCodeData");
  Serial.println(qrCodeData);
  Serial.println("paymentId");
  Serial.println(paymentId);
  Serial.println("status");
  Serial.println(status);
  ultimoQRCode = millis();
  Serial.println(qrCodeData);
 
}
/*
paymentId
180897867971
status

00020126360014br.gov.bcb.pix0114+556298611353152040000530398654040.505802BR5925MONICAADRANAAVELINASOBRIN6006Goinia62250521mpqrinter18089786797163045449
*/
                         
