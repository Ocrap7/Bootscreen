#include <linux/fb.h>
#include <linux/kd.h>
#include <linux/input.h>
#include <linux/vt.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <signal.h>
#include <syslog.h>
#include <png.h>

#define RGB(r, g, b) (0 | (r << 16) | (g << 8) | b)
#define RED(c) ((uint8_t)((c >> 16) & 0xFF))
#define GREEN(c) ((uint8_t)((c >> 8) & 0xFF))
#define BLUE(c) ((uint8_t)(c & 0xFF))

uint32_t *pixels;
int mouseX;
int mouseY;
struct fb_var_screeninfo *info;

int x, y;

int width, height;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep *row_pointers;

bool read_png_file(char *file_name)
{
    char header[8]; // 8 is the maximum size that can be checked

    /* open file and test for it being a png */
    FILE *fp = fopen(file_name, "rb");
    if (!fp)
        return false;
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8))
        return false;

    /* initialize stuff */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
        return false;

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
        return false;

    if (setjmp(png_jmpbuf(png_ptr)))
        return false;

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    /* read file */
    if (setjmp(png_jmpbuf(png_ptr)))
        return false;

    row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    for (y = 0; y < height; y++)
        row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png_ptr, info_ptr));

    png_read_image(png_ptr, row_pointers);

    fclose(fp);
    return true;
}

static void skeleton_daemon()
{
    pid_t pid;
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);
    umask(0);
    chdir("/");
    openlog("firstdaemon", LOG_PID, LOG_DAEMON);
}

uint32_t interp(uint32_t c1, uint32_t c2, double amt)
{
    uint32_t col = RGB(
        (uint32_t)((RED(c2) - RED(c1)) * amt + RED(c1)),
        (uint32_t)((GREEN(c2) - GREEN(c1)) * amt + GREEN(c1)),
        (uint32_t)((BLUE(c2) - BLUE(c1)) * amt + BLUE(c1)));
    return col;
}

void rect(int x, int y, int w, int h, uint32_t color)
{
    for (int yy = y; yy < y + h; yy++)
        for (int xx = x; xx < x + w; xx++)
            pixels[yy * info->xres_virtual + xx] = color;
}

bool draw_file(uint32_t ix, uint32_t iy, uint32_t nwidth, uint32_t nheight)
{
    if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
        return false;

    if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA)
        return false;

    double wratio = (double)width / (double)nwidth;
    double hratio = (double)height / (double)nheight;
    int sy = iy;
    for (y = 0; sy < nheight + iy; y = round((double)y + hratio), sy++)
    {
        int sx = ix;
        png_byte *row = row_pointers[y];
        for (x = 0; sx < nwidth + ix; x = round((double)x + wratio), sx++)
        {
            png_byte *ptr = &(row[x * 4]);
            if (ptr[3] == 0)
                continue;
            pixels[sy * info->xres_virtual + sx] = RGB(ptr[0], ptr[1], ptr[2]);
            /* set red value to 0 and green value to the blue one */
            ptr[0] = 0;
            ptr[1] = ptr[2];
        }
    }
}

#define DRAW_IMG_CENTER 1

bool draw_files(uint32_t ix, uint32_t iy, double scale, uint16_t flags)
{
    if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
        return false;

    if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA)
        return false;
    double nwidth = (double)width * scale;
    double nheight = (double)height * scale;
    double wratio = (double)width / nwidth;
    double hratio = (double)height / nheight;
    if (flags & DRAW_IMG_CENTER)
    {
        ix -= nwidth / 2;
        iy -= nheight / 2;
    }
    int sy = iy;
    for (y = 0; sy < nheight + iy; y = round((double)y + hratio), sy++)
    {
        int sx = ix;
        png_byte *row = row_pointers[y];
        for (x = 0; sx < nwidth + ix; x = round((double)x + wratio), sx++)
        {
            png_byte *ptr = &(row[x * 4]);
            if (ptr[3] == 0)
                continue;
            pixels[sy * info->xres_virtual + sx] = RGB(ptr[0], ptr[1], ptr[2]);
            /* set red value to 0 and green value to the blue one */
            ptr[0] = 0;
            ptr[1] = ptr[2];
        }
    }
}

int main(int argc, char **argv)
{

    int term = open("/dev/tty1", O_RDWR);
    struct vt_sizes size = {.v_rows = 100, .v_cols = 100, .v_scrollsize = 0};
    ioctl(term, VT_RESIZE, &size);
    int fb = open("/dev/fb0", O_RDWR);
    struct fb_var_screeninfo _info;
    info = &_info;
    ioctl(fb, FBIOGET_VSCREENINFO, info);
    size_t len = 4 * info->xres_virtual * info->yres_virtual;
    pixels = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);

    read_png_file("/etc/daikonSplash/daikon.png");

    ioctl(term, KDSETMODE, KD_GRAPHICS);
    unsigned long previousMode;
    ioctl(term, KDGKBMODE, &previousMode);
    ioctl(term, KDSKBMODE, K_OFF);
    ioctl(term, 0x4B51, 1);

    skeleton_daemon();

    syslog(LOG_NOTICE, "First daemon started.");
    rect(0, 0, info->xres, info->yres, RGB(43, 43, 43));
    draw_files(info->xres / 2, info->yres / 2, 0.25, DRAW_IMG_CENTER);
    sleep(10);

    ioctl(term, KDSETMODE, KD_TEXT);
    ioctl(term, KDSKBMODE, previousMode);
    ioctl(term, 0x4B51, 0);

    syslog(LOG_NOTICE, "First daemon terminated.");
    closelog();

    return EXIT_SUCCESS;

    return 0;
}