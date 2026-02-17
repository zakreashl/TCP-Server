#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <curses.h>
#include <locale.h>

#define PORT 8080
#define HISTORY_BUFFER_SIZE 8192
#define MESSAGE_BUFFER_SIZE 1024
#define RECV_BUFFER_SIZE 1024
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

#define BUFFER_OVERFLOW_MESSAGE "\n\033[31m|--- BUFFER OVERFLOW ---|\033[0m"

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

void draw_history(Layout* layout, Buffer* history_buffer) {
    mvaddstr(0, 0, history_buffer->buffer);
}

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

void draw_box(Layout* layout, Buffer* input_buffer, Buffer* history_buffer) {
    // Draw log history on top
    draw_history(layout, history_buffer);

    // Horizontal top bar
    draw_top_bar(layout);

    // Middle input section
    draw_middle_input(layout, input_buffer);
        
    // 2 Vertical lines at sides
    draw_vertical_lines(layout);

    // Bottom bar
    draw_bottom_bar(layout);
}

void process_ch(Buffer* message_buffer, int client_fd) {
    int ch = getch();

    bool new_line = ch == ENTER_KEY;

    bool printable_char = (ch >= PRINTABLE_KEY_MIN && ch <= PRINTABLE_KEY_MAX);
    bool input_buffer_overflow = (message_buffer->end >= message_buffer->buffer + message_buffer->size - 1);
        
    bool delete_key = (ch == DELETE_KEY);
    bool input_buffer_underflow = (message_buffer->end < message_buffer->buffer + 1);

    if (new_line) {
        // If it is a new line then send it to the server
        send(client_fd, message_buffer->buffer, strlen(message_buffer->buffer), 0);

        // Reset the message buffer
        message_buffer->end = message_buffer->buffer;
        *message_buffer->end = '\0';
    } else if (!input_buffer_overflow && printable_char) {
        // Add char to the input buffer
        *message_buffer->end = ch;

        // Add the null terminator
        message_buffer->end++;
        *message_buffer->end = '\0';
    } else if (delete_key && !input_buffer_underflow) {
        // Delete key
        message_buffer->end--;
        *message_buffer->end = '\0';
    }
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

void handle_recv(Buffer* history_buffer, Buffer* recv_buffer, int client_fd) {
    int bytes_received = recv(client_fd, recv_buffer->buffer, recv_buffer->size - 1, 0);
    bool buffer_overflow = (history_buffer->end + strlen(recv_buffer->buffer) > history_buffer->buffer + history_buffer->size);

    if(buffer_overflow) {
        strcpy(history_buffer->buffer + history_buffer->size - strlen(BUFFER_OVERFLOW_MESSAGE), BUFFER_OVERFLOW_MESSAGE);
        exit(EXIT_FAILURE);
    }
    
    if (bytes_received > 0) {
        recv_buffer->buffer[bytes_received] = '\0';
        strcpy(history_buffer->end, recv_buffer->buffer);
        history_buffer->end += bytes_received;
        *history_buffer->end = '\0';
        *history_buffer->end++ = '\n';

        *recv_buffer->buffer = '\0';
    }
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

    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    char recv_storage[RECV_BUFFER_SIZE];
    Buffer recv_buffer = {
        .buffer = recv_storage,
        .end = recv_storage,
        .size = sizeof(recv_storage)
    };

    char history_storage[HISTORY_BUFFER_SIZE];
    Buffer history_buffer = {
        .buffer = history_storage,
        .end = history_storage,
        .size = sizeof(history_storage)
    };
    
    char message_storage[MESSAGE_BUFFER_SIZE];
    Buffer message_buffer = {
        .buffer = message_storage,
        .end = message_storage,
        .size = sizeof(message_storage)
    };
    
    Layout layout;

    clear();
    generate_layout(&layout, &message_buffer);
    draw_box(&layout, &message_buffer, &history_buffer);
    refresh();

    while(1) {
        erase();
        generate_layout(&layout, &message_buffer);

        handle_recv(&history_buffer, &recv_buffer, client_fd);
        
        move(layout.cursor_y, layout.cursor_x);
        
        draw_box(&layout, &message_buffer, &history_buffer);
        
        process_ch(&message_buffer, client_fd);

        refresh();
    }

    return 0;
}