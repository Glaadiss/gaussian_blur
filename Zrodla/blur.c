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
    Pixel **data;
} Image;

extern Avg *sub(Pixel *first, Pixel *second)
{
    Avg *px = malloc(sizeof(Avg));
    px->r = first->r - second->r;
    px->g = first->g - second->g;
    px->b = first->b - second->b;
    return px;
}

extern void addTo(Avg *to, Avg *from)
{
    to->r += from->r;
    to->g += from->g;
    to->b += from->b;
}

extern void addToP(Avg *to, Pixel *from)
{
    to->r += from->r;
    to->g += from->g;
    to->b += from->b;
}

extern void mulTo(Avg *to, double from)
{
    to->r *= from;
    to->g *= from;
    to->b *= from;
}

extern void init(Avg *avg, Pixel *px)
{
    avg->r = px->r;
    avg->g = px->g;
    avg->b = px->b;
}

extern void box_blur_h(Image *source, Image *target, int w, int h, int radius)
{
    Avg *avg = malloc(sizeof(Avg));
    double iarr = (double)1 / (radius + radius + 1); //
    for (int i = 0; i < h; i++)
    {
        int li = 0;
        int ri = radius;
        Pixel left_pixel = source->data[i][0];
        Pixel right_pixel = source->data[i][w - 1];
        init(avg, &left_pixel);
        mulTo(avg, radius + 1);
        for (int j = 0; j < radius; j++)
        {
            Pixel pixel = source->data[i][j];
            addToP(avg, &pixel);
        }
        for (int j = 0; j < w; j++)
        {
            Pixel pixel = source->data[i][ri % w];
            Pixel second_pixle = source->data[i][li % w];
            if (j <= radius)
            {
                ri++;
                addTo(avg, sub(&pixel, &left_pixel));
            }
            else if (j < w - radius)
            {
                ri++;
                li++;
                addTo(avg, sub(&pixel, &second_pixle));
            }
            else
            {
                li++;
                addTo(avg, sub(&right_pixel, &pixel));
            }

            Pixel result = {avg->r * iarr, avg->g * iarr, avg->b * iarr};
            target->data[i][j] = result;
        }
    }
}

extern void box_blur(Image *source, Image *target, int w, int h, int radius)
{
    box_blur_h(target, source, w, h, radius);
}

extern Image *gaussian_blur(Image *source, Image *target, double radius)
{
    int width = source->width;
    int height = source->height;
    radius = 5;

    box_blur_h(source, target, width, height, radius);

    printf("(%f)\n", radius);
    return target;
}
