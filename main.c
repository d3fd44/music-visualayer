#include <complex.h>
#include <math.h>
#include <raylib.h>
#include <string.h>
#include <unistd.h>

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
    char *path = argv[1];
    assert(!access(path, F_OK));

    SetTraceLogLevel(LOG_NONE);
    InitWindow(0, 0, "Visualayer");
    SetTargetFPS(60);

    int   screenHeight = GetScreenHeight();
    int   screenWidth = GetScreenWidth();
    float maxpreview = screenHeight * 0.75;
    float maxlog = 4.0f;  // global scale

    Camera2D cam = { 0 };
    cam.target = (Vector2){ 0.0f, 0.0f };
    cam.offset = (Vector2){ 0.0f, screenHeight };
    cam.zoom = 1.0f;

    InitAudioDevice();

    Music m = LoadMusicStream(path);
    AttachAudioStreamProcessor(m.stream, get_data);
    PlayMusicStream(m);
    SetMusicVolume(m, 0.5f);

    float dx = (float)(screenWidth) / 512;
    float sec = 0.0f;

    // for smooth transition
    float lastnormal[256] = { 0 };
    float raise = 0.3f;
    float decay = 0.15f;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_RIGHT))
        {
            float len = GetMusicTimeLength(m);
            sec = GetMusicTimePlayed(m);
            sec = sec < len - 5.0f ? sec + 5.0f : len;
            SeekMusicStream(m, sec);
        }
        if (IsKeyPressed(KEY_LEFT))
        {
            sec = GetMusicTimePlayed(m);
            sec = sec > 5.0f ? sec - 5.0f : 0.0f;
            SeekMusicStream(m, sec);
        }

        // clang-format off
        UpdateMusicStream(m);
        for (int i = 0; i < 512; i++) { c1s[i] = frames[i].c1; c2s[i] = frames[i].c2; }  // copy samples

        if (IsKeyPressed(KEY_SPACE))
        {
            if (IsMusicStreamPlaying(m)) PauseMusicStream(m);
            else                         ResumeMusicStream(m);
        }

        // perform FFT on copied samples earlier
        fft(c1s, freqs1, 512, 1);
        fft(c2s, freqs2, 512, 1);

        float outmags[256] = { 0 };

        for (int i = 0; i < 256; i += 2)
        {
            // take average over 2 frequencies
            float magleftc = cabsf(freqs1[i] + freqs1[i + 1]) / 2;
            float magrightc = cabsf(freqs2[i] + freqs2[i + 1]) / 2;

            float logleftc = log10f(magleftc + 1);  // 1 to prevent log10(0)
            float logrightc = log10f(magrightc + 1);

            // frequencies for the left channel goes first (normal - left to right)
            outmags[i / 2] = logleftc;
            // then frequencies for the right channel (reversed right to left)
            outmags[255 - i / 2] = logrightc;  
        }

        BeginDrawing();
            BeginMode2D(cam);

                ClearBackground(BLACK);
                DrawFPS(0, 0);

                for (size_t i = 0; i < 256; i++)
                {
                    float sum = 0.0f;
                    int   smooth_radius = 18;
                    int   smooth_count = (smooth_radius * 2) + 1;  // 18 left, 18 right + center = 37

                    // smooth curve (accross rectangles)
                    for (int j = -smooth_radius; j <= smooth_radius; j++)
                    {
                        int target = (int)i + j;

                        if (target < 0) target = 0;
                        if (target > 255) target = 255;

                        sum += outmags[target];
                    }

                    float normal = (sum / smooth_count) / maxlog;
                    if (normal > 1.0f) normal = 1.0f;

                    if (normal < lastnormal[i]) lastnormal[i] += (normal - lastnormal[i]) * decay;
                    if (normal > lastnormal[i]) lastnormal[i] += (normal - lastnormal[i]) * raise;

                    float recth = lastnormal[i] * maxpreview;
                    float rectw = dx * 0.7;
                    float x = (i * dx * 2);
                    float y = -recth;

                    Vector2 pos = { x, y - 100 };
                    Vector2 size = { rectw, recth };
                    DrawRectangleV(pos, size, BLUE);
                }

            EndMode2D();
        EndDrawing();
    }

    StopMusicStream(m);
    CloseAudioDevice();
    CloseWindow();
    // clang-format on

    return 0;
}
