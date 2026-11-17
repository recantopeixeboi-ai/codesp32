#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// CONFIGURAÇÃO DO ID DESTE PAGER (Mude para 1, 2, 3...)
const int MEU_ID = 2; 

#define LARGURA_TELA 128
#define ALTURA_TELA 64
Adafruit_SSD1306 display(LARGURA_TELA, ALTURA_TELA, &Wire, -1);

const int PINO_BUZZER = 19;
const int PINO_VIBRACAO = 2;

typedef struct struct_mensagem {
    int idAlvo;
    int numeroMesa;
} struct_mensagem;

typedef struct struct_resposta {
    int idPager;
    bool recebido;
} struct_resposta;

struct_mensagem dadosRecebidos;
struct_resposta minhaResposta;

void aoReceberDados(const esp_now_recv_info *info, const uint8_t *dados, int tamanho) {
  if (tamanho == sizeof(struct_mensagem)) {
    memcpy(&dadosRecebidos, dados, sizeof(dadosRecebidos));
    
    if (dadosRecebidos.idAlvo == MEU_ID) {
      Serial.println("É para mim! Respondendo à base...Esp32");

      // 1. Envia IMEDIATAMENTE a confirmação de recebimento para a Base antes de travar no loop de vibração
      minhaResposta.idPager = MEU_ID;
      minhaResposta.recebido = true;
      
      // Adiciona temporariamente a Base como peer para responder diretamente para o MAC dela
      esp_now_peer_info_t peerInfo;
      memset(&peerInfo, 0, sizeof(peerInfo));
      memcpy(peerInfo.peer_addr, info->src_addr, 6);
      peerInfo.channel = 1;
      peerInfo.encrypt = false;
      
      if (!esp_now_is_peer_exist(info->src_addr)) {
        esp_now_add_peer(&peerInfo);
      }
      
      // Envia a resposta de sucesso
      esp_now_send(info->src_addr, (uint8_t *) &minhaResposta, sizeof(minhaResposta));

      // 2. Atualiza a Tela OLED
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(10, 10);
      display.print("PAGER "); display.print(MEU_ID);
      display.setCursor(10, 40);
      display.print("MESA: "); display.print(dadosRecebidos.numeroMesa);
      display.display();

      // 3. Alertas físicos
      for(int i = 0; i < 5; i++) {
        digitalWrite(PINO_BUZZER, HIGH);
        digitalWrite(PINO_VIBRACAO, HIGH);
        delay(400);
        digitalWrite(PINO_BUZZER, LOW);
        digitalWrite(PINO_VIBRACAO, LOW);
        delay(400);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(PINO_VIBRACAO, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("Erro OLED");
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.print("PAGER ID:");
  display.setCursor(55, 45);
  display.print(MEU_ID);
  display.display();

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) return;
  
  esp_now_register_recv_cb(aoReceberDados);
}

void loop() {}

