#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct pixel_t
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} __attribute__((__packed__)) Pixel;

typedef struct avg_t
{
    unsigned int r;
    unsigned int g;
    unsigned int b;
} __attribute__((__packed__)) Avg;

typedef struct image_t
{
    uint32_t width;
    uint32_t height;
    Pixel **data;
} Image;

Pixel get_pixel(Image *source, int index)
{
    Pixel result;
    int i = index / source->width;
    int j = index % source->width;
    result = source->data[i][j];
    return result;
}

extern void copy_image(Image *source, Image *target)
{
    for (uint32_t row = 0; row < source->height; row++)
    {
        for (uint32_t coll = 0; coll < source->width; coll++)
        {
            target->data[row][coll] = source->data[row][coll];
        }
    }
}

extern void set_pixel(Image *source, int index, int r, int g, int b)
{
    int i = index / source->width;
    int j = index % source->width;

    if (r < 0)
        r = 0;
    if (r > 255)
        r = 255;

    if (g < 0)
        g = 0;
    if (g > 255)
        g = 255;

    if (b < 0)
        b = 0;
    if (b > 255)
        b = 255;

    Pixel result = {r, g, b};
    source->data[i][j] = result;
}

extern Pixel *sub(Pixel *first, Pixel *second)
{
    Pixel px;
    px.r = first->r - second->r;
    px.g = first->g - second->g;
    px.b = first->b - second->b;
    return &px;
}

extern void *addTo(Avg *to, Pixel *from)
{
    to->r += from->r;
    to->g += from->g;
    to->b += from->b;
}

extern void box_blur_h(Image *source, Image *target, int w, int h, int radius)
{
    double iarr = (double)1 / (radius + radius + 1); //
    for (int i = 0; i < h; i++)
    {
        int li = 0;
        int ri = radius;
        Pixel left_pixel = source->data[i][0];
        Pixel right_pixel = source->data[i][w - 1];
        // Avg *avg = malloc(typeof(Avg));
        unsigned currennt_r = left_pixel.r * (radius + 1);
        unsigned currennt_g = left_pixel.g * (radius + 1);
        unsigned currennt_b = left_pixel.b * (radius + 1);

        for (int j = 0; j < radius; j++)
        {
            Pixel pixel = source->data[i][j];
            currennt_r += pixel.r;
            currennt_g += pixel.g;
            currennt_b += pixel.b;
        }

        for (int j = 0; j < w; j++)
        {
            Pixel pixel = source->data[i][ri % w];
            Pixel second_pixle = source->data[i][li % w];

            if (j <= radius)
            {
                ri++;
                currennt_r += (pixel.r - left_pixel.r);
                currennt_g += (pixel.g - left_pixel.g);
                currennt_b += (pixel.b - left_pixel.b);
            }
            else if (j < w - radius)
            {
                ri++;
                li++;
                currennt_r += (pixel.r - second_pixle.r);
                currennt_g += (pixel.g - second_pixle.g);
                currennt_b += (pixel.b - second_pixle.b);
            }
            else
            {
                li++;
                currennt_r += (right_pixel.r - pixel.r);
                currennt_g += (right_pixel.g - pixel.g);
                currennt_b += (right_pixel.b - pixel.b);
            }
            Pixel result = {currennt_r * iarr, currennt_g * iarr, currennt_b * iarr};
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
    radius = 4;

    box_blur_h(source, target, width, height, radius);

    printf("(%f)\n", radius);
    return target;
}
