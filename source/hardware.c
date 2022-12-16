//
// Created by Jonathon Moore on 11/1/2022.
//
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "structures.h"
#include "terasic_os_includes.h"
#include "LCD_Lib.h"
#include "lcd_graphic.h"
#include "font.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "address_map_arm.h"
#include <sys/mman.h>


#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

#define USER_IO_DIR     (0x01000000)
#define BIT_LED         (0x01000000)
#define BUTTON_MASK     (0x02000000)

#define TRUE ( 1 )
#define FALSE ( 0 )

// variables for LCD display
void *virtual_base_LCD;
int fd_LCD;
LCD_CANVAS LcdCanvas;

// variables for user button
void *virtual_base_USER_BUTTON;
int fd_USER_BUTTON;
uint32_t scan_input;
volatile int *KEY_ptr;

void * virtual_base;

// variables for lED/FPGA
int open_physical(int);
void * map_physical(int, unsigned int, unsigned int);
void close_physical(int);
int unmap_physical(void *, unsigned int);
volatile unsigned int *JP1_ptr;

int fd = -1;
void *LW_virtual;

/**
 * initialize all hardware and pointers to be used
 */
void initializeHardware()
{
    // initialize user button
    if( ( fd_USER_BUTTON = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
        printf( "ERROR: could not open \"/dev/mem\"...\n" );

    }

    virtual_base = mmap( NULL, LW_BRIDGE_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd_USER_BUTTON, LW_BRIDGE_BASE );

    if( virtual_base == MAP_FAILED ) {
        printf( "ERROR: mmap() failed...\n" );
        close( fd_USER_BUTTON );

    }

    KEY_ptr = virtual_base + KEY_BASE;

    printf("SUCCESS");

    // initialize LCD Display
    // map the address space for the LED registers into user space so we can interact with them.
    // we'll actually map in the entire CSR span of the HPS since we want to access various registers within that span
    if( ( fd_LCD = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 )
    {
        printf( "ERROR: could not open \"/dev/mem\"...\n" );
    }

    virtual_base_LCD = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd_LCD, HW_REGS_BASE );

    if( virtual_base_LCD == MAP_FAILED )
    {
        printf( "ERROR: mmap() failed...\n" );
        close( fd_LCD );
    }

    //printf("Can you see LCD?(CTRL+C to terminate this program)\r\n");
    printf("Graphic LCD Demo\r\n");

    LcdCanvas.Width = LCD_WIDTH;
    LcdCanvas.Height = LCD_HEIGHT;
    LcdCanvas.BitPerPixel = 1;
    LcdCanvas.FrameSize = LcdCanvas.Width * LcdCanvas.Height / 8;
    LcdCanvas.pFrame = (void *)malloc(LcdCanvas.FrameSize);

    if (LcdCanvas.pFrame == NULL)
    {
        printf("failed to allocate lcd frame buffer\r\n");
    }

    LCDHW_Init(virtual_base_LCD);
    LCDHW_BackLight(true); // turn on LCD backlight

    LCD_Init();
}

/**
 * Cleans up all pointers and memory map
 */
void cleanupHardware()
{
    // clear and cleanup LCD display
    //turn off the backlight
    DRAW_Clear(&LcdCanvas, LCD_BLACK);
    DRAW_Refresh(&LcdCanvas);
    LCDHW_BackLight(FALSE);

    free(LcdCanvas.pFrame);

    // clean up our memory mapping and exit
    if( munmap( virtual_base_LCD, HW_REGS_SPAN ) != 0 ) {
        printf( "ERROR: munmap() failed...\n" );
        close( fd_LCD );
    }


    // cleanup user buttons
    // Clean up our memory
    if (munmap(virtual_base_USER_BUTTON, HW_REGS_SPAN) != 0) {
        printf("ERROR: munmap() failed...\n");
        close(fd_USER_BUTTON);
    }

    // cleanup LED display
    unmap_physical(LW_virtual, LW_BRIDGE_SPAN);
    close_physical(fd);
}


/**
 * Takes in a string and displays it to the center of the LED Display
 */
void lcdDisplayText(char text[])
{
    printf("-------------\n");

    DRAW_Clear(&LcdCanvas, LCD_WHITE);
    DRAW_PrintString(&LcdCanvas, 40, 5, text, LCD_BLACK, &font_16x16);
    DRAW_Refresh(&LcdCanvas);
    usleep(2 * 1000000);
}


/**
 * Loop through the snake body and food then place all parts on screen
 */
void lcdDisplayUpdate(Snake *snake, Position food)
{
    DRAW_Clear(&LcdCanvas, LCD_WHITE);
    // for now just print the x and y values of each body part and food
    int i;
    for (i = 0; i < snake->used; i++)
    {
        printf("(%d,%d)\n", snake->parts[i].x, snake->parts[i].y);
        printf("-----------------------\n");
    }
    printf("food(x,y): (%d,%d)\n", food.x, food.y);
    printf("direction: (%d,%d)\n", snake->direction[0], snake->direction[1]);
    printf("*****************************\n");

    // add the border around the game
    DRAW_Rect(&LcdCanvas, 0,0, LcdCanvas.Width-1, LcdCanvas.Height-1, LCD_BLACK);

    // draw right line to extend right border
    DRAW_Line(&LcdCanvas, LcdCanvas.Width-2,1, LcdCanvas.Width-2, LcdCanvas.Height-1, LCD_BLACK);

    // draw left line to extend left border
    DRAW_Line(&LcdCanvas, 1,1, 1, LcdCanvas.Height-1, LCD_BLACK);
    DRAW_Line(&LcdCanvas, 2,1, 2, LcdCanvas.Height-1, LCD_BLACK);

    // draw top line to extend top border
    DRAW_Line(&LcdCanvas, 1,1, LcdCanvas.Width-2, 1, LCD_BLACK);
    DRAW_Line(&LcdCanvas, 1,2, LcdCanvas.Width-2, 2, LCD_BLACK);

    // paint the snake on the screen
    int v, z;

    for(i = 0; i < snake->used; i++)
    {
        v = snake->parts[i].x * 3;
        z = snake->parts[i].y * 3;

        DRAW_Pixel(&LcdCanvas, 0+v, 0+z, LCD_BLACK);
        DRAW_Pixel(&LcdCanvas, 1+v, 0+z, LCD_BLACK);
        DRAW_Pixel(&LcdCanvas, 0+v, 1+z, LCD_BLACK);
        DRAW_Pixel(&LcdCanvas, 1+v, 1+z, LCD_BLACK);
        DRAW_Pixel(&LcdCanvas, 2+v, 0+z, LCD_BLACK);
        DRAW_Pixel(&LcdCanvas, 2+v, 1+z, LCD_BLACK);
        DRAW_Pixel(&LcdCanvas, 0+v, 2+z, LCD_BLACK);
        DRAW_Pixel(&LcdCanvas, 1+v, 2+z, LCD_BLACK);
        DRAW_Pixel(&LcdCanvas, 2+v, 2+z, LCD_BLACK);
    }

    v = food.x * 3;
    z = food.y * 3;

    DRAW_Pixel(&LcdCanvas, 0+v, 0+z, LCD_BLACK);
    DRAW_Pixel(&LcdCanvas, 1+v, 0+z, LCD_BLACK);
    DRAW_Pixel(&LcdCanvas, 0+v, 1+z, LCD_BLACK);
    DRAW_Pixel(&LcdCanvas, 1+v, 1+z, LCD_BLACK);
    DRAW_Pixel(&LcdCanvas, 2+v, 0+z, LCD_BLACK);
    DRAW_Pixel(&LcdCanvas, 2+v, 1+z, LCD_BLACK);
    DRAW_Pixel(&LcdCanvas, 0+v, 2+z, LCD_BLACK);
    DRAW_Pixel(&LcdCanvas, 1+v, 2+z, LCD_BLACK);
    DRAW_Pixel(&LcdCanvas, 2+v, 2+z, LCD_BLACK);

    DRAW_Refresh(&LcdCanvas);
    usleep(.25 * 1000000);
}




/**
 * Read in an integer and use that integer to place correct numbers into each LCD display 1-1000's place
 */
void ledDisplay(int num)
{
    // set the 1000s spot
    printf("Display %d to 1000's LCD\n", num / 1000);
    num = num % 1000;
    // set the 100's spot
    printf("Display %d to 100's LCD\n", num / 100);
    num = num % 100;
    // set the 10's spot
    printf("Display %d to 10's LCD\n", num / 10);
    num = num % 10;
    // set the 1's spot
    printf("Display %d to 1's LCD\n", num / 1);

    if((fd = open_physical(fd)) == -1)
        printf("failed to initialize lcd display");
    if((LW_virtual = map_physical(fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
        printf("failed to initialize lcd display");

    JP1_ptr = (unsigned int *) (LW_virtual + JP1_BASE);

    printf("BCD 7 Segment Decoder Circuit Test\r\n");
    *(JP1_ptr + 1) = 0x0000000F;

    printf("Writing 0 thru 9 to the BCD 7 Segment Decoder circuit using GPIO[0] - [3] pins\r\n");

    *(JP1_ptr + 0) = num;
    printf("Wrote Value of %d\r\n", num);

}


/**
 * Check if button 1 has been pressed, if it has return 1, if not return 0
 */
int checkButton1()
{
    if(*KEY_ptr == 4)
        return 1;
    else
        return 0;
}


/**
 * Check if button 2 has been pressed, if it has return 1, if not return 0
 */
int checkButton2()
{
    if(*KEY_ptr == 8)
        return 1;
    else
        return 0;
}

/**
 * Map the physical address for the FPGA LED to user space
 */
int open_physical(int fd){
    if(fd==-1)
        if((fd = open("/dev/mem", (O_RDWR | O_SYNC))) == -1){
            printf("ERROR: Could not Open...");
            return (-1);
        }
    return fd;
}

/**
 * Close the physical address for the FPGA LED to user space
 */
void close_physical(int fd){
    close(fd);
}

/**
 * Map the virtual base for the LED
 */
void* map_physical(int fd, unsigned int base, unsigned int span){
    void *virtual_base;

    virtual_base = mmap(NULL, span, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, base);
    if(virtual_base == MAP_FAILED){
        printf("ERROR: mmap() failed...");
        close(fd);
        return (NULL);
    }
    return virtual_base;
}

/**
 * Unmap the virtual base for the LED
 */
int unmap_physical(void * virtual_base, unsigned int span){
    if(munmap (virtual_base, span) != 0){
        printf("ERROR: munmap failed...");
        return(-1);
    }
    return 0;
}
