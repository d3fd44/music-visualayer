#include "ft.h"

// #define PI8 pi / 8

double pi;

void dft(float *inv, float complex *outv, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        outv[i] = 0;
        for (size_t k = 0; k < n; k++)
        {
            outv[i] += inv[k] * cexp(2 * I * pi * k * i / n);
        }
    }
}

void fft(float *inv, float complex *outv, size_t n, int step)
{
    assert(n > 0);

    if (n == 1)
    {
        outv[0] = inv[0];
        return;
    }

    fft(inv, outv, n / 2, step * 2);
    fft(inv + step, outv + n / 2, n / 2, step * 2);

    for (size_t i = 0; i < n / 2; i++)
    {
        float complex v = cexp(2 * I * pi * i / n) * outv[i + n / 2];
        float complex e = outv[i];
        outv[i] = e + v;
        outv[i + n / 2] = e - v;
    }
}

// int main()
// {
//     pi = atan2(1, 1) * 4;
//
//     float         in[1024 * 128];
//     float complex out[1024 * 128] = { 0 };
//
//     for (size_t i = 0; i < 1024 * 128; i++)
//         in[i] = cosf(PI8 * i) + sinf(PI8 * 3 * i) + sinf(pi * i * 7.3f);
//
//     fft(in, out, 1024 * 128, 1);
//
//     printf("in : ");
//     for (size_t i = 0; i < 1024; i++)
//         printf("% 5.3f ", in[i]);
//
//     printf("\nout: ");
//     for (size_t i = 0; i < 1024; i++)
//         printf("% 5.3f ", cabsf(out[i]));
//
//     printf("\n");
//
//     dft(in, out, 1024 * 128);
//     printf("in : ");
//     for (size_t i = 0; i < 1024; i++)
//         printf("% 5.3f ", in[i]);
//
//     printf("\nout: ");
//     for (size_t i = 0; i < 1024; i++)
//         printf("% 5.3f ", cabsf(out[i]));
//
//     printf("\n");
//     return 0;
// }
