#include <stdio.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <sched.h>

#include <cpuid.h>

#define CPUID(INFO, LEAF, SUBLEAF) __cpuid_count(LEAF, SUBLEAF, INFO[0], INFO[1], INFO[2], INFO[3])

#define GETCPU(CPU)                                     \
    {                                                   \
        uint32_t CPUInfo[4];                            \
        CPUID(CPUInfo, 1, 0);                           \
        /* CPUInfo[1] is EBX, bits 24-31 are APIC ID */ \
        if ((CPUInfo[3] & (1 << 9)) == 0)               \
        {                                               \
            CPU = -1; /* no APIC on chip */             \
        }                                               \
        else                                            \
        {                                               \
            CPU = (unsigned)CPUInfo[1] >> 24;           \
        }                                               \
        if (CPU < 0)                                    \
            CPU = 0;                                    \
    }
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

Image *(*fun1)(ImageInt *, ImageInt *, int, int, int);
unsigned int *(*pixFunc)(unsigned int **, int, int, unsigned int **, int radius);

void *Biblioteka;
const char *error;

void *cpp_function(ImageInt *target, ImageInt *current_image, int b, int start, int end)
{

    Biblioteka = dlopen("../build/blur.dylib", RTLD_LAZY);
    error = dlerror();
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }
    *(void **)(&fun1) = dlsym(Biblioteka, "gaussian_blur");
    Image *wynik = (*fun1)(target, current_image, b, start, end);
    dlclose(Biblioteka);
    return NULL;
}

void asm_function(unsigned int **pixel, int max, int width, unsigned int **data, int radius)
{
    Biblioteka = dlopen("../build/libDLL_ASM.dylib", RTLD_LAZY);
    error = dlerror();
    *(void **)(&pixFunc) = dlsym(Biblioteka, "gaussian_blur");
    (*pixFunc)(pixel, max, width, data, radius);
    dlclose(Biblioteka);
}

struct arg_struct
{
    unsigned ***startFrom;
    int current;
    int allParts;
    ImageInt *image;
    ImageInt *currentImage;
    int mode;
    int radius;
};
unsigned int **getRow(ImageInt *image, int current, int allParts)
{

    return &image->data[current * image->height / allParts];
}

void *SEND_DATA(void *arguments)
{
    int a;
    GETCPU(a);
    printf("CPU %d \n", a);
    struct arg_struct *args = arguments;
    int width = args->image->width * 8;
    int height = args->image->height / args->allParts;
    height += (args->current > (height % args->allParts)) ? 1 : 1;
    height -= (args->current == args->allParts - 1) ? 1 : 0;
    height *= 8;

    ImageInt *image = args->image;
    ImageInt *current = args->currentImage;
    unsigned int **imageRow = getRow(image, args->current, args->allParts);
    unsigned int **currentRow = getRow(current, args->current, args->allParts);
    int max = image->height * args->current / args->allParts;
    if (args->mode == 1)
    {
        cpp_function(image, current, args->radius, max - (args->current ? 1 : 0), args->image->height / args->allParts + max);
    }
    if (args->mode == 2)
    {
        asm_function(imageRow, height, width, currentRow, args->radius);
    }

    return NULL;
}

void handleThreads(int n, ImageInt *data, ImageInt *currentImage, int mode, int radius)
{
    n = n % data->height;

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
        arg.mode = mode;
        arg.radius = radius;
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

int chooseMode(char *arg)
{
    if (strcmp("c", arg) == 0)
    {
        return 1;
    }
    if (strcmp("asm", arg) == 0)
    {
        return 2;
    }
    return 3;
}

int main(int argc, char *argv[])
{
    int mode = chooseMode(argv[1]);
    int numofcpus = (int)sysconf(_SC_NPROCESSORS_ONLN);
    int radius = strtol(argv[2], NULL, 10);
    char *imgName = argv[3];
    int num = argc < 5 ? numofcpus : strtol(argv[4], NULL, 10);
    num = num < 1 ? 1 : num;

    FILE *file_in = fopen(imgName, "rb");
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
    if (mode == 1 || mode == 3)
    {
        double startTime = (float)clock() / CLOCKS_PER_SEC;
        handleThreads(num, tmpTarget, target, 1, radius);
        double endTime = (float)clock() / CLOCKS_PER_SEC;
        unsigned int *a = target->data[177][200];
        unsigned int *b = target->data[178][200];
        printf("C -  elapsed: %f\n", endTime - startTime);
    }
    if (mode == 2 || mode == 3)
    {
        double startTime = (float)clock() / CLOCKS_PER_SEC;
        handleThreads(num, tmpTarget, target, 2, radius);
        double endTime = (float)clock() / CLOCKS_PER_SEC;
        printf("ASM %d threads -  elapsed: %f\n", num, endTime - startTime);
    }

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