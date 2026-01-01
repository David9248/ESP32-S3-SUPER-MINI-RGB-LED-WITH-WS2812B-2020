#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>
#include <ESPmDNS.h>

#define LED_PIN 4
#define NUM_LEDS 21
CRGB leds[NUM_LEDS];

const char* ssid = "";
const char* password = "";
const char* hostname = ""; // Acesso: http://alvarinho.local

WebServer server(80);

// Estado atual
CRGB cor1 = CRGB(255, 0, 0);
CRGB cor2 = CRGB(255, 0, 255);
int efeitoAtual = 0;
int velocidade = 50;
int intensidade = 100; // Brilho 0-255
unsigned long ultimoUpdate = 0;
uint8_t hue = 0;
int posicao = 0;
int direcao = 1;
uint8_t gHue = 0;

void aplicarEfeito() {
  unsigned long agora = millis();
  
  if (agora - ultimoUpdate < velocidade) return;
  ultimoUpdate = agora;
  
  FastLED.setBrightness(intensidade); // Aplica intensidade
  
  switch(efeitoAtual) {
    // EFEITOS DE COR SÓLIDA
    case 0: // Sólido
      fill_solid(leds, NUM_LEDS, cor1);
      break;
      
    case 1: // Pulso
      {
        uint8_t brilho = beatsin8(60, 50, 255);
        fill_solid(leds, NUM_LEDS, cor1);
        FastLED.setBrightness(map(brilho, 0, 255, 0, intensidade));
      }
      break;
      
    case 2: // Breathing
      {
        uint8_t brilho = beatsin8(30, 30, 255);
        fill_solid(leds, NUM_LEDS, cor1);
        FastLED.setBrightness(map(brilho, 0, 255, 0, intensidade));
      }
      break;
    
    // EFEITOS DE GRADIENTE
    case 3: // Gradiente Estático
      fill_gradient_RGB(leds, 0, cor1, NUM_LEDS-1, cor2);
      break;
      
    case 4: // Gradiente Rotativo
      {
        for(int i = 0; i < NUM_LEDS; i++) {
          uint8_t pos = map(i, 0, NUM_LEDS-1, 0, 255);
          leds[i] = blend(cor1, cor2, pos);
        }
        hue += 2;
      }
      break;
      
    case 5: // Wave Gradiente
      fill_gradient_RGB(leds, 0, cor1, NUM_LEDS-1, cor2);
      for(int i = 0; i < NUM_LEDS; i++) {
        int brilho = sin8(hue + (i * 15));
        leds[i].fadeToBlackBy(255 - brilho);
      }
      hue += 2;
      break;
    
    // EFEITOS RAINBOW
    case 6: // Arco-íris Estático
      fill_rainbow(leds, NUM_LEDS, hue, 255/NUM_LEDS);
      break;
      
    case 7: // Arco-íris Rotativo
      fill_rainbow(leds, NUM_LEDS, hue, 255/NUM_LEDS);
      hue += 2;
      break;
      
    case 8: // Rainbow Wave
      for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(hue + (i * 10), 255, beatsin8(60 + i * 2, 50, 255));
      }
      hue++;
      break;
    
    // EFEITOS DE MOVIMENTO
    case 9: // Chase
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      for(int i = 0; i < 3; i++) {
        int pos = (posicao + i * 7) % NUM_LEDS;
        leds[pos] = cor1;
      }
      posicao++;
      if(posicao >= NUM_LEDS) posicao = 0;
      break;
      
    case 10: // Bounce
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      for(int i = 0; i < 3; i++) {
        if(posicao + i < NUM_LEDS && posicao + i >= 0) {
          leds[posicao + i] = cor1;
        }
      }
      posicao += direcao;
      if(posicao >= NUM_LEDS - 3 || posicao <= 0) direcao *= -1;
      break;
      
    case 11: // Scanner (Kitt)
      fadeToBlackBy(leds, NUM_LEDS, 80);
      leds[posicao] = cor1;
      if(posicao > 0) leds[posicao - 1] = cor1;
      if(posicao < NUM_LEDS - 1) leds[posicao + 1] = cor1;
      posicao += direcao;
      if(posicao >= NUM_LEDS - 1 || posicao <= 0) direcao *= -1;
      break;
      
    case 12: // Theater
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      for(int i = posicao; i < NUM_LEDS; i += 3) {
        leds[i] = cor1;
      }
      posicao++;
      if(posicao >= 3) posicao = 0;
      break;
      
    case 13: // Rainbow Chase
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      for(int i = 0; i < 5; i++) {
        int pos = (posicao + i * 4) % NUM_LEDS;
        leds[pos] = CHSV(hue + (i * 40), 255, 255);
      }
      posicao++;
      hue += 2;
      break;
      
    case 14: // Comet
      fadeToBlackBy(leds, NUM_LEDS, 64);
      leds[posicao] = CHSV(hue, 255, 255);
      posicao++;
      if(posicao >= NUM_LEDS) {
        posicao = 0;
        hue += 16;
      }
      break;
    
    // EFEITOS ESPECIAIS
    case 15: // Sparkle
      {
        fadeToBlackBy(leds, NUM_LEDS, 30);
        int pos = random16(NUM_LEDS);
        leds[pos] = CRGB::White;
      }
      break;
      
    case 16: // Fire
      for(int i = 0; i < NUM_LEDS; i++) {
        int heat = random8(50, 255);
        leds[i] = CRGB(heat, heat / 3, 0);
      }
      break;
      
    case 17: // Confetti
      fadeToBlackBy(leds, NUM_LEDS, 20);
      {
        int pos = random16(NUM_LEDS);
        leds[pos] += CHSV(gHue + random8(64), 200, 255);
      }
      gHue++;
      break;
      
    case 18: // Sinelon
      fadeToBlackBy(leds, NUM_LEDS, 20);
      {
        int pos = beatsin16(13, 0, NUM_LEDS - 1);
        leds[pos] += CHSV(gHue, 255, 192);
      }
      gHue++;
      break;
      
    case 19: // Juggle
      fadeToBlackBy(leds, NUM_LEDS, 20);
      for(int i = 0; i < 4; i++) {
        leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(gHue + (i * 32), 200, 255);
      }
      gHue++;
      break;
      
    case 20: // BPM
      {
        uint8_t BeatsPerMinute = 62;
        uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
        for(int i = 0; i < NUM_LEDS; i++) {
          leds[i] = ColorFromPalette(RainbowColors_p, gHue + (i * 2), beat - gHue + (i * 10));
        }
      }
      gHue++;
      break;
      
    case 21: // Police
      {
        int meio = NUM_LEDS / 2;
        if((millis() / 100) % 2) {
          fill_solid(leds, meio, CRGB::Red);
          fill_solid(leds + meio, NUM_LEDS - meio, CRGB::Blue);
        } else {
          fill_solid(leds, meio, CRGB::Blue);
          fill_solid(leds + meio, NUM_LEDS - meio, CRGB::Red);
        }
      }
      break;
      
    case 22: // Strobe
      if((millis() / 50) % 2) {
        fill_solid(leds, NUM_LEDS, cor1);
      } else {
        fill_solid(leds, NUM_LEDS, CRGB::Black);
      }
      break;
  }
  
  FastLED.show();
}

void setup() {
  Serial.begin(115200);
  
  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(intensidade);
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 255));
  FastLED.show();
  delay(500);
  
  fill_solid(leds, NUM_LEDS, CRGB(255, 255, 0));
  FastLED.show();
  
  WiFi.begin(ssid, password);
  WiFi.setHostname(hostname);
  
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 30) {
    fill_solid(leds, NUM_LEDS, (tentativas % 2) ? CRGB(255, 255, 0) : CRGB(0, 0, 0));
    FastLED.show();
    delay(500);
    tentativas++;
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    // Iniciar mDNS
    if (MDNS.begin(hostname)) {
      Serial.print("mDNS iniciado: http://");
      Serial.print(hostname);
      Serial.println(".local");
      MDNS.addService("http", "tcp", 80);
    } else {
      Serial.println("Erro ao iniciar mDNS");
    }
    
    for(int i = 0; i < 3; i++) {
      fill_solid(leds, NUM_LEDS, CRGB(0, 255, 0));
      FastLED.show();
      delay(200);
      fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
      FastLED.show();
      delay(200);
    }
  } else {
    Serial.println("\nFalha WiFi!");
    fill_solid(leds, NUM_LEDS, CRGB(255, 0, 0));
    FastLED.show();
    while(1) delay(1000);
  }
  
  server.on("/", []() {
    String html = R"(
<!DOCTYPE html>
<html>
<head>
<meta charset='utf-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>ALVARINHO LED</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{background:#0a0a0a;color:#fff;font-family:system-ui,-apple-system,sans-serif;padding:20px;overflow-x:hidden}
.container{max-width:600px;margin:0 auto}
.logo{text-align:center;margin-bottom:30px;padding:20px;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);border-radius:15px;box-shadow:0 10px 30px rgba(102,126,234,0.3)}
.logo h1{font-size:clamp(20px,5vw,32px);font-weight:900;margin-bottom:10px;text-shadow:2px 2px 4px rgba(0,0,0,0.3);letter-spacing:2px}
.emoji-line{font-size:clamp(18px,4vw,28px);letter-spacing:5px;animation:float 3s ease-in-out infinite}
@keyframes float{0%,100%{transform:translateY(0)}50%{transform:translateY(-10px)}}
.section{background:#1a1a1a;border-radius:12px;padding:20px;margin-bottom:20px}
.section h2{font-size:16px;margin-bottom:15px;color:#60a5fa;text-transform:uppercase;letter-spacing:1px}
.color-row{display:flex;gap:10px;margin-bottom:15px;align-items:center}
.color-picker{flex:1;height:50px;border:none;border-radius:8px;cursor:pointer;background:#2a2a2a}
.color-picker::-webkit-color-swatch-wrapper{padding:0}
.color-picker::-webkit-color-swatch{border:none;border-radius:8px}
.effect-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(140px,1fr));gap:8px}
.btn-effect{padding:12px;border:none;border-radius:8px;font-size:12px;font-weight:600;cursor:pointer;background:#2a2a2a;color:#fff;transition:.2s;text-align:left}
.btn-effect:active{transform:scale(.95)}
.btn-effect:hover{background:#3a3a3a}
.category{margin-bottom:20px}
.category-title{font-size:13px;color:#9ca3af;margin-bottom:10px;font-weight:600}
.slider-group{margin-top:15px}
.slider-container{background:#2a2a2a;padding:15px;border-radius:8px;margin-bottom:10px}
input[type=range]{width:100%;height:6px;border-radius:3px;background:#1a1a1a;outline:none;-webkit-appearance:none}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:18px;height:18px;border-radius:50%;background:#60a5fa;cursor:pointer}
.speed-label{display:flex;justify-content:space-between;margin-top:8px;font-size:13px;color:#6b7280}
.value-display{text-align:center;color:#60a5fa;font-weight:700;font-size:18px;margin-top:5px}
</style>
</head>
<body>
<div class='container'>

<div class='logo'>
<h1>ALVARINHO = TÓTÓ</h1>
<div class='emoji-line'>🍷✨🎉🔥💫🌟⚡🎊🎈🍾🥳🎭🎪🎨🌈</div>
</div>

<div class='section'>
<h2>🎨 Cores Personalizadas</h2>
<div class='color-row'>
<input type='color' id='c1' class='color-picker' value='#ff0000'>
<input type='color' id='c2' class='color-picker' value='#ff00ff'>
</div>
</div>

<div class='section'>
<h2>⚙️ Controles</h2>

<div class='slider-container'>
<div class='category-title'>💡 Intensidade (Brilho)</div>
<input type='range' id='int' min='5' max='255' value='100' oninput='setInt(this.value)'>
<div class='value-display' id='intVal'>100</div>
</div>

<div class='slider-container'>
<div class='category-title'>⚡ Velocidade</div>
<input type='range' id='vel' min='10' max='200' value='50' oninput='setVel(this.value)'>
<div class='speed-label'><span>Rápido</span><span>Lento</span></div>
</div>

</div>

<div class='section'>
<h2>✨ Efeitos LED</h2>

<div class='category'>
<div class='category-title'>🔵 Cor Sólida</div>
<div class='effect-grid'>
<button class='btn-effect' onclick='setEfeito(0)'>Sólido</button>
<button class='btn-effect' onclick='setEfeito(1)'>Pulso</button>
<button class='btn-effect' onclick='setEfeito(2)'>Breathing</button>
</div>
</div>

<div class='category'>
<div class='category-title'>🌈 Gradientes</div>
<div class='effect-grid'>
<button class='btn-effect' onclick='setEfeito(3)'>Gradiente</button>
<button class='btn-effect' onclick='setEfeito(4)'>Grad Rotativo</button>
<button class='btn-effect' onclick='setEfeito(5)'>Wave Grad</button>
</div>
</div>

<div class='category'>
<div class='category-title'>🌟 Rainbow</div>
<div class='effect-grid'>
<button class='btn-effect' onclick='setEfeito(6)'>Rainbow</button>
<button class='btn-effect' onclick='setEfeito(7)'>Rainbow Rota</button>
<button class='btn-effect' onclick='setEfeito(8)'>Rainbow Wave</button>
</div>
</div>

<div class='category'>
<div class='category-title'>⚡ Movimento</div>
<div class='effect-grid'>
<button class='btn-effect' onclick='setEfeito(9)'>Chase</button>
<button class='btn-effect' onclick='setEfeito(10)'>Bounce</button>
<button class='btn-effect' onclick='setEfeito(11)'>Scanner</button>
<button class='btn-effect' onclick='setEfeito(12)'>Theater</button>
<button class='btn-effect' onclick='setEfeito(13)'>Rainbow Chase</button>
<button class='btn-effect' onclick='setEfeito(14)'>Comet</button>
</div>
</div>

<div class='category'>
<div class='category-title'>🔥 Especiais</div>
<div class='effect-grid'>
<button class='btn-effect' onclick='setEfeito(15)'>Sparkle</button>
<button class='btn-effect' onclick='setEfeito(16)'>Fire</button>
<button class='btn-effect' onclick='setEfeito(17)'>Confetti</button>
<button class='btn-effect' onclick='setEfeito(18)'>Sinelon</button>
<button class='btn-effect' onclick='setEfeito(19)'>Juggle</button>
<button class='btn-effect' onclick='setEfeito(20)'>BPM</button>
<button class='btn-effect' onclick='setEfeito(21)'>Police</button>
<button class='btn-effect' onclick='setEfeito(22)'>Strobe</button>
</div>
</div>

</div>

</div>

<script>
let c1=document.getElementById('c1');
let c2=document.getElementById('c2');
let intSlider=document.getElementById('int');
let intVal=document.getElementById('intVal');

c1.oninput=()=>fetch(`/cor1?hex=${c1.value.substring(1)}`);
c2.oninput=()=>fetch(`/cor2?hex=${c2.value.substring(1)}`);

function setEfeito(e){fetch(`/efeito?id=${e}`)}
function setVel(v){fetch(`/vel?v=${v}`)}
function setInt(i){
  intVal.textContent=i;
  fetch(`/int?v=${i}`)
}
</script>
</body>
</html>
)";
    server.send(200, "text/html", html);
  });
  
  server.on("/cor1", []() {
    if(server.hasArg("hex")) {
      String hex = server.arg("hex");
      long rgb = strtol(hex.c_str(), NULL, 16);
      cor1 = CRGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
    }
    server.send(200, "text/plain", "OK");
  });
  
  server.on("/cor2", []() {
    if(server.hasArg("hex")) {
      String hex = server.arg("hex");
      long rgb = strtol(hex.c_str(), NULL, 16);
      cor2 = CRGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
    }
    server.send(200, "text/plain", "OK");
  });
  
  server.on("/efeito", []() {
    if(server.hasArg("id")) {
      efeitoAtual = server.arg("id").toInt();
      posicao = 0;
      direcao = 1;
    }
    server.send(200, "text/plain", "OK");
  });
  
  server.on("/vel", []() {
    if(server.hasArg("v")) {
      velocidade = server.arg("v").toInt();
    }
    server.send(200, "text/plain", "OK");
  });
  
  server.on("/int", []() {
    if(server.hasArg("v")) {
      intensidade = server.arg("v").toInt();
      FastLED.setBrightness(intensidade);
    }
    server.send(200, "text/plain", "OK");
  });
  
  server.begin();
  
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 255));
  FastLED.show();
}

void loop() {
  server.handleClient();
  aplicarEfeito();
  // mDNS no ESP32 não precisa de update no loop!
}