#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <curses.h>
#include <locale.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define QUIT ":quit"

#define BORDER_WIDTH 1
#define PROMPT_WIDTH 1
#define TOTAL_BORDER (BORDER_WIDTH * 2)
#define INPUT_OFFSET_X 2
#define USABLE_WIDTH(cols) ((cols) - TOTAL_BORDER - PROMPT_WIDTH)
#define LAST_COLUMN_INDEX(cols) (cols - 1)
#define INNER_CONTENT_WIDTH(cols) ((cols) - TOTAL_BORDER)

#define DELETE_KEY 127
#define ENTER_KEY  '\n'

#define PRINTABLE_KEY_MIN 32
#define PRINTABLE_KEY_MAX 126

#define LLCORNER ACS_LLCORNER
#define LRCORNER ACS_LRCORNER
#define ULCORNER ACS_ULCORNER
#define URCORNER ACS_URCORNER
#define HLINE ACS_HLINE
#define VLINE ACS_VLINE

enum UI_DIMENSIONS {
    LEFT_MARGIN = 1,
    PROMPT_CHAR = '>'
};

typedef struct {
    char* buffer;
    char* end;
    size_t size;
} Buffer;

typedef struct {
    int rows;
    int cols;

    int box_top;
    int box_bottom;

    int input_top;
    int input_height;

    int cursor_x;
    int cursor_y;
} Layout;

void draw_top_bar(Layout* layout) {
    mvaddch(layout->box_top, 0, ULCORNER);
    for(int i = 0; i <= USABLE_WIDTH(layout->cols); i++) addch(HLINE);
    addch(URCORNER);
}

void draw_middle_input(Layout* layout, Buffer* input_buffer) {
    mvaddch(layout->input_top, LEFT_MARGIN, PROMPT_CHAR);

    // mvaddnstr moves a specified location and print the text there
    for (int i = 0; i < layout->input_height; i++) {
        mvaddnstr(
            layout->input_top + i, // Move the y to the line needed
            INPUT_OFFSET_X, // The text will always be 2 chars to the right
            input_buffer->buffer + i * USABLE_WIDTH(layout->cols), // print the input buffer while trimming what do don't need
            USABLE_WIDTH(layout->cols) // We print only cols - 3 chars
        );

    }
}

void draw_vertical_lines(Layout* layout) {
    for(int i = 0; i < layout->input_height; i++) {
        mvaddch(layout->input_top + i, 0, VLINE);
        mvaddch(layout->input_top + i, layout->cols - 1, VLINE);
    }
}

void draw_bottom_bar(Layout* layout) {
    mvaddch(layout->box_bottom, 0, LLCORNER);
    for(int i = 0; i <= USABLE_WIDTH(layout->cols); i++) addch(HLINE);
    addch(LRCORNER);
}

void draw_box(Layout* layout, Buffer* input_buffer) {
    // Horizontal top bar
    draw_top_bar(layout);

    // Middle input section
    draw_middle_input(layout, input_buffer);
        
    // 2 Vertical lines at sides
    draw_vertical_lines(layout);

    // Bottom bar
    draw_bottom_bar(layout);
}

int get_input_height(Buffer* input_buffer, int cols) {
    return (strlen(input_buffer->buffer)) / USABLE_WIDTH(cols) + BORDER_WIDTH;
}

int input_buffer_x(Layout* layout, Buffer* input_buffer) {
    return strlen(input_buffer->buffer) - (layout->input_height - BORDER_WIDTH) * USABLE_WIDTH(layout->cols) + INPUT_OFFSET_X;
}

void generate_layout(Layout* layout, Buffer* input_buffer) {
    getmaxyx(stdscr, layout->rows, layout->cols); // Get the rows and cols of the terminal

    layout->input_height = get_input_height(input_buffer, layout->cols);
    
    layout->box_top = layout->rows - layout->input_height - (TOTAL_BORDER);
    layout->input_top = layout->box_top + BORDER_WIDTH;
    layout->box_bottom = layout->box_top + layout->input_height + BORDER_WIDTH;

    layout->cursor_y = layout->input_top + layout->input_height -  BORDER_WIDTH;
    layout->cursor_x = input_buffer_x(layout, input_buffer);
}

int main() {
    setlocale(LC_ALL, "");
    initscr();
    keypad(stdscr, TRUE);
    cbreak();
    noecho();

    // Will store server addr
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    char buffer[BUFFER_SIZE] = {0};

    // Create client socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if(connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    Layout layout;

    char message_storage[1024];
    Buffer message_buffer = {
        .buffer = message_storage,
        .end = message_storage,
        .size = sizeof(message_storage)
    };

    clear();
    generate_layout(&layout, &message_buffer);
    draw_box(&layout, &message_buffer);
    refresh();

    while(1) {
        clear();

        generate_layout(&layout, &message_buffer);
        draw_box(&layout, &message_buffer);
        
        // Send data to the server
        //send(client_fd, message_buffer.buffer, strlen(message_buffer.buffer), 0);
        
        // Read data from the server
        //recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        //memset(buffer, 0, sizeof(buffer));

        refresh();
    }

    return 0;
}