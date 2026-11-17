void setup() {
#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Estruturas de dados
typedef struct struct_mensagem {
    int idAlvo;
    int numeroMesa;
} struct_mensagem;

typedef struct struct_resposta {
    int idPager;
    bool recebido;
} struct_resposta;

struct_mensagem meuDado;
struct_resposta respostaPager;
WebServer server(80);

// Variáveis de controle de status
int ultimoPagerChamado = 0;
String statusAtual = "Aguardando comando...";
String classeStatus = "status-neutral";

// Página Web dinâmica com Javascript que monitora o retorno
const char* html_pagina = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Painel de Controle - Pagers</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f9; margin: 0; padding: 20px; }
        h1 { color: #333; }
        .container { max-width: 500px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
        .grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 15px; margin-top: 20px; }
        button { background-color: #007BFF; color: white; border: none; padding: 15px; font-size: 16px; border-radius: 5px; cursor: pointer; transition: 0.3s; }
        button:hover { background-color: #0056b3; }
        .input-group { margin-top: 20px; display: flex; justify-content: center; gap: 10px; }
        input { padding: 10px; font-size: 16px; width: 80px; text-align: center; border: 1px solid #ccc; border-radius: 5px; }
        
        /* Classes de Feedback Visual */
        .status-box { margin-top: 25px; padding: 15px; border-radius: 5px; font-weight: bold; font-size: 16px; transition: 0.5s; }
        .status-neutral { background-color: #e2e3e5; color: #383d41; }
        .status-sending { background-color: #fff3cd; color: #856404; animation: blink 1s infinite; }
        .status-success { background-color: #d4edda; color: #155724; border: 2px solid #c3e6cb; }
        @keyframes blink { 50% { opacity: 0.6; } }
    </style>
</head>
<body>
    <div class="container">
        <h1>Chamar Pager</h1>
        
        <div class="input-group">
            <label for="mesa" style="font-size: 18px; align-self: center;">Mesa Ocupada:</label>
            <input type="number" id="mesa" value="1" min="1">
        </div>

        <div class="grid">
            <button onclick="chamar(1)">Chamar Pager 1</button>
            <button onclick="chamar(2)">Chamar Pager 2</button>
            <button onclick="chamar(3)">Chamar Pager 3</button>
            <button onclick="chamar(4)">Chamar Pager 4</button>
            <button onclick="chamar(5)">Chamar Pager 5</button>
            <button onclick="chamar(6)">Chamar Pager 6</button>
            <button onclick="chamar(7)">Chamar Pager 7</button>
            <button onclick="chamar(8)">Chamar Pager 8</button>
            <button onclick="chamar(9)">Chamar Pager 9</button>
            <button onclick="chamar(10)">Chamar Pager 10</button>
            
        </div>

        <!-- Caixa de Confirmação Visual -->
        <div id="statusBox" class="status-box status-neutral">Aguardando comando...</div>
    </div>

    <script>
        function chamar(idPager) {
            let numMesa = document.getElementById('mesa').value;
            document.getElementById('statusBox').innerText = "Chamando Pager " + idPager + "...";
            document.getElementById('statusBox').className = "status-box status-sending";
            
            fetch(`/chamar?id=${idPager}&mesa=${numMesa}`);
        }

        // Função que roda em segundo plano atualizando a tela
        setInterval(function() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    let box = document.getElementById('statusBox');
                    box.innerText = data.mensagem;
                    box.className = "status-box " + data.classe;
                });
        }, 1000); // Checa o retorno a cada 1 segundo
    </script>
</body>
</html>
)rawliteral";

// Função executada quando o Pager responde de volta à Base
void aoReceberResposta(const esp_now_recv_info *info, const uint8_t *dados, int tamanho) {
  if (tamanho == sizeof(struct_resposta)) {
    memcpy(&respostaPager, dados, sizeof(respostaPager));
    
    if (respostaPager.idPager == ultimoPagerChamado && respostaPager.recebido) {
      statusAtual = "✅ SUCESSO: Pager " + String(respostaPager.idPager) + " recebeu e está tocando!";
      classeStatus = "status-success";
    }
  }
}

void tratarChamada() {
  if (server.hasArg("id") && server.hasArg("mesa")) {
    meuDado.idAlvo = server.arg("id").toInt();
    meuDado.numeroMesa = server.arg("mesa").toInt();
    ultimoPagerChamado = meuDado.idAlvo;

    statusAtual = "⏳ Enviando sinal para o Pager " + String(ultimoPagerChamado) + "...";
    classeStatus = "status-sending";

    esp_now_send(broadcastMac, (uint8_t *) &meuDado, sizeof(meuDado));
    server.send(200, "text/plain", "OK");
  }
}

void enviarStatus() {
  // Retorna um JSON para a página web ler facilmente
  String json = "{\"mensagem\":\"" + statusAtual + "\", \"classe\":\"" + classeStatus + "\"}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("Base_Pagers", "12345678", 1); 
  delay(100);

  if (esp_now_init() != ESP_OK) return;

  // Configura canal de envio (Broadcast)
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastMac, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;
  esp_now_add_peer(&peerInfo);

  // Registra a função para ouvir o retorno dos pagers
  esp_now_register_recv_cb(aoReceberResposta);

  // Rotas HTTP
  server.on("/", []() { server.send(200, "text/html", html_pagina); });
  server.on("/chamar", tratarChamada);
  server.on("/status", enviarStatus);

  server.begin();
}

void loop() {
  server.handleClient();
}

