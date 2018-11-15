#include <stdio.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

typedef struct pixel_t
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Pixel;

typedef struct avg_t
{
    unsigned int r;
    unsigned int g;
    unsigned int b;
} Avg;

typedef struct image_t_int
{
    uint32_t width;
    uint32_t height;
    Pixel **data;
} __attribute__((__packed__)) Image;

typedef struct image_t
{
    uint32_t width;
    uint32_t height;
    unsigned int ***data;
} __attribute__((__packed__)) ImageInt;

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

// common
void write_pixel_to_file(ImageInt *const source)
{
    FILE *file = fopen("pixel.data", "w");
    for (int h = 0; h < 10; ++h)
    {
        for (int w = 0; w < source->width; ++w)
        {
            unsigned int *current_pixel = source->data[h][w];
            fprintf(file, "(%d,%d,%d)", (int)current_pixel[0], (int)current_pixel[1], (int)current_pixel[2]);
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

void free_imageInt(ImageInt *source)
{
    for (unsigned i = 0; i < source->height; ++i)
    {
        free(source->data[i]);
    }
    free(source->data);
    free(source);
}

void copy_image(Image *source, ImageInt *target)
{
    int width = source->width;
    int height = source->height;
    target->width = width;
    target->height = height;
    target->data = calloc(height, sizeof(int *));
    for (int row = 0; row < height; row++)
    {
        target->data[row] = calloc(width, sizeof(int *));
    }
    for (int row = 0; row < source->height; row++)
    {
        for (int coll = 0; coll < source->width; coll++)
        {
            target->data[row][coll] = calloc(3, sizeof(int));
        }
        for (int coll = 0; coll < source->width; coll++)
        {
            target->data[row][coll][0] = source->data[row][coll].r;
            target->data[row][coll][1] = source->data[row][coll].g;
            target->data[row][coll][2] = source->data[row][coll].b;
        }
    }
}

void copy_for_bmp(ImageInt *source, Image *target)
{
    int width = source->width;
    int height = source->height;
    target->width = width;
    target->height = height;
    target->data = calloc(height, sizeof(Avg *));
    for (int row = 0; row < height; row++)
    {
        target->data[row] = calloc(width, sizeof(Avg));
    }
    for (int row = 0; row < source->height; row++)
    {
        for (int coll = 0; coll < source->width; coll++)
        {
            target->data[row][coll].r = source->data[row][coll][0];
            target->data[row][coll].g = source->data[row][coll][1];
            target->data[row][coll].b = source->data[row][coll][2];
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
write_error_code_t to_bmp(FILE *out, ImageInt *const img);

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

write_error_code_t to_bmp(FILE *out, ImageInt *const imgI)
{
    Image *img = malloc(sizeof(Image));
    copy_for_bmp(imgI, img);
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
    free(img);
    return WRITE_OK;
}

// Pixel **(*funkcja)(Image *, int);
Image *(*fun1)(ImageInt *, ImageInt *, int);
// Avg *(*pixFunc)(Avg **, int, int, Avg **);
unsigned int *(*pixFunc)(unsigned int **, int, int, unsigned int **, int radius);

void *Biblioteka;
const char *error;

void *cpp_function(ImageInt *target, ImageInt *current_image, int b)
{
    // printf("371 %d \n\n", 10);

    Biblioteka = dlopen("../build/blur.dylib", RTLD_LAZY);
    error = dlerror();
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }
    *(void **)(&fun1) = dlsym(Biblioteka, "gaussian_blur");
    Image *wynik = (*fun1)(target, current_image, b);
    dlclose(Biblioteka);
    return NULL;
}

void asm_function(unsigned int **pixel, int max, int width, unsigned int **data)
{
    Biblioteka = dlopen("../build/libDLL_ASM.dylib", RTLD_LAZY);
    // printf("PIXE %d \n", pixel[0]->r);
    error = dlerror();
    *(void **)(&pixFunc) = dlsym(Biblioteka, "gaussian_blur");
    // (*pixFunc)(pixel, max, width, data);
    // unsigned int *avg2 = pixel[0][0];
    // printf("left: %lu \n right%lu\n DOWN:%lu\n", &pixel[0][0], &pixel[0][1], &pixel[1][0]);
    // printf("RET: (%d,%d,%d) \n", avg2[0], avg2[1], avg2[2]);
    printf("RET2: %d\n", (*pixFunc)(pixel, max, width, data, 5));
    // unsigned int *avg = (*pixFunc)(pixel, max, width, data);
    // printf("RET2: (%d,%d,%d) \n", avg[0], avg[1], avg[2]);
    // register int i asm("xmm2");
    // printf("register: %d \n", i);
    // Pixel *pix = (*pixFunc)(pixel, max, width, data);
    // Pixel *p = pix;
    // printf("PIXE works \n");
    // printf("(%d,%d,%d)\n", p->r, p->g, p->b);
    // printf("r: %d\n", pixe);
    // printf("PIXE %d %d %d \n", pixe[0]->r, pixe[0]->g, pixe[0]->b);
    dlclose(Biblioteka);
}

struct arg_struct
{
    unsigned ***startFrom;
    int current;
    int allParts;
    ImageInt *image;
    ImageInt *currentImage;
};
unsigned int **getRow(ImageInt *image, int current, int allParts)
{
    return &image->data[current * image->height / allParts];
}

void *SEND_DATA(void *arguments)
{

    struct arg_struct *args = arguments;
    // if (args->current == 1 || args->current == 2)
    // {
    //     return NULL;
    // }
    int width = args->image->width * 8;
    int height = args->image->height * 8 / args->allParts;
    ImageInt *image = args->image;
    ImageInt *current = args->currentImage;
    unsigned int **imageRow = getRow(image, args->current, args->allParts);
    unsigned int **currentRow = getRow(current, args->current, args->allParts);

    asm_function(imageRow, height, width, currentRow);

    return NULL;
}

void handleThreads(int n, ImageInt *data, ImageInt *currentImage)
{
    int rowsNumber = data->height / n;
    int thread_cmp_count = n;
    int t, index, thread = 0;
    pthread_t *cmp_thread = malloc(thread_cmp_count * sizeof(pthread_t));
    struct arg_struct *args = malloc(thread_cmp_count * sizeof(struct arg_struct));

    for (int i = 0; i < n; i++)
    {
        struct arg_struct arg;
        arg.startFrom = &data->data[i * rowsNumber];
        arg.current = i;
        arg.allParts = n;
        arg.image = data;
        arg.currentImage = currentImage;
        args[i] = arg;
        pthread_create(&cmp_thread[i], NULL, &SEND_DATA, (void *)&args[i]);
    }

    for (int i = 0; i < n; i++)
    {
        pthread_join(cmp_thread[i], NULL);
    }

    free(args);
    free(cmp_thread);
}

int main(int argc, char *argv[])
{
    int num = strtol(argv[1], NULL, 10);
    int numofcpus = (int)sysconf(_SC_NPROCESSORS_ONLN);

    printf("Cores number: %d\n", numofcpus);

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

    // allocate image
    ImageInt *target = malloc(sizeof(ImageInt));

    ImageInt *tmpTarget = malloc(sizeof(ImageInt));

    copy_image(current_image, target);
    copy_image(current_image, tmpTarget);

    double startTime = (float)clock() / CLOCKS_PER_SEC;
    // cpp_function(tmpTarget, target, 5);
    double endTime = (float)clock() / CLOCKS_PER_SEC;
    printf("CPP elapsed: %f\n", endTime - startTime);

    // startTime = (float)clock() / CLOCKS_PER_SEC;
    // handleThreads(1, tmpTarget, target);
    // endTime = (float)clock() / CLOCKS_PER_SEC;
    // printf("ASM elapsed: %f\n", endTime - startTime);

    startTime = (float)clock() / CLOCKS_PER_SEC;
    handleThreads(3, tmpTarget, target);
    endTime = (float)clock() / CLOCKS_PER_SEC;
    printf("ASM 4 elapsed: %f\n", endTime - startTime);

    printf("DONE\n");
    write_pixel_to_file(target);

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
    free_imageInt(target);
    free_imageInt(tmpTarget);
    return 0;
}