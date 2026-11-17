#include <qrcode_st7735.h>
#include <QRCodeGenerator.h>
//#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <qrcode.h>
#include <SPI.h>
#include <TFT_eSPI.h>
//128x160

#define TFT_RST   -1    
#define TFT_CS    10     
#define TFT_DC   46    

#define TFTMODEL INITR_144GREENTAB

Adafruit_ST77XX tft = Adafruit_ST77XX(TFT_CS, TFT_DC, TFT_RST);
QRcode_ST7735  qrcode(&tft); 

const int botao = 17;
int led = 16;




void setup() {
  Serial.begin(9600);

    tft.init();
    tft.setRotation(0); // Modo Paisagem

    tft.invertDisplay(true);

  pinMode(led,OUTPUT);
 // tft.initR(INITR_BLACKTAB); // Inicializa o display (verifique o tipo de tabulação do seu display)

  tft.fillScreen(TFT_BLACK); // Limpa a tela com preto
  
  // Desenha a seta para baixo
  // Posição inicial (x, y) = (64, 40)
  // Cor: Vermelho (ST7735_RED)
 // drawArrowDown(64, 100, ST7735_WHITE); 
  drawArrowDown(); 
  limpar();
  tft.fillRoundRect(60, 155, 10, 20, 4, TFT_BLUE);
}

void loop() {

drawArrowDown(); 
digitalWrite(led,HIGH);
delay(500);
limpar();
digitalWrite(led,LOW);
delay(500);
  
}

void limpar (){
          tft.fillRect(0, 110, 125, 45, TFT_BLACK);
          uint16_t color = TFT_WHITE ;
            

            // Desenha o corpo da seta (um retângulo vertical)
            // fillRect(x, y, largura, altura, cor)
            //tft.fillRect(x - (size / 4), y, size / 2, size, color);
            tft.fillRect(50, 110, 30, 10, color);

            // Desenha a cabeça da seta (um triângulo preenchido)
            // fillTriangle(x1, y1, x2, y2, x3, y3, cor)
            // Ponto superior esquerdo, superior direito, e ponto inferior do triângulo
          /* tft.fillTriangle(x - (size / 2), y + size, // Ponto 1 (esquerda superior do triângulo)
                            x + (size / 2), y + size, // Ponto 2 (direita superior do triângulo)
                            x, y + size + headSize,   // Ponto 3 (ponta inferior)
                            color);*/

            tft.fillTriangle(34, 120, // Ponto 1 (esquerda superior do triângulo)54
                            94, 120, // Ponto 2 (direita superior do triângulo)74
                            64, 140,   // Ponto 3 (ponta inferior)
                            TFT_WHITE); 

            tft.fillRoundRect(40, 155, 50, 10, 4, TFT_BLUE);
}

// Função para desenhar uma seta preenchida apontando para baixo
//void drawArrowDown(int x, int y, uint16_t color) {
void drawArrowDown() {  
        tft.fillRect(0, 110, 125, 45, TFT5_BLACK);
          int size = 20; // Tamanho da seta (altura e largura aproximadas do corpo)
          int headSize = 20; // Tamanho da cabeça do triângulo
          int x = 64;
          int y = 100;
          uint16_t color =TFT_WHITE ;
          

          // Desenha o corpo da seta (um retângulo vertical)
          // fillRect(x, y, largura, altura, cor)
          //tft.fillRect(x - (size / 4), y, size / 2, size, color);
          tft.fillRect(50, 125, 30, 10, color);

          // Desenha a cabeça da seta (um triângulo preenchido)
          // fillTriangle(x1, y1, x2, y2, x3, y3, cor)
          // Ponto superior esquerdo, superior direito, e ponto inferior do triângulo
        /* tft.fillTriangle(x - (size / 2), y + size, // Ponto 1 (esquerda superior do triângulo)
                          x + (size / 2), y + size, // Ponto 2 (direita superior do triângulo)
                          x, y + size + headSize,   // Ponto 3 (ponta inferior)
                          color);*/

          tft.fillTriangle(34, 135, // Ponto 1 (esquerda superior do triângulo)54
                          94, 135, // Ponto 2 (direita superior do triângulo)74
                          64,  155,   // Ponto 3 (ponta inferior)
                          ST7735_WHITE);  

          tft.fillRoundRect(40, 155, 50, 10, 4, ST7735_BLUE);                                
}