#pragma once

// Front-panel controls for the Era TV build: a detented rotary encoder tunes
// channels, two volume buttons share one ADC pin through a resistor ladder, and
// a small on-screen display (OSD) is drawn by the playback task between video
// frames so it never competes with the decoder for the display bus.
//
// Profiles that do not define the encoder and ladder pins get no-op stubs, and
// the board button keeps its original behavior.

#if defined(ENCODER_PIN_A) && defined(ENCODER_PIN_B) && defined(VOLUME_LADDER_PIN)
#define FRONT_PANEL_ENABLED 1
#else
#define FRONT_PANEL_ENABLED 0
#endif

#if FRONT_PANEL_ENABLED

#ifndef ENCODER_STEPS_PER_DETENT
#define ENCODER_STEPS_PER_DETENT 4
#endif
#ifndef ENCODER_REVERSE
#define ENCODER_REVERSE 0
#endif
#ifndef ENCODER_SETTLE_MS
#define ENCODER_SETTLE_MS 300 // Tune once the knob stops instead of on every click
#endif
#ifndef VOLUME_STEP
#define VOLUME_STEP 10
#endif
#ifndef VOLUME_MAX
#define VOLUME_MAX 200 // Matches the web slider; above 100 is digital boost
#endif
#ifndef VOLUME_UP_MAX_RAW
#define VOLUME_UP_MAX_RAW 1000 // Vol+ shorts the ladder to GND and reads near 0
#endif
#ifndef VOLUME_DOWN_MAX_RAW
#define VOLUME_DOWN_MAX_RAW 2900 // Vol- forms a 10k/10k divider (~1900); idle reads 4095
#endif
#ifndef CHANNEL_STATIC_MS
#define CHANNEL_STATIC_MS 300
#endif

#define OSD_SHOW_MS 2000
#define SETTINGS_SAVE_DELAY_MS 2000
#define VOLUME_REPEAT_DELAY_MS 450
#define VOLUME_REPEAT_MS 150
#define VOLUME_BAR_SEGMENTS 20

#define OSD_NONE 0
#define OSD_CHANNEL 1
#define OSD_VOLUME 2

struct OsdRect {
  int16_t x, y, w, h;
};

// Panels are aligned to the decoder's 80x16 pixel draw blocks so the video
// blocks underneath an active panel can be skipped cleanly.
static const OsdRect kOsdChannelRect = {SCREEN_WIDTH - 160, 0, 160, 48};
static const OsdRect kOsdVolumeRect = {0, SCREEN_HEIGHT - 48, SCREEN_WIDTH, 48};

// Quadrature transitions indexed by (previous state << 2) | current state,
// where a state is (A << 1) | B. Bounces and skipped states contribute 0.
static const int8_t kQuadratureSteps[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

static Preferences frontPanelPrefs;
static portMUX_TYPE frontPanelMux = portMUX_INITIALIZER_UNLOCKED;
static volatile uint8_t osd_kind = OSD_NONE;
static volatile bool osd_dirty = false;
static volatile uint32_t osd_hide_at_ms = 0;
static volatile int osd_channel = 0;
static volatile int pending_channel = -1;
static volatile uint32_t pending_channel_at_ms = 0;
static volatile bool channel_static_requested = false;
static volatile bool settings_save_pending = false;
static volatile uint32_t settings_save_at_ms = 0;
static uint8_t osd_drawn_kind = OSD_NONE; // Playback task only
static uint32_t static_noise_state = 0x9E3779B9;

static const OsdRect &osdRectFor(uint8_t kind) {
  return kind == OSD_VOLUME ? kOsdVolumeRect : kOsdChannelRect;
}

// Called from drawMCU for every decoded block, in screen coordinates.
static inline bool frontPanelCoversBlock(int x, int y, int w, int h) {
  uint8_t kind = osd_kind;
  if (kind == OSD_NONE) return false;
  const OsdRect &r = osdRectFor(kind);
  return x < r.x + r.w && x + w > r.x && y < r.y + r.h && y + h > r.y;
}

static void frontPanelShowOsd(uint8_t kind) {
  portENTER_CRITICAL(&frontPanelMux);
  osd_kind = kind;
  osd_hide_at_ms = millis() + OSD_SHOW_MS;
  osd_dirty = true;
  portEXIT_CRITICAL(&frontPanelMux);
}

static void drawChannelOsd(int channel) {
  const OsdRect &r = kOsdChannelRect;
  char label[12];
  if (channel == 0) {
    snprintf(label, sizeof(label), "RANDOM");
  } else {
    snprintf(label, sizeof(label), "CH %02d", channel);
  }
  gfx->fillRect(r.x, r.y, r.w, r.h, BLACK);
  gfx->setTextSize(4); // 24x32 pixel characters
  gfx->setTextColor(GREEN);
  gfx->setCursor(r.x + r.w - (int)strlen(label) * 24 - 12, r.y + 8);
  gfx->print(label);
}

static void drawVolumeOsd() {
  const OsdRect &r = kOsdVolumeRect;
  gfx->fillRect(r.x, r.y, r.w, r.h, BLACK);
  gfx->setTextSize(2);
  gfx->setCursor(r.x + 12, r.y + 16);
  if (is_muted) {
    gfx->setTextColor(RED);
    gfx->print("MUTE");
    return;
  }
  gfx->setTextColor(GREEN);
  gfx->print("VOL");
  const int segmentWidth = 12;
  const int barX = r.x + r.w - VOLUME_BAR_SEGMENTS * segmentWidth - 20;
  const int lit = (audio_volume * VOLUME_BAR_SEGMENTS + VOLUME_MAX / 2) / VOLUME_MAX;
  const int boostStart = 100 * VOLUME_BAR_SEGMENTS / VOLUME_MAX;
  for (int i = 0; i < VOLUME_BAR_SEGMENTS; i++) {
    uint16_t color = DARKGREY;
    if (i < lit) color = i < boostStart ? GREEN : YELLOW;
    gfx->fillRect(barX + i * segmentWidth, r.y + 14, segmentWidth - 3, 20, color);
  }
}

// Draws, refreshes, or retires the OSD. Runs on the playback task: after each
// video frame, during channel static, and from loop() while nothing plays.
static void frontPanelServiceOsd() {
  portENTER_CRITICAL(&frontPanelMux);
  uint8_t kind = osd_kind;
  if (kind != OSD_NONE && (int32_t)(millis() - osd_hide_at_ms) >= 0) {
    kind = OSD_NONE;
    osd_kind = OSD_NONE;
  }
  bool dirty = osd_dirty;
  osd_dirty = false;
  int channel = osd_channel;
  portEXIT_CRITICAL(&frontPanelMux);

  if (osd_drawn_kind != OSD_NONE && osd_drawn_kind != kind && !playback_active) {
    // No video is repainting underneath, so erase the retired panel.
    const OsdRect &r = osdRectFor(osd_drawn_kind);
    gfx->fillRect(r.x, r.y, r.w, r.h, BLACK);
  }
  if (kind != OSD_NONE && (dirty || kind != osd_drawn_kind)) {
    if (kind == OSD_CHANNEL) {
      drawChannelOsd(channel);
    } else {
      drawVolumeOsd();
    }
  }
  osd_drawn_kind = kind;
}

static inline uint32_t staticNoise() {
  static_noise_state ^= static_noise_state << 13;
  static_noise_state ^= static_noise_state >> 17;
  static_noise_state ^= static_noise_state << 5;
  return static_noise_state;
}

static void playStaticHiss() {
  if (is_muted || audio_volume == 0) return;
  int16_t samples[256];
  const int amplitude = 2500 * audio_volume / 100;
  for (int i = 0; i < 256; i++) {
    int16_t sample = (int16_t)((int32_t)(staticNoise() % (2 * amplitude + 1)) - amplitude);
#ifdef USE_INTERNAL_DAC
    samples[i] = (uint16_t)sample + 0x8000;
#else
    samples[i] = sample;
#endif
  }
  size_t written = 0;
  i2s_write(_i2s_num, samples, sizeof(samples), &written, 0); // Never block the picture
}

// Fills the screen with analog-style snow for CHANNEL_STATIC_MS after the knob
// tunes a new channel. Called by the playback task before the next video opens.
static void frontPanelPlayChannelStatic() {
  if (!channel_static_requested) return;
  channel_static_requested = false;
#if CHANNEL_STATIC_MS > 0
  const int bandRows = 4;
  static uint16_t band[SCREEN_WIDTH * bandRows];
  const uint32_t started = millis();
  while (millis() - started < CHANNEL_STATIC_MS && !web_stop_requested) {
    for (int y = 0; y < SCREEN_HEIGHT; y += bandRows) {
      for (int i = 0; i < SCREEN_WIDTH * bandRows; i += 2) {
        uint32_t noise = staticNoise();
        uint16_t a = noise & 0x1F;
        uint16_t b = (noise >> 8) & 0x1F;
        band[i] = (a << 11) | (a << 6) | a;
        band[i + 1] = (b << 11) | (b << 6) | b;
      }
      // Paint around a visible OSD panel instead of over it.
      int x0 = 0, x1 = SCREEN_WIDTH;
      uint8_t kind = osd_kind;
      if (kind != OSD_NONE) {
        const OsdRect &r = osdRectFor(kind);
        if (y < r.y + r.h && y + bandRows > r.y) {
          if (r.x > 0) x1 = r.x;
          else x0 = r.x + r.w;
        }
      }
      if (x1 > x0) {
        gfx->draw16bitRGBBitmap(x0 + SCREEN_OFFSET_X, y + SCREEN_OFFSET_Y, band, x1 - x0, bandRows);
      }
      if ((y % 32) == 0) playStaticHiss();
    }
    frontPanelServiceOsd();
  }
#endif
}

// Moves the tuner by one channel. The OSD updates immediately and the new
// channel starts once the knob has been still for ENCODER_SETTLE_MS.
static void frontPanelStepChannel(int direction) {
  if (video_count == 0 && !has_random) return;
  const int minimum = has_random ? 0 : 1;
  portENTER_CRITICAL(&frontPanelMux);
  int channel = (pending_channel >= 0 ? pending_channel : video_idx) + direction;
  if (channel < minimum) channel = video_count;
  else if (channel > video_count) channel = minimum;
  pending_channel = channel;
  pending_channel_at_ms = millis() + ENCODER_SETTLE_MS;
  osd_channel = channel;
  portEXIT_CRITICAL(&frontPanelMux);
  frontPanelShowOsd(OSD_CHANNEL);
}

static void commitPendingChannel() {
  portENTER_CRITICAL(&frontPanelMux);
  const int channel = pending_channel;
  const bool due = channel >= 0 && (int32_t)(millis() - pending_channel_at_ms) >= 0;
  if (due) pending_channel = -1;
  portEXIT_CRITICAL(&frontPanelMux);
  if (!due) return;

  if (screen_sleeping) wakeMiniTV();
  if (channel == video_idx && !playback_paused) return; // Turned away and back
  Serial.printf("Knob tuned channel %d\n", channel);
  logEvent("Knob tuned channel %d", channel);
  frontPanelPrefs.begin(APP_NAME, false);
  frontPanelPrefs.putInt(K_VIDEO_INDEX, channel);
  frontPanelPrefs.end();
  video_idx = channel;
  channel_static_requested = true;
  playback_paused = false;
  web_stop_requested = true;
}

static void frontPanelStepVolume(int direction) {
  // Like a real TV, either volume button cancels mute.
  is_muted = false;
  int volume = audio_volume;
  if (direction > 0) {
    volume = (volume / VOLUME_STEP + 1) * VOLUME_STEP;
  } else {
    volume = ((volume + VOLUME_STEP - 1) / VOLUME_STEP - 1) * VOLUME_STEP;
  }
  audio_volume = constrain(volume, 0, VOLUME_MAX);
  settings_save_at_ms = millis() + SETTINGS_SAVE_DELAY_MS;
  settings_save_pending = true;
  frontPanelShowOsd(OSD_VOLUME);
}

// Volume is saved once the buttons are idle to spare flash writes.
static void saveSettingsIfDue() {
  if (!settings_save_pending || (int32_t)(millis() - settings_save_at_ms) < 0) return;
  settings_save_pending = false;
  frontPanelPrefs.begin(APP_NAME, false);
  frontPanelPrefs.putInt("volume", audio_volume);
  frontPanelPrefs.putBool(K_MUTE, is_muted);
  frontPanelPrefs.end();
}

static uint8_t readEncoderPins() {
  return (digitalRead(ENCODER_PIN_A) << 1) | digitalRead(ENCODER_PIN_B);
}

static bool encoderAtDetent(uint8_t state) {
#if ENCODER_STEPS_PER_DETENT >= 4
  return state == 0b11;
#else
  return state == 0b11 || state == 0b00;
#endif
}

static int readVolumeButtons() {
  int raw = analogRead(VOLUME_LADDER_PIN);
  if (raw < VOLUME_UP_MAX_RAW) return 1;
  if (raw < VOLUME_DOWN_MAX_RAW) return -1;
  return 0;
}

static void frontPanelTask(void *) {
  uint8_t encoderState = readEncoderPins();
  int encoderSteps = 0;
  int volumeCandidate = 0;
  int volumeStable = 0;
  int volumeSamples = 0;
  uint32_t nextVolumeSampleMs = 0;
  uint32_t nextVolumeRepeatMs = 0;

  while (true) {
    if (setupModeActive) {
      encoderState = readEncoderPins();
      encoderSteps = 0;
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    // Poll the encoder every millisecond and count a channel step only when
    // the knob comes to rest in a detent, which rejects contact bounce.
    uint8_t state = readEncoderPins();
    if (state != encoderState) {
      encoderSteps += kQuadratureSteps[(encoderState << 2) | state];
      encoderState = state;
      if (encoderAtDetent(state)) {
        int direction = 0;
        if (encoderSteps >= ENCODER_STEPS_PER_DETENT / 2) direction = 1;
        else if (encoderSteps <= -(ENCODER_STEPS_PER_DETENT / 2)) direction = -1;
        if (direction) frontPanelStepChannel(ENCODER_REVERSE ? -direction : direction);
        encoderSteps = 0;
      }
    }

    // Sample the volume ladder every 10 ms, debounce over three samples, and
    // auto-repeat while a button is held.
    uint32_t now = millis();
    if ((int32_t)(now - nextVolumeSampleMs) >= 0) {
      nextVolumeSampleMs = now + 10;
      int button = readVolumeButtons();
      if (button != volumeCandidate) {
        volumeCandidate = button;
        volumeSamples = 0;
      } else if (volumeSamples < 3) {
        if (++volumeSamples == 3 && button != volumeStable) {
          volumeStable = button;
          if (button) {
            frontPanelStepVolume(button);
            nextVolumeRepeatMs = now + VOLUME_REPEAT_DELAY_MS;
          }
        }
      } else if (volumeStable && (int32_t)(now - nextVolumeRepeatMs) >= 0) {
        frontPanelStepVolume(volumeStable);
        nextVolumeRepeatMs = now + VOLUME_REPEAT_MS;
      }
    }

    commitPendingChannel();
    saveSettingsIfDue();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

static void frontPanelBegin() {
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(VOLUME_LADDER_PIN, INPUT); // GPIO 35 has no pull-up; the ladder supplies 10k
  analogSetPinAttenuation(VOLUME_LADDER_PIN, ADC_11db);
  BaseType_t result = xTaskCreatePinnedToCore(frontPanelTask, "Front panel", 3072, NULL, 2, NULL, 0);
  if (result != pdPASS) {
    Serial.printf("ERROR: Front panel task failed to start: %d\n", (int)result);
  }
}

#else

static inline void frontPanelBegin() {}
static inline bool frontPanelCoversBlock(int, int, int, int) { return false; }
static inline void frontPanelServiceOsd() {}
static inline void frontPanelPlayChannelStatic() {}
static inline void frontPanelStepChannel(int direction) { videoController(direction); }

#endif
