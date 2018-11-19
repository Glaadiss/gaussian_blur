#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct pixel_t
{
    unsigned int r;
    unsigned int g;
    unsigned int b;
} Pixel;

typedef struct avg_t
{
    int r;
    int g;
    int b;
} Avg;

typedef struct image_t
{
    uint32_t width;
    uint32_t height;
    unsigned int ***data;
} Image;

extern Avg *sub(unsigned int *first, unsigned int *second)
{
    Avg *px = malloc(sizeof(Avg));
    px->r = first[0] - second[0];
    px->g = first[1] - second[1];
    px->b = first[2] - second[2];
    return px;
}

extern void addTo(Avg *to, Avg *from)
{
    to->r += from->r;
    to->g += from->g;
    to->b += from->b;
}

extern void addToP(Avg *to, unsigned int *from)
{
    to->r += from[0];
    to->g += from[1];
    to->b += from[2];
}

extern void mulTo(Avg *to, double from)
{
    to->r *= from;
    to->g *= from;
    to->b *= from;
}

extern void init(Avg *avg, unsigned int *px)
{
    avg->r = px[0];
    avg->g = px[1];
    avg->b = px[2];
}

extern void box_blur_h(Image *source, Image *target, int w, int end, int radius, int start)
{
    Avg *avg = malloc(sizeof(Avg));
    double iarr = (double)1 / (radius + radius + 1); //

    for (int i = start; i <= end + 1; i++)
    {
        int li = 0;
        int ri = radius;
        unsigned int *left_pixel = source->data[i][0];
        unsigned int *right_pixel = source->data[i][w - 1];
        init(avg, left_pixel);

        mulTo(avg, radius + 1);

        for (int j = 0; j < radius; j++)
        {
            unsigned int *pixel = source->data[i][j];
            addToP(avg, pixel);
        }

        for (int j = 0; j < w; j++)
        {
            unsigned int *pixel = source->data[i][ri % w];
            unsigned int *second_pixle = source->data[i][li % w];
            if (j <= radius)
            {
                ri++;
                addTo(avg, sub(pixel, left_pixel));
            }
            else if (j < w - radius)
            {
                ri++;
                li++;
                addTo(avg, sub(pixel, second_pixle));
            }
            else
            {
                li++;
                addTo(avg, sub(right_pixel, pixel));
            }

            unsigned int *result = calloc(3, sizeof(int));
            result[0] = (unsigned int)(avg->r * iarr);
            result[1] = (unsigned int)(avg->g * iarr);
            result[2] = (unsigned int)(avg->b * iarr);
            target->data[i][j] = result;
        }
    }
}
extern void box_blur_t(Image *source, Image *target, int w, int end, int radius, int start)
{
    Avg *avg = malloc(sizeof(Avg));
    double iarr = (double)1 / (radius + radius + 1); //
    if (start != 0)
    {
        start -= 1;
    }
    for (int i = 0; i < w; i++)
    {

        int li = start;
        int ri = start + radius - 1;
        unsigned int *left_pixel = source->data[start][i];
        unsigned int *right_pixel = source->data[end - 1][i];

        init(avg, left_pixel);

        mulTo(avg, radius + 1);
        for (int j = start; j < start + radius; j++)
        {
            unsigned int *pixel = source->data[j][i];
            addToP(avg, pixel);
        }

        for (int j = start; j < end; j++)
        {

            unsigned int *pixel = source->data[ri][i];

            unsigned int *second_pixle = source->data[li][i];

            if (j <= start + radius)
            {
                ri++;
                addTo(avg, sub(pixel, left_pixel));
            }
            else if (j < end - radius)
            {

                ri++;
                li++;
                addTo(avg, sub(pixel, second_pixle));
            }
            else
            {
                li++;
                addTo(avg, sub(right_pixel, pixel));
            }

            unsigned int *result = calloc(3, sizeof(int));
            result[0] = (unsigned int)(avg->r * iarr);
            result[1] = (unsigned int)(avg->g * iarr);
            result[2] = (unsigned int)(avg->b * iarr);

            target->data[j][i] = result;
        }
        // printf(" %d %d \n ", ri, li);
    }
}

// extern void box_blur(Image *source, Image *target, int w, int h, int radius)
// {
//     // box_blur_h(target, source, w, h, radius);
//     // box_blur_t(target, source, w, h, radius);
// }

extern Image *gaussian_blur(Image *source, Image *target, int radius, int start, int end)
{
    printf(" %d %d %d \n ", radius, start, end);
    int width = source->width;
    // box_blur_t(source, target, width, height, radius);
    // box_blur_t(source, target, width, end, radius, start);
    box_blur_h(source, target, width, end, radius, start);

    // printf("(%f)\n", radius);
    return target;
}
