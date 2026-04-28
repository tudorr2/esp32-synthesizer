#pragma once
#include <Arduino.h>
#include <functional>

class Motif
{
public:
    bool enabled;
    int root_note;
    float gate_percent;
    float bpm;
    int steps_per_beat;
    float per_beat_time;
    float note_duration;

    int trans_steps;
    int trans_distance;
    int trans_pos;

    std::function<void(int)> note_on_handler;
    std::function<void(int)> note_off_handler;

    static const int NUM_motS = 15;
    static const int mot_LEN = 4;

    const char *mot_names[NUM_motS] = {
        "major", "minor7th", "diminished", "suspended4th",
        "octaves", "octaves2", "octaves3", "new1", "new2", "power", "jazzy", "fifth-jump", "glitchy", "leap", "root"};
    const int mot_table[NUM_motS][mot_LEN] = {
        {0, 4, 7, 12},    // major
        {0, 3, 7, 10},    // minor7th
        {0, 3, 6, 3},     // diminished
        {0, 5, 7, 12},    // suspended4th
        {0, 12, 0, -12},  // octaves
        {0, 12, 24, -12}, // octaves2
        {0, -12, -12, 0}, // octaves3
        {0, 1, 1, 1},     // new1
        {0, 12, 12, 12},  // new2
        {0, 7, 12, 7},    // power
        {0, 4, 7, 11},    // jazzy
        {0, 12, 7, 19},   // fifth jump
        {0, 6, 13, 5},    // glitchy
        {0, 30, 20, 10},  // leap
        {0, 0, 0, 0},     // root
    };

    Motif()
        : enabled(false),
          root_note(48), // C3
          // bas 24 to 48 //lead 60 to 96
          gate_percent(0.90f), // longer notes, flowing instead of 0.3
          bpm(100.0f),         // max 180?
          steps_per_beat(2),
          mot_id(0),
          mot_pos(0),
          note_played(-1),
          last_beat_ms(0),
          trans_steps(0),
          trans_distance(12),
          trans_pos(0)
    {
        note_on_handler = [](int n)
        { Serial.printf("note on  %d\n", n); };
        note_off_handler = [](int n)
        { Serial.printf("note off %d\n", n); };
        _recalc_timing();
    }

    void set_bpm(float new_bpm, int new_steps_per_beat = 0)
    {
        bpm = new_bpm;
        if (new_steps_per_beat > 0)
            steps_per_beat = new_steps_per_beat;
        _recalc_timing();
    }

    void set_transpose(int distance = 12, int steps = 0)
    {
        trans_distance = distance;
        trans_steps = steps;
    }

    void on()
    {
        enabled = true;

        last_beat_ms = millis() - (unsigned long)(per_beat_time * 1000.0f);
    }

    void off() { enabled = false; }

    void set_mot(const char *name)
    {
        for (int i = 0; i < NUM_motS; i++)
        {
            if (strcmp(mot_names[i], name) == 0)
            {
                mot_id = i;
                return;
            }
        }
    }
    void set_mot(int id)
    {
        mot_id = id % NUM_motS;
    }

    const char *mot_name() { return mot_names[mot_id]; }

    void next_mot() { mot_id = (mot_id + 1) % NUM_motS; }

    void update()
    {
        if (!enabled)
            return;

        unsigned long now_ms = millis();
        float elapsed = (now_ms - last_beat_ms) / 1000.0f; // move on to the next note based on the bpm

        if (elapsed >= per_beat_time)
        {
            last_beat_ms = now_ms;

            int trans_amount = trans_distance * trans_pos;

            if (mot_pos == 0)
            {
                trans_pos = (trans_pos + 1) % (trans_steps + 1);
            }

            note_played = root_note + mot_table[mot_id][mot_pos] + trans_amount; // trans_amount changes the octave higher or lower bc trans_distance is set to 12
            note_on_handler(note_played);

            mot_pos = (mot_pos + 1) % mot_LEN;
        }

        float since_beat = (millis() - last_beat_ms) / 1000.0f;
        if (since_beat > note_duration && note_played >= 0)
        {
            note_off_handler(note_played);
            note_played = -1;
        }
    }

private:
    int mot_id;
    int mot_pos;
    int note_played;
    unsigned long last_beat_ms;

    void _recalc_timing()
    {
        per_beat_time = 60.0f / bpm / (float)steps_per_beat;
        note_duration = gate_percent * per_beat_time;
    }
};