#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 60
#define HEIGHT 20
#define MAX_OBJECTS 100

// ANSI Color Codes for enhanced terminal aesthetics
#define ANSI_COLOR_CYAN    "\033[1;36m"
#define ANSI_COLOR_YELLOW  "\033[1;33m"
#define ANSI_COLOR_GREEN   "\033[1;32m"
#define ANSI_COLOR_RED     "\033[1;31m"
#define ANSI_COLOR_RESET   "\033[0m"

typedef enum {
    OBJ_LINE,
    OBJ_RECTANGLE,
    OBJ_TRIANGLE,
    OBJ_CIRCLE
} ObjectType;

typedef struct {
    int x1, y1;
    int x2, y2;
} LineParams;

typedef struct {
    int x, y; // Top-left corner
    int width, height;
} RectParams;

typedef struct {
    int x1, y1;
    int x2, y2;
    int x3, y3;
} TriParams;

typedef struct {
    int cx, cy; // Center
    int radius;
} CircleParams;

typedef struct {
    ObjectType type;
    union {
        LineParams line;
        RectParams rect;
        TriParams tri;
        CircleParams circle;
    } params;
} Object;

// --- Helper Functions ---

// Robust integer input scanner to prevent infinite loops on invalid characters
int get_int(const char *prompt, int min_val, int max_val) {
    int val;
    char buffer[256];
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }
        if (sscanf(buffer, "%d", &val) == 1) {
            if (val >= min_val && val <= max_val) {
                return val;
            }
        }
        printf(ANSI_COLOR_RED "Invalid input. Please enter an integer between %d and %d." ANSI_COLOR_RESET "\n", min_val, max_val);
    }
}

// --- Canvas Drawing Algorithms ---

// Clear the canvas to the default underscore character
void clear_canvas(char canvas[HEIGHT][WIDTH]) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            canvas[y][x] = '_';
        }
    }
}

// Bresenham's Line Algorithm
void draw_line(char canvas[HEIGHT][WIDTH], int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        if (x1 >= 0 && x1 < WIDTH && y1 >= 0 && y1 < HEIGHT) {
            canvas[y1][x1] = '*';
        }
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Rectangle Drawing Function (outline)
void draw_rectangle(char canvas[HEIGHT][WIDTH], int x, int y, int w, int h) {
    if (w <= 0 || h <= 0) return;
    // Top and bottom edges
    for (int i = 0; i < w; i++) {
        int tx = x + i;
        int ty1 = y;
        int ty2 = y + h - 1;
        if (tx >= 0 && tx < WIDTH) {
            if (ty1 >= 0 && ty1 < HEIGHT) canvas[ty1][tx] = '*';
            if (ty2 >= 0 && ty2 < HEIGHT) canvas[ty2][tx] = '*';
        }
    }
    // Left and right edges
    for (int i = 0; i < h; i++) {
        int ty = y + i;
        int tx1 = x;
        int tx2 = x + w - 1;
        if (ty >= 0 && ty < HEIGHT) {
            if (tx1 >= 0 && tx1 < WIDTH) canvas[ty][tx1] = '*';
            if (tx2 >= 0 && tx2 < WIDTH) canvas[ty][tx2] = '*';
        }
    }
}

// Triangle Drawing Function (three lines)
void draw_triangle(char canvas[HEIGHT][WIDTH], int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line(canvas, x1, y1, x2, y2);
    draw_line(canvas, x2, y2, x3, y3);
    draw_line(canvas, x3, y3, x1, y1);
}

// Helper to plot 8 octants of a circle
void plot_circle_points(char canvas[HEIGHT][WIDTH], int cx, int cy, int x, int y) {
    int px, py;
    
    px = cx + x; py = cy + y; if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) canvas[py][px] = '*';
    px = cx - x; py = cy + y; if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) canvas[py][px] = '*';
    px = cx + x; py = cy - y; if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) canvas[py][px] = '*';
    px = cx - x; py = cy - y; if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) canvas[py][px] = '*';
    px = cx + y; py = cy + x; if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) canvas[py][px] = '*';
    px = cx - y; py = cy + x; if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) canvas[py][px] = '*';
    px = cx + y; py = cy - x; if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) canvas[py][px] = '*';
    px = cx - y; py = cy - x; if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) canvas[py][px] = '*';
}

// Midpoint Circle Algorithm
void draw_circle(char canvas[HEIGHT][WIDTH], int cx, int cy, int r) {
    if (r < 0) return;
    if (r == 0) {
        if (cx >= 0 && cx < WIDTH && cy >= 0 && cy < HEIGHT) {
            canvas[cy][cx] = '*';
        }
        return;
    }
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    plot_circle_points(canvas, cx, cy, x, y);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        plot_circle_points(canvas, cx, cy, x, y);
    }
}

// --- Object Database Rendering ---

void refresh_canvas(char canvas[HEIGHT][WIDTH], Object objects[], int object_count) {
    clear_canvas(canvas);
    for (int i = 0; i < object_count; i++) {
        Object obj = objects[i];
        switch (obj.type) {
            case OBJ_LINE:
                draw_line(canvas, obj.params.line.x1, obj.params.line.y1,
                          obj.params.line.x2, obj.params.line.y2);
                break;
            case OBJ_RECTANGLE:
                draw_rectangle(canvas, obj.params.rect.x, obj.params.rect.y,
                               obj.params.rect.width, obj.params.rect.height);
                break;
            case OBJ_TRIANGLE:
                draw_triangle(canvas, obj.params.tri.x1, obj.params.tri.y1,
                              obj.params.tri.x2, obj.params.tri.y2,
                              obj.params.tri.x3, obj.params.tri.y3);
                break;
            case OBJ_CIRCLE:
                draw_circle(canvas, obj.params.circle.cx, obj.params.circle.cy,
                            obj.params.circle.radius);
                break;
        }
    }
}

// Display the picture with a coordinate scale for better alignment
void display_canvas(char canvas[HEIGHT][WIDTH]) {
    printf("\n");
    // Column header (tens digit)
    printf("    " ANSI_COLOR_CYAN);
    for (int i = 0; i < WIDTH; i++) {
        if (i % 10 == 0) {
            printf("%d", i / 10);
        } else {
            printf(" ");
        }
    }
    printf(ANSI_COLOR_RESET "\n");

    // Column header (units digit)
    printf("    " ANSI_COLOR_CYAN);
    for (int i = 0; i < WIDTH; i++) {
        printf("%d", i % 10);
    }
    printf(ANSI_COLOR_RESET "\n");

    // Top border
    printf("   +" ANSI_COLOR_CYAN);
    for (int i = 0; i < WIDTH; i++) printf("-");
    printf(ANSI_COLOR_RESET "+\n");

    // Rows
    for (int y = 0; y < HEIGHT; y++) {
        printf(ANSI_COLOR_CYAN "%2d |" ANSI_COLOR_RESET, y);
        for (int x = 0; x < WIDTH; x++) {
            if (canvas[y][x] == '*') {
                printf(ANSI_COLOR_GREEN "%c" ANSI_COLOR_RESET, canvas[y][x]);
            } else {
                printf("%c", canvas[y][x]);
            }
        }
        printf(ANSI_COLOR_CYAN "|" ANSI_COLOR_RESET "\n");
    }

    // Bottom border
    printf("   +" ANSI_COLOR_CYAN);
    for (int i = 0; i < WIDTH; i++) printf("-");
    printf(ANSI_COLOR_RESET "+\n\n");
}

// --- Menu Handlers ---

void print_object(Object obj, int index) {
    switch (obj.type) {
        case OBJ_LINE:
            printf("[%d] Line: (%d, %d) to (%d, %d)\n", index, 
                   obj.params.line.x1, obj.params.line.y1, 
                   obj.params.line.x2, obj.params.line.y2);
            break;
        case OBJ_RECTANGLE:
            printf("[%d] Rectangle: Top-left (%d, %d), W: %d, H: %d\n", index,
                   obj.params.rect.x, obj.params.rect.y,
                   obj.params.rect.width, obj.params.rect.height);
            break;
        case OBJ_TRIANGLE:
            printf("[%d] Triangle: P1(%d, %d), P2(%d, %d), P3(%d, %d)\n", index,
                   obj.params.tri.x1, obj.params.tri.y1,
                   obj.params.tri.x2, obj.params.tri.y2,
                   obj.params.tri.x3, obj.params.tri.y3);
            break;
        case OBJ_CIRCLE:
            printf("[%d] Circle: Center (%d, %d), Radius: %d\n", index,
                   obj.params.circle.cx, obj.params.circle.cy,
                   obj.params.circle.radius);
            break;
    }
}

void add_object_menu(Object objects[], int *object_count) {
    if (*object_count >= MAX_OBJECTS) {
        printf(ANSI_COLOR_RED "Error: Maximum object limit reached (%d objects)." ANSI_COLOR_RESET "\n", MAX_OBJECTS);
        return;
    }

    printf("\n" ANSI_COLOR_YELLOW "--- Add Object ---" ANSI_COLOR_RESET "\n");
    printf("1. Line\n");
    printf("2. Rectangle\n");
    printf("3. Triangle\n");
    printf("4. Circle\n");
    printf("5. Cancel (Back to main menu)\n");
    int type_choice = get_int("Select shape type: ", 1, 5);

    if (type_choice == 5) return;

    Object obj;
    switch (type_choice) {
        case 1:
            obj.type = OBJ_LINE;
            obj.params.line.x1 = get_int("Enter point 1 X (0 to 59): ", 0, WIDTH - 1);
            obj.params.line.y1 = get_int("Enter point 1 Y (0 to 19): ", 0, HEIGHT - 1);
            obj.params.line.x2 = get_int("Enter point 2 X (0 to 59): ", 0, WIDTH - 1);
            obj.params.line.y2 = get_int("Enter point 2 Y (0 to 19): ", 0, HEIGHT - 1);
            break;
        case 2:
            obj.type = OBJ_RECTANGLE;
            obj.params.rect.x = get_int("Enter top-left X (0 to 59): ", 0, WIDTH - 1);
            obj.params.rect.y = get_int("Enter top-left Y (0 to 19): ", 0, HEIGHT - 1);
            obj.params.rect.width = get_int("Enter width (1 to 60): ", 1, WIDTH);
            obj.params.rect.height = get_int("Enter height (1 to 20): ", 1, HEIGHT);
            break;
        case 3:
            obj.type = OBJ_TRIANGLE;
            obj.params.tri.x1 = get_int("Enter point 1 X (0 to 59): ", 0, WIDTH - 1);
            obj.params.tri.y1 = get_int("Enter point 1 Y (0 to 19): ", 0, HEIGHT - 1);
            obj.params.tri.x2 = get_int("Enter point 2 X (0 to 59): ", 0, WIDTH - 1);
            obj.params.tri.y2 = get_int("Enter point 2 Y (0 to 19): ", 0, HEIGHT - 1);
            obj.params.tri.x3 = get_int("Enter point 3 X (0 to 59): ", 0, WIDTH - 1);
            obj.params.tri.y3 = get_int("Enter point 3 Y (0 to 19): ", 0, HEIGHT - 1);
            break;
        case 4:
            obj.type = OBJ_CIRCLE;
            obj.params.circle.cx = get_int("Enter center X (0 to 59): ", 0, WIDTH - 1);
            obj.params.circle.cy = get_int("Enter center Y (0 to 19): ", 0, HEIGHT - 1);
            obj.params.circle.radius = get_int("Enter radius (0 to 30): ", 0, 30);
            break;
    }

    objects[*object_count] = obj;
    (*object_count)++;
    printf(ANSI_COLOR_GREEN "Object added successfully!" ANSI_COLOR_RESET "\n");
}

void delete_object_menu(Object objects[], int *object_count) {
    if (*object_count == 0) {
        printf(ANSI_COLOR_RED "No objects on the canvas to delete." ANSI_COLOR_RESET "\n");
        return;
    }

    printf("\n" ANSI_COLOR_YELLOW "--- Delete Object ---" ANSI_COLOR_RESET "\n");
    for (int i = 0; i < *object_count; i++) {
        print_object(objects[i], i);
    }
    printf("[%d] Cancel (Back to main menu)\n", *object_count);

    int choice = get_int("Select object index to delete: ", 0, *object_count);
    if (choice == *object_count) return;

    for (int i = choice; i < *object_count - 1; i++) {
        objects[i] = objects[i + 1];
    }
    (*object_count)--;
    printf(ANSI_COLOR_GREEN "Object deleted successfully!" ANSI_COLOR_RESET "\n");
}

void modify_object_menu(Object objects[], int object_count) {
    if (object_count == 0) {
        printf(ANSI_COLOR_RED "No objects on the canvas to modify." ANSI_COLOR_RESET "\n");
        return;
    }

    printf("\n" ANSI_COLOR_YELLOW "--- Modify Object ---" ANSI_COLOR_RESET "\n");
    for (int i = 0; i < object_count; i++) {
        print_object(objects[i], i);
    }
    printf("[%d] Cancel (Back to main menu)\n", object_count);

    int choice = get_int("Select object index to modify: ", 0, object_count);
    if (choice == object_count) return;

    Object *obj = &objects[choice];
    printf("\nModifying: ");
    print_object(*obj, choice);

    switch (obj->type) {
        case OBJ_LINE:
            obj->params.line.x1 = get_int("Enter new point 1 X (0 to 59): ", 0, WIDTH - 1);
            obj->params.line.y1 = get_int("Enter new point 1 Y (0 to 19): ", 0, HEIGHT - 1);
            obj->params.line.x2 = get_int("Enter new point 2 X (0 to 59): ", 0, WIDTH - 1);
            obj->params.line.y2 = get_int("Enter new point 2 Y (0 to 19): ", 0, HEIGHT - 1);
            break;
        case OBJ_RECTANGLE:
            obj->params.rect.x = get_int("Enter new top-left X (0 to 59): ", 0, WIDTH - 1);
            obj->params.rect.y = get_int("Enter new top-left Y (0 to 19): ", 0, HEIGHT - 1);
            obj->params.rect.width = get_int("Enter new width (1 to 60): ", 1, WIDTH);
            obj->params.rect.height = get_int("Enter new height (1 to 20): ", 1, HEIGHT);
            break;
        case OBJ_TRIANGLE:
            obj->params.tri.x1 = get_int("Enter new point 1 X (0 to 59): ", 0, WIDTH - 1);
            obj->params.tri.y1 = get_int("Enter new point 1 Y (0 to 19): ", 0, HEIGHT - 1);
            obj->params.tri.x2 = get_int("Enter new point 2 X (0 to 59): ", 0, WIDTH - 1);
            obj->params.tri.y2 = get_int("Enter new point 2 Y (0 to 19): ", 0, HEIGHT - 1);
            obj->params.tri.x3 = get_int("Enter new point 3 X (0 to 59): ", 0, WIDTH - 1);
            obj->params.tri.y3 = get_int("Enter new point 3 Y (0 to 19): ", 0, HEIGHT - 1);
            break;
        case OBJ_CIRCLE:
            obj->params.circle.cx = get_int("Enter new center X (0 to 59): ", 0, WIDTH - 1);
            obj->params.circle.cy = get_int("Enter new center Y (0 to 19): ", 0, HEIGHT - 1);
            obj->params.circle.radius = get_int("Enter new radius (0 to 30): ", 0, 30);
            break;
    }
    printf(ANSI_COLOR_GREEN "Object modified successfully!" ANSI_COLOR_RESET "\n");
}

void print_header() {
    printf("\n" ANSI_COLOR_CYAN "============================================================" ANSI_COLOR_RESET "\n");
    printf(ANSI_COLOR_YELLOW "                 2D TERMINAL GRAPHICS EDITOR                 " ANSI_COLOR_RESET "\n");
    printf(ANSI_COLOR_CYAN "============================================================" ANSI_COLOR_RESET "\n");
}

int main() {
    char canvas[HEIGHT][WIDTH];
    Object objects[MAX_OBJECTS];
    int object_count = 0;

    // Initial canvas setup
    clear_canvas(canvas);

    while (1) {
        print_header();
        printf("1. Display Canvas\n");
        printf("2. Add Shape\n");
        printf("3. Delete Shape\n");
        printf("4. Modify Shape\n");
        printf("5. Clear All Shapes\n");
        printf("6. Exit\n");
        
        int choice = get_int("Enter choice (1-6): ", 1, 6);

        switch (choice) {
            case 1:
                refresh_canvas(canvas, objects, object_count);
                display_canvas(canvas);
                break;
            case 2:
                add_object_menu(objects, &object_count);
                refresh_canvas(canvas, objects, object_count);
                break;
            case 3:
                delete_object_menu(objects, &object_count);
                refresh_canvas(canvas, objects, object_count);
                break;
            case 4:
                modify_object_menu(objects, object_count);
                refresh_canvas(canvas, objects, object_count);
                break;
            case 5:
                object_count = 0;
                clear_canvas(canvas);
                printf(ANSI_COLOR_GREEN "Canvas cleared!" ANSI_COLOR_RESET "\n");
                break;
            case 6:
                printf("\nExiting editor. Goodbye!\n");
                return 0;
        }
    }
}
