/* ============================================================================
   DIGICAM-8000  ·  ESP32-CAM retro point-and-shoot
   ========================================================================== */

#include "esp_camera.h"
#include <WiFi.h>
#include "FS.h"
#include "SD_MMC.h"
#include "esp_http_server.h"
#include <Preferences.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "index.h"          // PAGE_INDEX — HTML UI

/* ------------------------------ USER CONFIG ------------------------------ */

const char *AP_SSID  = "DIGICAM-8000";
const char *AP_PASS  = "shutter123";  // min 8 chars

IPAddress   AP_IP     (192, 168, 4, 1);
IPAddress   AP_GATEWAY(192, 168, 4, 1);
IPAddress   AP_SUBNET (255, 255, 255, 0);
#define     AP_CHANNEL     6
#define     AP_MAX_CLIENTS 4

#define SHUTTER_PIN     13            
#define LAMP_PIN         4            
#define LAMP_LEDC_CH     5            

#define DISABLE_BROWNOUT false

/* --------------------------- AI-THINKER PINOUT --------------------------- */

#define PWDN_GPIO_NUM   32
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM    0
#define SIOD_GPIO_NUM   26
#define SIOC_GPIO_NUM   27
#define Y9_GPIO_NUM     35
#define Y8_GPIO_NUM     34
#define Y7_GPIO_NUM     39
#define Y6_GPIO_NUM     36
#define Y5_GPIO_NUM     21
#define Y4_GPIO_NUM     19
#define Y3_GPIO_NUM     18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM  25
#define HREF_GPIO_NUM   23
#define PCLK_GPIO_NUM   22

/* ------------------------- LEDC COMPAT (core 2/3) ------------------------ */

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  #define LAMP_SETUP()   ledcAttachChannel(LAMP_PIN, 5000, 8, LAMP_LEDC_CH)
  #define LAMP_WRITE(d)  ledcWrite(LAMP_PIN, (d))
#else
  #define LAMP_SETUP()   do { ledcSetup(LAMP_LEDC_CH, 5000, 8); \
                              ledcAttachPin(LAMP_PIN, LAMP_LEDC_CH); } while (0)
  #define LAMP_WRITE(d)  ledcWrite(LAMP_LEDC_CH, (d))
#endif

/* ------------------------------- GLOBALS --------------------------------- */

Preferences prefs;

httpd_handle_t uiServer     = NULL;
httpd_handle_t streamServer = NULL;

volatile bool streamPaused = false;
volatile bool busy         = false;

uint32_t shotCounter = 1;
bool     sdReady     = false;

struct Settings {
  uint8_t  photoSize;
  uint8_t  liveSize;
  uint8_t  lampLevel;
  bool     lampOn;
} cfg;

/* ------------------------------- HELPERS --------------------------------- */

static bool getParam(httpd_req_t *req, const char *key, char *out, size_t outLen) {
  size_t qlen = httpd_req_get_url_query_len(req) + 1;
  if (qlen <= 1) return false;
  char *buf = (char *)malloc(qlen);
  if (!buf) return false;
  bool ok = false;
  if (httpd_req_get_url_query_str(req, buf, qlen) == ESP_OK) {
    if (httpd_query_key_value(buf, key, out, outLen) == ESP_OK) ok = true;
  }
  free(buf);
  return ok;
}

static void sendJson(httpd_req_t *req, const String &body) {
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_send(req, body.c_str(), body.length());
}

static void saveSettings() {
  prefs.begin("digicam", false);
  prefs.putUChar("photoSize", cfg.photoSize);
  prefs.putUChar("liveSize",  cfg.liveSize);
  prefs.putUChar("lampLevel", cfg.lampLevel);
  prefs.putBool ("lampOn",    cfg.lampOn);
  prefs.putUInt ("shot",      shotCounter);
  prefs.end();
}

static void loadSettings() {
  prefs.begin("digicam", true);
  cfg.photoSize = prefs.getUChar("photoSize", FRAMESIZE_UXGA);
  cfg.liveSize  = prefs.getUChar("liveSize",  FRAMESIZE_SVGA);
  cfg.lampLevel = prefs.getUChar("lampLevel", 120);
  cfg.lampOn    = prefs.getBool ("lampOn",    false);
  shotCounter   = prefs.getUInt ("shot",      1);
  prefs.end();
}

/* ------------------------------- CAMERA ---------------------------------- */

static bool startCamera() {
  camera_config_t c;
  c.ledc_channel = LEDC_CHANNEL_0;
  c.ledc_timer   = LEDC_TIMER_0;
  c.pin_d0 = Y2_GPIO_NUM;   c.pin_d1 = Y3_GPIO_NUM;
  c.pin_d2 = Y4_GPIO_NUM;   c.pin_d3 = Y5_GPIO_NUM;
  c.pin_d4 = Y6_GPIO_NUM;   c.pin_d5 = Y7_GPIO_NUM;
  c.pin_d6 = Y8_GPIO_NUM;   c.pin_d7 = Y9_GPIO_NUM;
  c.pin_xclk = XCLK_GPIO_NUM;   c.pin_pclk  = PCLK_GPIO_NUM;
  c.pin_vsync = VSYNC_GPIO_NUM; c.pin_href  = HREF_GPIO_NUM;
  c.pin_sccb_sda = SIOD_GPIO_NUM; c.pin_sccb_scl = SIOC_GPIO_NUM;
  c.pin_pwdn = PWDN_GPIO_NUM;   c.pin_reset = RESET_GPIO_NUM;
  c.xclk_freq_hz = 20000000;
  c.pixel_format = PIXFORMAT_JPEG;
  c.grab_mode    = CAMERA_GRAB_LATEST;
  c.fb_location  = CAMERA_FB_IN_PSRAM;

  if (psramFound()) {
    c.frame_size   = FRAMESIZE_UXGA;
    c.jpeg_quality = 10;
    c.fb_count     = 2;
  } else {
    c.frame_size   = FRAMESIZE_SVGA;
    c.jpeg_quality = 12;
    c.fb_count     = 1;
    c.fb_location  = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&c);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return false;
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s != NULL) {
    // FLIP & MIRROR BY DEFAULT
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);

    // NORMAL COLORS DEFAULT
    s->set_brightness(s, 0);
    s->set_contrast(s, 0);
    s->set_saturation(s, 0);

    // NO FILTER DEFAULT
    s->set_special_effect(s, 0); // 0 = OFF

    // NORMAL AUTO ISO/GAIN
    s->set_gain_ctrl(s, 1);

    s->set_framesize(s, (framesize_t)cfg.liveSize);
  }
  return true;
}

/* --------------------------------- SD ------------------------------------ */

static bool startSD() {
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD mount failed");
    return false;
  }
  if (SD_MMC.cardType() == CARD_NONE) {
    Serial.println("No SD card");
    return false;
  }
  if (!SD_MMC.exists("/DCIM")) {
    SD_MMC.mkdir("/DCIM");
  }
  return true;
}

static String nextPath() {
  char p[32];
  for (int i = 0; i < 10000; i++) {
    snprintf(p, sizeof(p), "/DCIM/IMG_%04lu.JPG", (unsigned long)shotCounter);
    if (!SD_MMC.exists(p)) return String(p);
    shotCounter++;
  }
  return String("/DCIM/IMG_OVER.JPG");
}

static String takePhoto(String &err) {
  if (!sdReady) { err = "No SD card"; return ""; }
  if (busy)     { err = "Busy";       return ""; }
  busy = true;
  streamPaused = true;
  delay(100);

  sensor_t *s = esp_camera_sensor_get();
  bool resized = (cfg.photoSize != cfg.liveSize);
  if (resized && s) { 
    s->set_framesize(s, (framesize_t)cfg.photoSize); 
    delay(300); 
  }

  if (cfg.lampOn) {
    LAMP_WRITE(cfg.lampLevel);
    delay(150);
  }

  for (int i = 0; i < 2; i++) {
    camera_fb_t *t = esp_camera_fb_get();
    if (t) esp_camera_fb_return(t);
  }

  camera_fb_t *fb = esp_camera_fb_get();
  String path = "";
  if (!fb) {
    err = "Capture failed";
  } else {
    path = nextPath();
    File f = SD_MMC.open(path.c_str(), FILE_WRITE);
    if (!f) {
      err = "Write failed";
      path = "";
    } else {
      f.write(fb->buf, fb->len);
      f.close();
      shotCounter++;
      saveSettings();
    }
    esp_camera_fb_return(fb);
  }

  if (resized && s) { 
    s->set_framesize(s, (framesize_t)cfg.liveSize); 
    delay(200); 
  }

  streamPaused = false;
  busy = false;
  return path;
}

/* ------------------------------ HANDLERS --------------------------------- */

static esp_err_t indexHandler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, PAGE_INDEX, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t statusHandler(httpd_req_t *req) {
  sensor_t *s = esp_camera_sensor_get();
  if (!s) return ESP_FAIL;

  String j = "{";
  j += "\"live_size\":"   + String(cfg.liveSize);
  j += ",\"photo_size\":" + String(cfg.photoSize);
  j += ",\"quality\":"    + String(s->status.quality);
  j += ",\"brightness\":" + String(s->status.brightness);
  j += ",\"contrast\":"   + String(s->status.contrast);
  j += ",\"saturation\":" + String(s->status.saturation);
  j += ",\"sharpness\":"  + String(s->status.sharpness);
  j += ",\"special_effect\":" + String(s->status.special_effect);
  j += ",\"awb\":"        + String(s->status.awb);
  j += ",\"awb_gain\":"   + String(s->status.awb_gain);
  j += ",\"wb_mode\":"    + String(s->status.wb_mode);
  j += ",\"aec\":"        + String(s->status.aec);
  j += ",\"aec2\":"       + String(s->status.aec2);
  j += ",\"ae_level\":"   + String(s->status.ae_level);
  j += ",\"aec_value\":"  + String(s->status.aec_value);
  j += ",\"agc\":"        + String(s->status.agc);
  j += ",\"agc_gain\":"   + String(s->status.agc_gain);
  j += ",\"gainceiling\":" + String(s->status.gainceiling);
  j += ",\"bpc\":"        + String(s->status.bpc);
  j += ",\"wpc\":"        + String(s->status.wpc);
  j += ",\"raw_gma\":"    + String(s->status.raw_gma);
  j += ",\"lenc\":"       + String(s->status.lenc);
  j += ",\"hmirror\":"    + String(s->status.hmirror);
  j += ",\"vflip\":"      + String(s->status.vflip);
  j += ",\"dcw\":"        + String(s->status.dcw);
  j += ",\"lamp_on\":"    + String(cfg.lampOn ? 1 : 0);
  j += ",\"lamp_level\":" + String(cfg.lampLevel);
  j += ",\"sd\":"         + String(sdReady ? 1 : 0);
  if (sdReady) {
    uint64_t total = SD_MMC.totalBytes() / (1024ULL * 1024ULL);
    uint64_t used  = SD_MMC.usedBytes()  / (1024ULL * 1024ULL);
    j += ",\"sd_total\":" + String((uint32_t)total);
    j += ",\"sd_free\":"  + String((uint32_t)(total - used));
  }
  j += ",\"shot\":" + String(shotCounter);
  j += "}";
  sendJson(req, j);
  return ESP_OK;
}

static esp_err_t controlHandler(httpd_req_t *req) {
  char var[32], val[32];
  if (!getParam(req, "var", var, sizeof(var)) ||
      !getParam(req, "val", val, sizeof(val))) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing parameters");
    return ESP_FAIL;
  }
  int v = atoi(val);
  sensor_t *s = esp_camera_sensor_get();
  int res = 0;

  if (!strcmp(var, "live_size")) { 
    cfg.liveSize = v; 
    streamPaused = true; 
    delay(60);
    if (s) res = s->set_framesize(s, (framesize_t)v);
    delay(200); 
    streamPaused = false; 
    saveSettings(); 
  }
  else if (!strcmp(var, "photo_size"))  { cfg.photoSize = v; saveSettings(); }
  else if (!s)                          { res = -1; }
  else if (!strcmp(var, "quality"))     res = s->set_quality(s, v);
  else if (!strcmp(var, "brightness"))  res = s->set_brightness(s, v);
  else if (!strcmp(var, "contrast"))    res = s->set_contrast(s, v);
  else if (!strcmp(var, "saturation"))  res = s->set_saturation(s, v);
  else if (!strcmp(var, "sharpness"))   res = s->set_sharpness(s, v);
  else if (!strcmp(var, "special_effect")) res = s->set_special_effect(s, v);
  else if (!strcmp(var, "awb"))         res = s->set_whitebal(s, v);
  else if (!strcmp(var, "awb_gain"))    res = s->set_awb_gain(s, v);
  else if (!strcmp(var, "wb_mode"))     res = s->set_wb_mode(s, v);
  else if (!strcmp(var, "aec"))         res = s->set_exposure_ctrl(s, v);
  else if (!strcmp(var, "aec2"))        res = s->set_aec2(s, v);
  else if (!strcmp(var, "ae_level"))    res = s->set_ae_level(s, v);
  else if (!strcmp(var, "aec_value"))   res = s->set_aec_value(s, v);
  else if (!strcmp(var, "agc"))         res = s->set_gain_ctrl(s, v);
  else if (!strcmp(var, "agc_gain"))    res = s->set_agc_gain(s, v);
  else if (!strcmp(var, "gainceiling")) res = s->set_gainceiling(s, (gainceiling_t)v);
  else if (!strcmp(var, "bpc"))         res = s->set_bpc(s, v);
  else if (!strcmp(var, "wpc"))         res = s->set_wpc(s, v);
  else if (!strcmp(var, "raw_gma"))     res = s->set_raw_gma(s, v);
  else if (!strcmp(var, "lenc"))        res = s->set_lenc(s, v);
  else if (!strcmp(var, "hmirror"))     res = s->set_hmirror(s, v);
  else if (!strcmp(var, "vflip"))       res = s->set_vflip(s, v);
  else if (!strcmp(var, "dcw"))         res = s->set_dcw(s, v);
  else if (!strcmp(var, "lamp_level"))  { 
    cfg.lampLevel = constrain(v, 0, 255);
    if (cfg.lampOn) LAMP_WRITE(cfg.lampLevel);
    saveSettings(); 
  }
  else if (!strcmp(var, "lamp_on"))     { 
    cfg.lampOn = v;
    LAMP_WRITE(cfg.lampOn ? cfg.lampLevel : 0);
    saveSettings(); 
  }
  else res = -1;

  if (res < 0) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Setting not supported");
    return ESP_FAIL;
  }
  sendJson(req, "{\"ok\":1}");
  return ESP_OK;
}

static esp_err_t shootHandler(httpd_req_t *req) {
  String err = "";
  String path = takePhoto(err);
  if (path == "") {
    sendJson(req, "{\"ok\":0,\"error\":\"" + err + "\"}");
  } else {
    sendJson(req, "{\"ok\":1,\"file\":\"" + path + "\"}");
  }
  return ESP_OK;
}

static esp_err_t listHandler(httpd_req_t *req) {
  if (!sdReady) { sendJson(req, "{\"ok\":0,\"files\":[]}"); return ESP_OK; }
  File dir = SD_MMC.open("/DCIM");
  String j = "{\"ok\":1,\"files\":[";
  bool first = true;
  if (dir && dir.isDirectory()) {
    File f = dir.openNextFile();
    while (f) {
      if (!f.isDirectory()) {
        String nm = String(f.name());
        int sl = nm.lastIndexOf('/');
        if (sl >= 0) nm = nm.substring(sl + 1);
        if (!first) j += ",";
        j += "{\"n\":\"" + nm + "\",\"s\":" + String((uint32_t)f.size()) + "}";
        first = false;
      }
      f = dir.openNextFile();
    }
  }
  j += "]}";
  sendJson(req, j);
  return ESP_OK;
}

static esp_err_t downloadHandler(httpd_req_t *req) {
  char name[64], dlFlag[4];
  if (!getParam(req, "f", name, sizeof(name))) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing file param");
    return ESP_FAIL;
  }
  String path = "/DCIM/" + String(name);
  File f = SD_MMC.open(path.c_str());
  if (!f || f.isDirectory()) {
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
    return ESP_FAIL;
  }
  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  bool wantsDownload = getParam(req, "dl", dlFlag, sizeof(dlFlag)) && dlFlag[0] == '1';
  char disp[96];
  snprintf(disp, sizeof(disp), "%s; filename=\"%s\"", wantsDownload ? "attachment" : "inline", name);
  httpd_resp_set_hdr(req, "Content-Disposition", disp);

  uint8_t buf[2048];
  size_t n;
  while ((n = f.read(buf, sizeof(buf))) > 0) {
    if (httpd_resp_send_chunk(req, (const char *)buf, n) != ESP_OK) {
      f.close();
      return ESP_FAIL;
    }
  }
  f.close();
  httpd_resp_send_chunk(req, NULL, 0);
  return ESP_OK;
}

static esp_err_t deleteHandler(httpd_req_t *req) {
  char name[64];
  if (!getParam(req, "f", name, sizeof(name))) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing file param");
    return ESP_FAIL;
  }
  String path = "/DCIM/" + String(name);
  bool ok = SD_MMC.remove(path.c_str());
  sendJson(req, ok ? "{\"ok\":1}" : "{\"ok\":0,\"error\":\"Delete failed\"}");
  return ESP_OK;
}

/* ------------------------------- STREAM ---------------------------------- */

#define PART_BOUNDARY "digicamframe"
static const char *STREAM_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static esp_err_t streamHandler(httpd_req_t *req) {
  esp_err_t res = httpd_resp_set_type(req, STREAM_TYPE);
  if (res != ESP_OK) return res;
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  char part[64];
  while (true) {
    if (streamPaused) { 
      delay(50); 
      continue; 
    }

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) { 
      delay(20);
      continue; 
    }

    size_t len = fb->len;
    res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, strlen(STREAM_BOUNDARY));
    if (res == ESP_OK) {
      size_t hlen = snprintf(part, sizeof(part), STREAM_PART, len);
      res = httpd_resp_send_chunk(req, part, hlen);
    }
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)fb->buf, len);
    esp_camera_fb_return(fb);
    if (res != ESP_OK) break;
    
    taskYIELD();
  }
  return res;
}

/* ------------------------------ SERVERS ---------------------------------- */

static void startServers() {
  httpd_config_t ui = HTTPD_DEFAULT_CONFIG();
  ui.server_port = 80;
  ui.ctrl_port   = 32768;
  ui.max_uri_handlers = 12;
  ui.stack_size  = 8192;

  httpd_uri_t routes[] = {
    { "/",         HTTP_GET, indexHandler,    NULL },
    { "/status",   HTTP_GET, statusHandler,   NULL },
    { "/control",  HTTP_GET, controlHandler,  NULL },
    { "/shoot",    HTTP_GET, shootHandler,    NULL },
    { "/list",     HTTP_GET, listHandler,     NULL },
    { "/dl",       HTTP_GET, downloadHandler, NULL },
    { "/rm",       HTTP_GET, deleteHandler,   NULL },
  };

  if (httpd_start(&uiServer, &ui) == ESP_OK) {
    for (auto &r : routes) httpd_register_uri_handler(uiServer, &r);
  }

  httpd_config_t st = HTTPD_DEFAULT_CONFIG();
  st.server_port = 81;
  st.ctrl_port   = 32769;
  st.stack_size  = 8192;
  httpd_uri_t streamUri = { "/stream", HTTP_GET, streamHandler, NULL };
  if (httpd_start(&streamServer, &st) == ESP_OK) {
    httpd_register_uri_handler(streamServer, &streamUri);
  }
}

/* -------------------------------- SETUP ---------------------------------- */

void setup() {
#if DISABLE_BROWNOUT
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
#endif
  Serial.begin(115200);
  delay(500);

  loadSettings();

  pinMode(SHUTTER_PIN, INPUT_PULLUP);

  if (!startCamera()) {
    Serial.println("Halting: camera failed");
    while (true) delay(1000);
  }

  sdReady = startSD();
  Serial.println(sdReady ? "SD ready" : "SD not available");

  LAMP_SETUP();
  cfg.lampOn = false;
  LAMP_WRITE(0);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
  WiFi.softAP(AP_SSID, strlen(AP_PASS) ? AP_PASS : NULL, AP_CHANNEL, 0, AP_MAX_CLIENTS);
  WiFi.setSleep(false);
  
  Serial.println("=====================================");
  Serial.print  ("  WiFi   : "); Serial.println(AP_SSID);
  Serial.print  ("  Pass   : "); Serial.println(AP_PASS);
  Serial.print  ("  Open   : http://"); Serial.println(WiFi.softAPIP());
  Serial.println("=====================================");

  startServers();
  Serial.println("DIGICAM-8000 ready");
}

/* --------------------------------- LOOP ---------------------------------- */

void loop() {
  static bool     lastState = HIGH;
  static uint32_t lastEdge  = 0;

  bool now = digitalRead(SHUTTER_PIN);
  if (now != lastState && millis() - lastEdge > 50) {
    lastEdge  = millis();
    lastState = now;
    if (now == LOW) {
      String err = "";
      String p = takePhoto(err);
      Serial.println(p != "" ? "Saved " + p : "Shutter error: " + err);
    }
  }
  delay(10);
}
