#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "display.h"
#include "lib/io.h"
#include "lib/memory.h"

// Keyboard scancodes
#define KEY_UP 0x48
#define KEY_DOWN 0x50
#define KEY_LEFT 0x4B
#define KEY_RIGHT 0x4D
#define KEY_ENTER 0x1C
#define KEY_ESC 0x01
#define KEY_NUM_1 0x4F
#define KEY_NUM_2 0x50
#define KEY_NUM_3 0x51
#define KEY_SPACE 0x39

// AxeniaOS Color Scheme
#define AXENIA_BACKGROUND 0x1E1E2E
#define AXENIA_TITLEBAR 0x313244
#define AXENIA_ACCENT 0x89B4FA
#define AXENIA_TEXT 0xCDD6F4
#define AXENIA_HIGHLIGHT 0x45475A
#define AXENIA_BORDER 0x89B4FA
#define AXENIA_INACTIVE 0x6C7086
#define AXENIA_ERROR 0xF38BA8
#define AXENIA_SUCCESS 0xA6E3A1

// Window dimensions - Centered
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define TITLEBAR_HEIGHT 30
#define CORNER_RADIUS 15

// Calculate centered position
#define CENTER_OFFSET_X(fb) ((fb->width - WINDOW_WIDTH) / 2)
#define CENTER_OFFSET_Y(fb) ((fb->height - WINDOW_HEIGHT) / 2)

// UI Elements
#define MENU_ITEM_HEIGHT 50
#define MENU_ITEM_PADDING 15

typedef enum {
    MENU_MAIN,
    APP_CALCULATOR,
    APP_SNAKE,
    APP_PAINT
} AppState;

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point position;
    Point direction;
    Point food;
    Point body[100];
    int length;
    bool game_over;
} Snake;

typedef struct {
    int32_t num1;    // Fixed-point (x100)
    int32_t num2;    // Fixed-point (x100)
    char operator;
    char display[32];
    bool new_number;
} Calculator;

typedef struct {
    const char* title;
    AppState state;
    const char* items[10];
    int item_count;
    int selected;
    bool active;
    Snake snake;
    Calculator calc;
} Window;

bool key_pressed(uint8_t keycode) {
    uint8_t scancode = inb(0x60); // Read from the keyboard's data port
    return scancode == keycode;
}

// Simple random number generator for bare metal
static uint32_t rand_seed = 1;

static uint32_t rand(void) {
    rand_seed = rand_seed * 1103515245 + 12345;
    return (rand_seed >> 16) & 32767;
}

static size_t strlen(const char* str) {
    const char* s = str;
    while (*s) s++;
    return s - str;
}

// Basic string concatenation
static char* strcat(char* dest, const char* src) {
    char* ptr = dest + strlen(dest);
    while (*src != '\0') {
        *ptr++ = *src++;
    }
    *ptr = '\0';
    return dest;
}

// String to float conversion (basic implementation)
static double atof(const char* str) {
    double result = 0.0;
    double fraction = 0.0;
    double power = 1.0;
    int sign = 1;
    
    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    }
    
    // Process integer part
    while (*str >= '0' && *str <= '9') {
        result = result * 10.0 + (*str - '0');
        str++;
    }
    
    // Process decimal part
    if (*str == '.') {
        str++;
        while (*str >= '0' && *str <= '9') {
            fraction = fraction * 10.0 + (*str - '0');
            power *= 10.0;
            str++;
        }
    }
    
    return sign * (result + fraction / power);
}

// Sprintf - basic implementation for numbers and strings
static int simple_sprintf(char* str, const char* num) {
    char* start = str;
    while (*num) {
        *str++ = *num++;
    }
    *str = '\0';
    return str - start;
}

// Helper function to draw a rounded corner
void draw_corner(struct limine_framebuffer *fb, int center_x, int center_y, 
                int radius, int quadrant, uint32_t color) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                int draw_x = center_x;
                int draw_y = center_y;
                
                switch (quadrant) {
                    case 1: draw_x += x; draw_y -= y; break;  // Top right
                    case 2: draw_x -= x; draw_y -= y; break;  // Top left
                    case 3: draw_x -= x; draw_y += y; break;  // Bottom left
                    case 4: draw_x += x; draw_y += y; break;  // Bottom right
                }
                
                if (draw_x >= 0 && draw_x < fb->width && 
                    draw_y >= 0 && draw_y < fb->height) {
                    ((uint32_t*)fb->address)[draw_y * (fb->pitch / 4) + draw_x] = color;
                }
            }
        }
    }
}

void draw_rounded_rectangle(struct limine_framebuffer *fb, int x, int y, 
                          int width, int height, int radius, uint32_t color) {
    // Draw main rectangle
    for (int py = y + radius; py < y + height - radius; py++) {
        for (int px = x; px < x + width; px++) {
            ((uint32_t*)fb->address)[py * (fb->pitch / 4) + px] = color;
        }
    }
    
    // Draw top and bottom rectangles
    for (int py = y; py < y + radius; py++) {
        for (int px = x + radius; px < x + width - radius; px++) {
            ((uint32_t*)fb->address)[py * (fb->pitch / 4) + px] = color;
        }
    }
    for (int py = y + height - radius; py < y + height; py++) {
        for (int px = x + radius; px < x + width - radius; px++) {
            ((uint32_t*)fb->address)[py * (fb->pitch / 4) + px] = color;
        }
    }
    
    // Draw rounded corners
    draw_corner(fb, x + radius, y + radius, radius, 2, color);
    draw_corner(fb, x + width - radius, y + radius, radius, 1, color);
    draw_corner(fb, x + radius, y + height - radius, radius, 3, color);
    draw_corner(fb, x + width - radius, y + height - radius, radius, 4, color);
}

void init_snake(Snake *snake) {
    snake->position.x = WINDOW_WIDTH / 2;
    snake->position.y = WINDOW_HEIGHT / 2;
    snake->direction.x = 1;
    snake->direction.y = 0;
    snake->length = 1;
    snake->game_over = false;
    snake->food.x = (WINDOW_WIDTH / 4) * 3;
    snake->food.y = WINDOW_HEIGHT / 2;
    snake->body[0] = snake->position;
}

void update_snake(Window *window, struct limine_framebuffer *fb) {
    Snake *snake = &window->snake;
    
    // Update position
    snake->position.x += snake->direction.x * 10;
    snake->position.y += snake->direction.y * 10;
    
    // Check collisions with walls
    if (snake->position.x < CENTER_OFFSET_X(fb) + 20 || 
        snake->position.x >= CENTER_OFFSET_X(fb) + WINDOW_WIDTH - 20 ||
        snake->position.y < CENTER_OFFSET_Y(fb) + TITLEBAR_HEIGHT + 20 || 
        snake->position.y >= CENTER_OFFSET_Y(fb) + WINDOW_HEIGHT - 20) {
        snake->game_over = true;
    }
    
    // Update body
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i-1];
    }
    snake->body[0] = snake->position;
    
    // Check food collision
    if (snake->position.x >= snake->food.x - 5 && 
        snake->position.x <= snake->food.x + 5 &&
        snake->position.y >= snake->food.y - 5 && 
        snake->position.y <= snake->food.y + 5) {
        snake->length++;
        // New food position
        snake->food.x = CENTER_OFFSET_X(fb) + 20 + 
                       (rand() % (WINDOW_WIDTH - 40));
        snake->food.y = CENTER_OFFSET_Y(fb) + TITLEBAR_HEIGHT + 20 + 
                       (rand() % (WINDOW_HEIGHT - TITLEBAR_HEIGHT - 40));
    }
}

void draw_snake(struct limine_framebuffer *fb, Snake *snake) {
    // Draw food
    draw_rounded_rectangle(fb, snake->food.x - 5, snake->food.y - 5, 
                         10, 10, 2, AXENIA_ERROR);
    
    // Draw snake
    for (int i = 0; i < snake->length; i++) {
        draw_rounded_rectangle(fb, snake->body[i].x - 5, snake->body[i].y - 5, 
                             10, 10, 2, AXENIA_SUCCESS);
    }
}

static void init_calculator(Calculator* calc) {
    calc->num1 = 0;
    calc->num2 = 0;
    calc->operator = 0;
    calc->new_number = true;
    simple_sprintf(calc->display, "0.00");
}

static void fixed_to_str(int32_t num, char* str) {
    // Handle negative numbers
    if (num < 0) {
        *str++ = '-';
        num = -num;
    }
    
    // Extract whole and decimal parts
    int32_t whole = num / 100;
    int32_t decimal = num % 100;
    
    // Convert whole part
    char temp[32];
    int i = 0;
    
    if (whole == 0) {
        *str++ = '0';
    } else {
        while (whole > 0) {
            temp[i++] = '0' + (whole % 10);
            whole /= 10;
        }
        while (i > 0) {
            *str++ = temp[--i];
        }
    }
    
    // Add decimal point and decimal part
    *str++ = '.';
    if (decimal < 10) *str++ = '0';
    
    // Convert decimal part
    i = 0;
    if (decimal == 0) {
        *str++ = '0';
        *str++ = '0';
    } else {
        while (decimal > 0) {
            temp[i++] = '0' + (decimal % 10);
            decimal /= 10;
        }
        while (i > 0) {
            *str++ = temp[--i];
        }
    }
    
    *str = '\0';
}

static int32_t str_to_fixed(const char* str) {
    int32_t result = 0;
    int32_t sign = 1;
    int32_t decimal_places = 0;
    bool after_decimal = false;
    
    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    }
    
    // Process digits
    while (*str) {
        if (*str == '.') {
            after_decimal = true;
        } else if (*str >= '0' && *str <= '9') {
            if (!after_decimal) {
                result = result * 10 + (*str - '0');
            } else {
                if (decimal_places < 2) {
                    result = result * 10 + (*str - '0');
                    decimal_places++;
                }
            }
        }
        str++;
    }
    
    // Adjust for decimal places
    while (decimal_places < 2) {
        result *= 10;
        decimal_places++;
    }
    
    return result * sign;
}

static void update_calculator(Calculator* calc, char input) {
    if (input >= '0' && input <= '9') {
        if (calc->new_number) {
            calc->display[0] = input;
            calc->display[1] = '\0';
            calc->new_number = false;
        } else {
            strcat(calc->display, &input);
        }
    } else if (input == '+' || input == '-' || input == '*' || input == '/') {
        calc->num1 = str_to_fixed(calc->display);
        calc->operator = input;
        calc->new_number = true;
    } else if (input == '=') {
        calc->num2 = str_to_fixed(calc->display);
        int32_t result = 0;
        
        switch (calc->operator) {
            case '+': result = calc->num1 + calc->num2; break;
            case '-': result = calc->num1 - calc->num2; break;
            case '*': result = (calc->num1 * calc->num2) / 100; break; // Adjust for fixed-point
            case '/': result = calc->num2 != 0 ? (calc->num1 * 100) / calc->num2 : 0; break;
        }
        
        fixed_to_str(result, calc->display);
        calc->new_number = true;
    }
}


void draw_calculator(struct limine_framebuffer *fb, Window *window) {
    int base_x = CENTER_OFFSET_X(fb) + 50;
    int base_y = CENTER_OFFSET_Y(fb) + TITLEBAR_HEIGHT + 50;
    
    // Draw display
    draw_rounded_rectangle(fb, base_x, base_y, 
                         WINDOW_WIDTH - 100, 40, 5, AXENIA_HIGHLIGHT);
    draw_string(fb, window->calc.display, base_x + 10, base_y + 10, AXENIA_TEXT);
    
    // Draw buttons
    const char* buttons[] = {
        "7", "8", "9", "+",
        "4", "5", "6", "-",
        "1", "2", "3", "*",
        "0", ".", "=", "/"
    };
    
    for (int i = 0; i < 16; i++) {
        int x = base_x + (i % 4) * 70;
        int y = base_y + 60 + (i / 4) * 70;
        draw_rounded_rectangle(fb, x, y, 60, 60, 5, AXENIA_ACCENT);
        draw_string(fb, buttons[i], x + 25, y + 20, AXENIA_TEXT);
    }
}

void handle_app_input(Window *window, uint8_t key) {
    switch (window->state) {
        case APP_SNAKE:
            switch (key) {
                case KEY_UP:
                    if (window->snake.direction.y != 1) {
                        window->snake.direction.x = 0;
                        window->snake.direction.y = -1;
                    }
                    break;
                case KEY_DOWN:
                    if (window->snake.direction.y != -1) {
                        window->snake.direction.x = 0;
                        window->snake.direction.y = 1;
                    }
                    break;
                case KEY_LEFT:
                    if (window->snake.direction.x != 1) {
                        window->snake.direction.x = -1;
                        window->snake.direction.y = 0;
                    }
                    break;
                case KEY_RIGHT:
                    if (window->snake.direction.x != -1) {
                        window->snake.direction.x = 1;
                        window->snake.direction.y = 0;
                    }
                    break;
            }
            break;
            
        case APP_CALCULATOR:
            // Map keyboard input to calculator input
            if (key >= KEY_NUM_1 && key <= KEY_NUM_3) {
                update_calculator(&window->calc, '1' + (key - KEY_NUM_1));
            }
            // Add more calculator input handling here
            break;
    }
}

void draw_window(struct limine_framebuffer *fb, Window *window) {
    int x = CENTER_OFFSET_X(fb);
    int y = CENTER_OFFSET_Y(fb);
    
    // Draw main window
    draw_rounded_rectangle(fb, x, y, WINDOW_WIDTH, WINDOW_HEIGHT, 
                         CORNER_RADIUS, AXENIA_BACKGROUND);
    
    // Draw titlebar
    draw_rounded_rectangle(fb, x, y, WINDOW_WIDTH, TITLEBAR_HEIGHT, 
                         CORNER_RADIUS, AXENIA_TITLEBAR);
    
    // Draw window title
    draw_string(fb, window->title, x + 10, y + 5, AXENIA_TEXT);
    
    // Draw window controls
    draw_rounded_rectangle(fb, x + WINDOW_WIDTH - 90, y + 5, 
                         20, 20, 5, AXENIA_ERROR);
    draw_string(fb, "X", x + WINDOW_WIDTH - 85, y + 7, AXENIA_TEXT);
    
    // Draw content based on current state
    switch (window->state) {
        case MENU_MAIN:
            for (int i = 0; i < window->item_count; i++) {
                int item_y = y + TITLEBAR_HEIGHT + (i * MENU_ITEM_HEIGHT) + 
                           MENU_ITEM_PADDING;
                
                if (i == window->selected) {
                    draw_rounded_rectangle(fb, x + MENU_ITEM_PADDING, 
                                       item_y - 5, 
                                       WINDOW_WIDTH - (MENU_ITEM_PADDING * 2), 
                                       MENU_ITEM_HEIGHT - 10, 
                                       10, AXENIA_HIGHLIGHT);
                }
                
                draw_string(fb, window->items[i], 
                          x + MENU_ITEM_PADDING * 2, 
                          item_y + 5, 
                          i == window->selected ? AXENIA_TEXT : AXENIA_INACTIVE);
            }
            break;
            
        case APP_SNAKE:
            draw_snake(fb, &window->snake);
            if (window->snake.game_over) {
                draw_string(fb, "Game Over! Press ESC to return", 
                          x + WINDOW_WIDTH/2 - 100, 
                          y + WINDOW_HEIGHT/2, AXENIA_ERROR);
            }
            break;
            
        case APP_CALCULATOR:
            draw_calculator(fb, window);
            break;
            
        case APP_PAINT:
            // Basic paint interface
            draw_string(fb, "Paint App (Coming Soon)", 
                      x + WINDOW_WIDTH/2 - 80, 
                      y + WINDOW_HEIGHT/2, AXENIA_TEXT);
            break;
    }
}

void display_axenia_menu(struct limine_framebuffer *fb) {
    Window window = {
        .title = "AxeniaOS",
        .state = MENU_MAIN,
        .items = {
            "Calculator",
            "Snake Game",
            "Paint",
            "System Settings",
            "About AxeniaOS"
        },
        .item_count = 5,
        .selected = 0,
        .active = true
    };

    init_snake(&window.snake);
    init_calculator(&window.calc);

    // Main event loop
    while (window.active) {
        // Clear screen with gradient background
        for (uint32_t y = 0; y < fb->height; y++) {
            uint32_t gradient = 0x1E1E2E + (y * 0x000101);
            for (uint32_t x = 0; x < fb->width; x++) {
                ((uint32_t*)fb->address)[y * (fb->pitch / 4) + x] = gradient;
            }
        }

        // Draw window and handle current state
        draw_window(fb, &window);

        // Handle keyboard input
        if (key_pressed(KEY_ESC)) {
            if (window.state != MENU_MAIN) {
                window.state = MENU_MAIN;
            } else {
                // Show exit confirmation
                draw_rounded_rectangle(fb, 
                    CENTER_OFFSET_X(fb) + WINDOW_WIDTH/2 - 150,
                    CENTER_OFFSET_Y(fb) + WINDOW_HEIGHT/2 - 50,
                    300, 100, 10, AXENIA_HIGHLIGHT);
                draw_string(fb, "Exit AxeniaOS?", 
                    CENTER_OFFSET_X(fb) + WINDOW_WIDTH/2 - 60,
                    CENTER_OFFSET_Y(fb) + WINDOW_HEIGHT/2 - 30,
                    AXENIA_TEXT);
                draw_string(fb, "Press Enter to confirm", 
                    CENTER_OFFSET_X(fb) + WINDOW_WIDTH/2 - 100,
                    CENTER_OFFSET_Y(fb) + WINDOW_HEIGHT/2,
                    AXENIA_INACTIVE);
                
                // Wait for confirmation
                bool waiting = true;
                while (waiting) {
                    if (key_pressed(KEY_ENTER)) {
                        window.active = false;
                        waiting = false;
                    } else if (key_pressed(KEY_ESC)) {
                        waiting = false;
                    }
                }
            }
        }

        switch (window.state) {
            case MENU_MAIN:
                if (key_pressed(KEY_UP)) {
                    window.selected = (window.selected > 0) ? 
                        window.selected - 1 : window.item_count - 1;
                    for (volatile int i = 0; i < 100000; i++);
                }
                
                if (key_pressed(KEY_DOWN)) {
                    window.selected = (window.selected + 1) % window.item_count;
                    for (volatile int i = 0; i < 100000; i++);
                }
                
                if (key_pressed(KEY_ENTER)) {
                    switch (window.selected) {
                        case 0: // Calculator
                            window.state = APP_CALCULATOR;
                            init_calculator(&window.calc);
                            break;
                        case 1: // Snake
                            window.state = APP_SNAKE;
                            init_snake(&window.snake);
                            break;
                        case 2: // Paint
                            window.state = APP_PAINT;
                            break;
                        case 3: // Settings
                            // Show settings dialog
                            draw_rounded_rectangle(fb,
                                CENTER_OFFSET_X(fb) + 100,
                                CENTER_OFFSET_Y(fb) + 100,
                                WINDOW_WIDTH - 200,
                                WINDOW_HEIGHT - 200,
                                10, AXENIA_HIGHLIGHT);
                            draw_string(fb, "System Settings",
                                CENTER_OFFSET_X(fb) + 120,
                                CENTER_OFFSET_Y(fb) + 120,
                                AXENIA_TEXT);
                            for (volatile int i = 0; i < 1000000; i++);
                            break;
                        case 4: // About
                            // Show about dialog
                            draw_rounded_rectangle(fb,
                                CENTER_OFFSET_X(fb) + 100,
                                CENTER_OFFSET_Y(fb) + 100,
                                WINDOW_WIDTH - 200,
                                WINDOW_HEIGHT - 200,
                                10, AXENIA_HIGHLIGHT);
                            draw_string(fb, "AxeniaOS v1.0",
                                CENTER_OFFSET_X(fb) + 120,
                                CENTER_OFFSET_Y(fb) + 120,
                                AXENIA_TEXT);
                            draw_string(fb, "A Modern Lightweight Operating System",
                                CENTER_OFFSET_X(fb) + 120,
                                CENTER_OFFSET_Y(fb) + 150,
                                AXENIA_INACTIVE);
                            for (volatile int i = 0; i < 1000000; i++);
                            break;
                    }
                }
                break;

            case APP_SNAKE:
                if (!window.snake.game_over) {
                    update_snake(&window, fb);
                }
                handle_app_input(&window, key_pressed(0));
                break;

            case APP_CALCULATOR:
                // Handle calculator input
                for (uint8_t key = 0; key < 128; key++) {
                    if (key_pressed(key)) {
                        handle_app_input(&window, key);
                    }
                }
                break;

            case APP_PAINT:
                // Basic paint input handling
                if (key_pressed(KEY_SPACE)) {
                    // Clear canvas
                    draw_rounded_rectangle(fb,
                        CENTER_OFFSET_X(fb) + 50,
                        CENTER_OFFSET_Y(fb) + TITLEBAR_HEIGHT + 50,
                        WINDOW_WIDTH - 100,
                        WINDOW_HEIGHT - TITLEBAR_HEIGHT - 100,
                        5, AXENIA_BACKGROUND);
                }
                break;
        }

        // Frame delay
        for (volatile int i = 0; i < 50000; i++);
    }

    // Show exit animation
    for (int i = 0; i < 256; i += 2) {
        for (uint32_t y = 0; y < fb->height; y++) {
            for (uint32_t x = 0; x < fb->width; x++) {
                ((uint32_t*)fb->address)[y * (fb->pitch / 4) + x] = 
                    AXENIA_BACKGROUND + (i << 16) + (i << 8) + i;
            }
        }
        for (volatile int j = 0; j < 100000; j++);
    }
}

void display_init(struct limine_framebuffer *fb) {
    // Welcome animation
    for (int i = 0; i < fb->height; i += 2) {
        draw_rounded_rectangle(fb, 0, 0, fb->width, i, 0, AXENIA_BACKGROUND);
        draw_string(fb, "Welcome to AxeniaOS", 
            fb->width/2 - 100, fb->height/2 - 10, AXENIA_TEXT);
        for (volatile int j = 0; j < 10000; j++);
    }

    // Loading animation
    const char *loading = "Loading";
    const char *dots = "...";
    for (int i = 0; i < 3; i++) {
        draw_string(fb, loading, fb->width/2 - 50, fb->height/2 + 20, AXENIA_TEXT);
        for (int j = 0; j <= i; j++) {
            draw_string(fb, &dots[j], 
                fb->width/2 + 10 + (j * 10), 
                fb->height/2 + 20, AXENIA_TEXT);
        }
        for (volatile int k = 0; k < 500000; k++);
    }

    // Launch main menu
    display_axenia_menu(fb);
}