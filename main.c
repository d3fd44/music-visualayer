#include <complex.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "fft/ft.h"

#define MAX_FRAME_SIZE 512
#define UNIT_SIZE      25

typedef struct Frame
{
    float c1, c2;
} Frame;

// clang-format off
Frame         frames[MAX_FRAME_SIZE] = { 0 }; // frames read from the callback
float         c1s   [MAX_FRAME_SIZE] = { 0 }; // channel 1 samples
float         c2s   [MAX_FRAME_SIZE] = { 0 }; // channel 2 samples
float complex freqs1[MAX_FRAME_SIZE] = { 0 }; // FFT of c1s
float complex freqs2[MAX_FRAME_SIZE] = { 0 }; // FFT of c2s
// clang-format on

void get_data(void *fstream, unsigned int count)
{
    memmove(frames, (Frame *)fstream, sizeof(Frame) * count);
}

int main(int argc, char *argv[])
{
    SetTraceLogLevel(LOG_NONE);
    InitWindow(0, 0, "GG");

    SetTargetFPS(60);

    int screenHeight = GetScreenHeight();
    int screenWidth = GetScreenWidth();
    int prevheight = screenHeight - 200;
    int prevwidth = screenWidth - 300;
    int maxmag = 25;

    Camera2D cam = { 0 };
    cam.target = (Vector2){ 0.0f, 0.0f };
    cam.offset = (Vector2){ 0, prevheight };
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;

    InitAudioDevice();
    // Music m = LoadMusicStream("cathedral.mp3");
    // Music m = LoadMusicStream("vampire_killer.mp3");
    // Music m = LoadMusicStream("chambers.mp3");
    // Music m = LoadMusicStream("monster.mp3");
    // Music m = LoadMusicStream("bloody_tears.mp3");
    // Music m = LoadMusicStream("themaster.mp3");
    // Music m = LoadMusicStream("megalovania.mp3");
    // Music m = LoadMusicStream("time_leaper.mp3");
    Music m = LoadMusicStream("rnt.mp3");

    AttachAudioStreamProcessor(m.stream, get_data);

    PlayMusicStream(m);
    SetMusicVolume(m, 0.5f);

    float dx = (float)screenWidth / 64;
    float sec = 0.0f;

    while (!WindowShouldClose())
    {
        int c;
        if (IsKeyPressed(KEY_RIGHT))  // seek forward
        {
            float len = GetMusicTimeLength(m);
            sec = GetMusicTimePlayed(m);
            sec = sec < len - 5.0f ? sec + 5.0f : len;
            SeekMusicStream(m, sec);
        }
        if (IsKeyPressed(KEY_LEFT))  // seek backward
        {
            sec = GetMusicTimePlayed(m);
            sec = sec > 5.0f ? sec - 5.0f : 0.0f;
            SeekMusicStream(m, sec);
        }

        // clang-format off
        UpdateMusicStream(m);
        for (int i = 0; i < 512; i++) { c1s[i] = frames[i].c1; c2s[i] = frames[i].c2; } // copy samples

        if (IsKeyPressed(KEY_SPACE))
        {
            if (IsMusicStreamPlaying(m))
                PauseMusicStream(m);
            else
                ResumeMusicStream(m);
        }

        fft(c1s, freqs1, 512, 1); // perform FFT on copied samples earlier
        fft(c2s, freqs2, 512, 1);

        float out_len[64] = { 0 }; // output vector

        for (int i = 0; i < 256; i += 8)
        {
            float len1 = 0, len2 = 0;
            for (int j = 0; j < 8; j++)
            {
                len1 += cabsf(freqs1[i + j]); // take average over 8 frequencies
                len2 += cabsf(freqs2[i + j]);
            }

            out_len[i / 8] = len1 / 8 * (1 + (float)i / 256); // frequencies for the left channel goes first (normal - left to right)
            out_len[63 - i / 8] = len2 / 8 * (1 + (float)i / 256); // then frequencies for the right channel (reversed right to left)
        }

        BeginDrawing();
            BeginMode2D(cam);

                ClearBackground(BLACK);
                DrawFPS(0, -prevheight);

                for(size_t i = 2; i < 62; i++)
                    DrawRectangle(i * dx, 0, -15, - out_len[i] * UNIT_SIZE, WHITE);

            EndMode2D();
        EndDrawing();
        // clang-format on
    }
    StopMusicStream(m);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
