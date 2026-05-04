
#include <Arduino.h>
#include <driver/dac.h>
#include <esp_timer.h>
#include "lib.h"

#define PIN_KNOB_A 36
#define PIN_KNOB_B 39
#define PIN_BUTTON_A 14 // transpose steps
#define PIN_BUTTON_B 27 // mot pattern
#define PIN_DAC 25
#define PIN_LED 22
#define POWER_SW 15

#define SAMPLE_RATE 44100 // khz
#define NUM_VOICES 3
#define WAVE_LEN 512

int16_t wave_saw[WAVE_LEN];

struct Voice
{
  float phase;
  float freq;
  float lpf_out;
  float lpf_cutoff;
};
Voice voices[NUM_VOICES];

volatile float amp_level = 0.0f; // changes inside the isr
volatile float amp_target = 0.0f;
const float ATTACK_RATE = 1.0f / (0.01f * SAMPLE_RATE); // how fast it goes to 100; fade-in rate
const float RELEASE_RATE = 1.0f / (0.3f * SAMPLE_RATE);

float knobAval = 0, knobBval = 0;
const float KNOB_FILTER = 0.8f;

bool lastBtnA = HIGH, lastBtnB = HIGH;
unsigned long lastBtnAms = 0, lastBtnBms = 0;
const unsigned long DEBOUNCE_MS = 50; // waiting period after the btn press

Motif motif;

float midi_to_hz(int note)
{
  return 440.0f * powf(2.0f, (note - 69) / 12.0f);
}

float map_range(float s, float a1, float a2, float b1, float b2)
{
  return b1 + ((s - a1) * (b2 - b1) / (a2 - a1));
}

void note_on(int n)
{
  Serial.printf("  note on  %d\n", n);
  digitalWrite(PIN_LED, HIGH);

  float fo = midi_to_hz(n);
  for (int i = 0; i < NUM_VOICES; i++)
  {
    voices[i].freq = fo * (1.0f + i * 0.007f);
    float cutoff = (fo * 8.0f) / (SAMPLE_RATE / 2.0f);
    voices[i].lpf_cutoff = constrain(cutoff, 0.05f, 0.9f);
  }
  amp_target = 1.0f;
}

void note_off(int n)
{
  Serial.printf("  note off %d\n", n);
  digitalWrite(PIN_LED, LOW);
  amp_target = 0.0f;
}

void audio_isr(void *arg) // interrupt service routine
{

  // volume fade-in
  if (amp_level < amp_target)
  {
    amp_level += ATTACK_RATE;
    if (amp_level > amp_target)
      amp_level = amp_target;
  } // fade-out
  else if (amp_level > amp_target)
  {
    amp_level -= RELEASE_RATE;
    if (amp_level < 0.0f)
      amp_level = 0.0f;
  }

  if (amp_level <= 0.001f)
  {
    dacWrite(PIN_DAC, 128);
    return;
  }

  float mixed = 0.0f;
  for (int v = 0; v < NUM_VOICES; v++)
  {
    voices[v].phase += voices[v].freq / (float)SAMPLE_RATE; // phase accumulator
    if (voices[v].phase >= 1.0f)
      voices[v].phase -= 1.0f;

    int idx = (int)(voices[v].phase * (WAVE_LEN - 1));
    float sample = wave_saw[idx] / 32768.0f;

    float c = voices[v].lpf_cutoff;
    voices[v].lpf_out += c * (sample - voices[v].lpf_out); // low pass filter
    mixed += voices[v].lpf_out;
  }

  mixed = (mixed / (float)NUM_VOICES) * amp_level;

  int dac_val = (int)(mixed * 90.0f) + 128;

  dacWrite(PIN_DAC, (uint8_t)constrain(dac_val, 0, 255));
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  for (int i = 0; i < WAVE_LEN; i++)
  {
    wave_saw[i] = (int16_t)(30000 - (60000 * i / WAVE_LEN));
  }

  pinMode(PIN_BUTTON_A, INPUT_PULLUP);
  pinMode(PIN_BUTTON_B, INPUT_PULLUP);
  pinMode(POWER_SW, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);

  dac_output_enable(DAC_CHANNEL_1);
  dacWrite(PIN_DAC, 128);

  esp_timer_handle_t audio_timer;
  const esp_timer_create_args_t timer_args = {
      .callback = &audio_isr,
      .arg = NULL,
      .dispatch_method = ESP_TIMER_TASK, // run the audio in a high priority background
      .name = "audio_isr"};
  esp_timer_create(&timer_args, &audio_timer);
  esp_timer_start_periodic(audio_timer, 1000000 / SAMPLE_RATE); // every 22.6ms

  motif.note_on_handler = note_on;
  motif.note_off_handler = note_off;
  motif.on();
  motif.root_note = 37;
  motif.set_mot("suspended4th");
  motif.set_bpm(110, 4);

  Serial.println("Ready");
}

bool lastPowerState = false; 

void loop()
{
  bool powerOn = digitalRead(POWER_SW) == LOW;

  if (!powerOn)
  {
    if (lastPowerState) 
    {
      motif.off();
      amp_target = 0.0f;
      dacWrite(PIN_DAC, 128);
      digitalWrite(PIN_LED, LOW);
      Serial.println("Standby");
    }
    lastPowerState = false;
    return;
  }

  if (!lastPowerState) 
  {
    motif.on();
    Serial.println("Running");
  }
  lastPowerState = true;

  bool btnA = digitalRead(PIN_BUTTON_A);
  if (btnA == LOW && lastBtnA == HIGH && (millis() - lastBtnAms > DEBOUNCE_MS))
  {
    motif.next_mot();
    Serial.print("Pattern: ");
    Serial.println(motif.mot_name());
    lastBtnAms = millis();
  }
  lastBtnA = btnA;

  bool btnB = digitalRead(PIN_BUTTON_B);
  if (btnB == LOW && lastBtnB == HIGH && (millis() - lastBtnBms > DEBOUNCE_MS))
  {
    int steps = (motif.trans_steps + 1) % 3;
    motif.set_transpose(12, steps);
    Serial.printf("Transpose Steps: %d\n", steps);
    lastBtnBms = millis();
  }
  lastBtnB = btnB;

  knobAval = (knobAval * KNOB_FILTER) + (1.0f - KNOB_FILTER) * analogRead(PIN_KNOB_A);
  knobBval = (knobBval * KNOB_FILTER) + (1.0f - KNOB_FILTER) * analogRead(PIN_KNOB_B);

  motif.root_note = (int)map_range(knobAval, 0, 4095, 24, 72);
  motif.set_bpm(map_range(knobBval, 0, 4095, 40, 180));

  motif.update();
}