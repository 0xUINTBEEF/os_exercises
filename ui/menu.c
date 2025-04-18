/**
 * @file menu.c
 * @brief Terminal-based UI menu system for OS concepts visualization
 * 
 * This program provides a simple terminal-based UI to demonstrate
 * and visualize various operating system concepts.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>

#define MAX_OPTIONS 10
#define MAX_TITLE_LENGTH 50
#define MAX_DESC_LENGTH 200

typedef struct {
    char title[MAX_TITLE_LENGTH];
    char description[MAX_DESC_LENGTH];
    void (*function)(void);
} MenuOption;

typedef struct {
    char title[MAX_TITLE_LENGTH];
    MenuOption options[MAX_OPTIONS];
    int option_count;
} Menu;

// Function prototypes
void show_threads_menu(void);
void show_process_menu(void);
void show_sync_menu(void);
void clear_screen(void);
void print_header(const char* title);
void print_footer(void);
void wait_for_key(void);

// Global menu instances
Menu main_menu = {
    "Operating System Concepts",
    {
        {"Threads", "Thread creation, management and synchronization", show_threads_menu},
        {"Processes", "Process creation, management and IPC", show_process_menu},
        {"Synchronization", "Synchronization problems and solutions", show_sync_menu},
        {"Exit", "Exit program", NULL}
    },
    4
};

Menu threads_menu = {
    "Thread Operations",
    {
        {"Factorial Calculation", "Multi-threaded factorial calculation example", NULL},
        {"Fibonacci Calculation", "Multi-threaded fibonacci calculation example", NULL},
        {"Thread Pool", "Thread pool for task management example", NULL},
        {"Thread Priorities", "Thread priority management and scheduling example", NULL},
        {"Back", "Return to main menu", NULL}
    },
    5
};

Menu process_menu = {
    "Process Operations",
    {
        {"Process Creation", "Process creation and management example", NULL},
        {"Process Scheduling", "Process scheduling and priority management example", NULL},
        {"Zombie Process", "Zombie process management and prevention example", NULL},
        {"IPC Mechanisms", "Inter-process communication examples", NULL},
        {"Back", "Return to main menu", NULL}
    },
    5
};

Menu sync_menu = {
    "Synchronization Operations",
    {
        {"Peterson's Algorithm", "Peterson's mutual exclusion algorithm example", NULL},
        {"Monitor", "Monitor synchronization example", NULL},
        {"Dining Philosophers", "Dining philosophers problem solution", NULL},
        {"Readers-Writers", "Readers-writers problem solution", NULL},
        {"Deadlock Detection", "Deadlock detection and prevention example", NULL},
        {"Back", "Return to main menu", NULL}
    },
    6
};

void clear_screen(void) {
    system("clear || cls");
}

void print_header(const char* title) {
    printf("\n");
    printf("========================================\n");
    printf("  %s\n", title);
    printf("========================================\n\n");
}

void print_footer(void) {
    printf("\n========================================\n");
    printf("  Select an option (1-%d): ", main_menu.option_count);
}

void wait_for_key(void) {
    printf("\nPress any key to continue...");
    getchar();
    getchar(); // Clear the newline
}

void show_menu(Menu* menu) {
    int choice;
    
    do {
        clear_screen();
        print_header(menu->title);
        
        for (int i = 0; i < menu->option_count; i++) {
            printf("%d. %s\n", i + 1, menu->options[i].title);
            printf("   %s\n\n", menu->options[i].description);
        }
        
        print_footer();
        scanf("%d", &choice);
        
        if (choice > 0 && choice <= menu->option_count) {
            if (menu->options[choice - 1].function != NULL) {
                menu->options[choice - 1].function();
            } else if (strcmp(menu->options[choice - 1].title, "Back") == 0) {
                return;
            } else {
                printf("\nThis feature is not yet implemented.\n");
                wait_for_key();
            }
        }
    } while (choice != menu->option_count);
}

void show_threads_menu(void) {
    show_menu(&threads_menu);
}

void show_process_menu(void) {
    show_menu(&process_menu);
}

void show_sync_menu(void) {
    show_menu(&sync_menu);
}

int main(void) {
    show_menu(&main_menu);
    return 0;
} 