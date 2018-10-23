#include <stdio.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>

typedef struct pixel_t
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} __attribute__((__packed__)) Pixel;

typedef struct image_t
{
    uint32_t width;
    uint32_t height;
    Pixel **data;
} Image;

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

Image *rotate(Image *const source, unsigned angle)
{
    if (angle > 360)
    {
        fprintf(stderr, "Undefined angle; please write number > 0 && < 360");
        return NULL;
    }
    float radian = (2 * 3.1416 * angle) / 360;

    double cosine = (float)cos(radian);
    double sine = (float)sin(radian);

    unsigned height = source->height;
    unsigned width = source->width;

    float point1_x = ((float)(-1) * height * sine);
    float point1_y = (height * cosine);
    float point2_x = (width * cosine - height * sine);
    float point2_y = (height * cosine + width * sine);
    float point3_x = (width * cosine);
    float point3_y = (width * sine);

    float min_x = min(0, min(point1_x, min(point2_x, point3_x)));
    float min_y = min(0, min(point1_y, min(point2_y, point3_y)));

    float max_x = max(point1_x, max(point2_x, point3_x));
    float max_y = max(point1_y, max(point2_y, point3_y));

    int dest_width = (int)ceil(fabs(max_x) - min_x);
    int dest_height = (int)ceil(fabs(max_y) - min_y);

    Image *data_image = malloc(sizeof(Image));
    data_image->width = dest_width;
    data_image->height = dest_height;

    // allocate memory
    data_image->data = calloc(dest_height, sizeof(Pixel *));
    for (unsigned row = 0; row < dest_height; ++row)
    {
        data_image->data[row] = calloc(dest_width, sizeof(Pixel));
    }

    Pixel white_pixel = {255, 255, 255};

    // initialize
    for (int y = 0; y < dest_height; ++y)
    {
        for (int x = 0; x < dest_width; ++x)
        {
            int src_x = (int)((x + min_x) * cosine + (y + min_y) * sine);
            int src_y = (int)((y + min_y) * cosine - (x + min_x) * sine);
            if (src_x >= 0 && src_x < width && src_y >= 0 && src_y < height)
            {
                data_image->data[y][x] = source->data[src_y][src_x];
            }
            else
            {
                data_image->data[y][x] = white_pixel;
            }
        }
    }
    return data_image;
}

// common
void write_pixel_to_file(Image *const source)
{
    FILE *file = fopen("../build/pixel.data", "w");
    for (int h = 0; h < 100; ++h)
    {
        for (int w = 0; w < 100; ++w)
        {
            Pixel current_pixel = source->data[h][w];
            fprintf(file, "(%d,%d,%d)", (int)current_pixel.r, (int)current_pixel.g, (int)current_pixel.b);
        }
        fprintf(file, "\n");
    }
}

void free_image(Image *source)
{
    for (unsigned i = 0; i < source->height; ++i)
    {
        free(source->data[i]);
    }
    free(source->data);
    free(source);
}

void copy_image(Image *source, Image *target)
{
    for (int row = 0; row < source->height; row++)
    {
        for (int coll = 0; coll < source->width; coll++)
        {
            target->data[row][coll] = source->data[row][coll];
        }
    }
}

Pixel get_pixel(Image *source, int index)
{
    Pixel result;
    int i = index / source->width;
    int j = index % source->width;
    result = source->data[i][j];
    return result;
}

void set_pixel(Image *source, int index, int r, int g, int b)
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

typedef enum
{
    READ_OK = 0,
    READ_INVALID_SIGNATURE,
    READ_INVALID_BITS,
    READ_INVALID_HEADER
} read_error_code_t;

typedef struct
{
    uint16_t bfType;
    uint32_t bfileSize;
    uint32_t bfReserved;
    uint32_t bOffBits;
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} __attribute__((__packed__)) bmp_head;

typedef enum
{
    WRITE_OK = 0,
    WRITE_ERROR
} write_error_code_t;

// function
read_error_code_t from_bmp(FILE *in, Image **read);
write_error_code_t to_bmp(FILE *out, Image *const img);

read_error_code_t from_bmp(FILE *in, Image **read)
{
    // read head
    bmp_head head;
    if (fread(&head, sizeof(head), 1, in) != 1)
    {
        return READ_INVALID_HEADER;
    }
    if (head.biBitCount != 24)
    {
        return READ_INVALID_BITS;
    }
    // in BPM image first two byte equal string "BM"
    if (head.bfType != 0x4d42)
    {
        return READ_INVALID_SIGNATURE;
    }

    (*read) = malloc(sizeof(Image));
    (*read)->height = head.biHeight;
    (*read)->width = head.biWidth;

    int padding = (4 - ((*read)->width * sizeof(Pixel)) % 4) % 4;
    (*read)->data = calloc(head.biHeight, sizeof(Pixel *));

    for (unsigned row = 0; row < head.biHeight; ++row)
    {
        (*read)->data[row] = calloc(head.biWidth, sizeof(Pixel));
        fread(((*read)->data[row]), sizeof(Pixel), head.biWidth, in);
        fseek(in, padding, SEEK_CUR);
    }
    return READ_OK;
}

write_error_code_t to_bmp(FILE *out, Image *const img)
{
    // initialize header
    bmp_head head;
    head.bfType = 0x4d42;
    head.bfileSize = img->width * img->height + sizeof(Pixel) + sizeof(bmp_head);
    head.bfReserved = 0;
    head.bOffBits = 54;
    head.biSize = 40;
    head.biWidth = img->width;
    head.biHeight = img->height;
    head.biPlanes = 1;
    head.biBitCount = 24;
    head.biCompression = 0;
    head.biSizeImage = img->width * img->height + sizeof(Pixel);
    head.biXPelsPerMeter = 0;
    head.biYPelsPerMeter = 0;
    head.biClrUsed = 0;
    head.biClrImportant = 0;

    if (fwrite(&head, sizeof(bmp_head), 1, out) != 1)
    {
        return WRITE_ERROR;
    }
    const int padding = (4 - (img->width * sizeof(Pixel)) % 4) % 4;
    unsigned char trash[padding];

    const unsigned long write_size = sizeof(Pixel) * img->width;
    for (unsigned coll = 0; coll < img->height; ++coll)
    {
        // write content
        if (fwrite(img->data[coll], write_size, 1, out) != 1)
        {
            return WRITE_ERROR;
        };
        // write padding
        if (padding != 0)
        {
            if (fwrite(&trash, padding, 1, out) != 1)
            {
                return WRITE_ERROR;
            }
        }
    }
    return WRITE_OK;
}

// Pixel **(*funkcja)(Image *, int);
Image *(*fun1)(Image *, int);

void *Biblioteka;
const char *error;

Image *cpp_function(Image *target, int b)
{
    Biblioteka = dlopen("../build/blur.dylib", RTLD_LAZY);
    error = dlerror();
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }
    *(void **)(&fun1) = dlsym(Biblioteka, "gaussian_blur");
    Image *wynik = (*fun1)(target, b);
    dlclose(Biblioteka);
    return wynik;
}

Image *asm_function(Image *target, int b)
{
    Biblioteka = dlopen("../build/libDLL_ASM.dylib", RTLD_LAZY);

    error = dlerror();
    *(void **)(&fun1) = dlsym(Biblioteka, "sumuj");
    // *(void **)(&fun1) = dlsym(Biblioteka2, "sumuj");

    // printf("nr1: %d\n", (*fun1)(target, 4));

    Image *wynik = (*fun1)(target, 4);
    dlclose(Biblioteka);

    return wynik;
}

int main(int argc, char *argv[])
{
    FILE *file_in = fopen("big.bmp", "rb");
    if (!file_in)
    {
        fprintf(stderr, "Can`t open file to read");
        return -1;
    }
    Image *current_image;

    read_error_code_t read_code = from_bmp(file_in, &current_image);

    fclose(file_in);

    switch (read_code)
    {
    case READ_INVALID_HEADER:
        fprintf(stderr, "can`t read header of BMP image\n");
        return -1;
    case READ_INVALID_SIGNATURE:
        fprintf(stderr, "Invalid signature in image file\n");
        return -1;
    case READ_INVALID_BITS:
        fprintf(stderr, "Invalid bits bmp file\n");
        return -1;
    case READ_OK:
        printf("Image successfully read\n");
        break;
    default:
        fprintf(stderr, "Undefined error\n");
        return -1;
    }

    // debugger
    write_pixel_to_file(current_image);

    Image *target;
    unsigned char chouse = 'n';

    // printf("nr: %d\n", (asm_function(current_image, 5)));
    // Pixel pixl = (asm_function(current_image, 5))[1][1];

    // pixl = current_image->data[0][0];
    // printf("width: %d\n", (asm_function(current_image, 5))->width);
    // printf("(%d,%d,%d)\n", (int)pixl.r, (int)pixl.g, (int)pixl.b);
    target = cpp_function(current_image, 5);

    if (!target)
    {
        fprintf(stderr, "Can`t blur image");
        return -1;
    }

    FILE *file_out = fopen("../build/big.bmp", "wb");
    if (!file_out)
    {
        fprintf(stderr, "Can`t open file to write");
        return -1;
    }

    write_error_code_t write_code = to_bmp(file_out, target);

    fclose(file_out);
    switch (write_code)
    {
    case WRITE_ERROR:
        fprintf(stderr, "can`t write BMP image");
        return -1;
    case WRITE_OK:
        printf("Image successfully saved\n");
        break;
    default:
        fprintf(stderr, "Undefined error");
        return -1;
    }

    free_image(current_image);
    free_image(target);
    return 0;
}