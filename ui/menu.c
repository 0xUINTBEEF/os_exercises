/**
 * Terminal-based UI Menu System
 * 
 * This program provides a simple terminal-based UI to demonstrate
 * and visualize various operating system concepts. It includes
 * menus for threads, processes, and synchronization operations.
 * 
 * Features:
 * - Hierarchical menu system
 * - Clear screen management
 * - Input validation
 * - Error handling
 * - Graceful exit
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <errno.h>
#include <signal.h>

// Constants
#define MAX_OPTIONS 10
#define MAX_TITLE_LENGTH 50
#define MAX_DESC_LENGTH 200
#define MAX_INPUT_LENGTH 10

// Menu option structure
typedef struct {
    char title[MAX_TITLE_LENGTH];
    char description[MAX_DESC_LENGTH];
    void (*function)(void);
} MenuOption;

// Menu structure
typedef struct {
    char title[MAX_TITLE_LENGTH];
    MenuOption options[MAX_OPTIONS];
    int option_count;
} Menu;

// Function prototypes
static void show_threads_menu(void);
static void show_process_menu(void);
static void show_sync_menu(void);
static void clear_screen(void);
static void print_header(const char* title);
static void print_footer(void);
static void wait_for_key(void);
static void handle_signal(int sig);
static int get_valid_input(int max_choice);
static void show_menu(Menu* menu);

// Global variables
static volatile sig_atomic_t running = 1;

// Menu instances
static Menu main_menu = {
    "Operating System Concepts",
    {
        {"Threads", "Thread creation, management and synchronization", show_threads_menu},
        {"Processes", "Process creation, management and IPC", show_process_menu},
        {"Synchronization", "Synchronization problems and solutions", show_sync_menu},
        {"Exit", "Exit program", NULL}
    },
    4
};

static Menu threads_menu = {
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

static Menu process_menu = {
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

static Menu sync_menu = {
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

// Signal handler for graceful shutdown
static void handle_signal(int sig) {
    (void)sig;
    running = 0;
}

// Clear the terminal screen
static void clear_screen(void) {
    if (system("clear || cls") == -1) {
        perror("Failed to clear screen");
    }
}

// Print menu header
static void print_header(const char* title) {
    printf("\n");
    printf("========================================\n");
    printf("  %s\n", title);
    printf("========================================\n\n");
}

// Print menu footer
static void print_footer(void) {
    printf("\n========================================\n");
    printf("  Select an option (1-%d): ", main_menu.option_count);
}

// Wait for user input
static void wait_for_key(void) {
    printf("\nPress any key to continue...");
    getchar();
    getchar(); // Clear the newline
}

// Get valid user input
static int get_valid_input(int max_choice) {
    char input[MAX_INPUT_LENGTH];
    int choice;
    
    while (1) {
        if (fgets(input, sizeof(input), stdin) == NULL) {
            if (feof(stdin)) {
                clearerr(stdin);
                continue;
            }
            perror("Failed to read input");
            return -1;
        }
        
        if (sscanf(input, "%d", &choice) != 1) {
            printf("Invalid input. Please enter a number: ");
            continue;
        }
        
        if (choice < 1 || choice > max_choice) {
            printf("Invalid choice. Please enter a number between 1 and %d: ", max_choice);
            continue;
        }
        
        return choice;
    }
}

// Show menu and handle user input
static void show_menu(Menu* menu) {
    int choice;
    
    do {
        clear_screen();
        print_header(menu->title);
        
        for (int i = 0; i < menu->option_count; i++) {
            printf("%d. %s\n", i + 1, menu->options[i].title);
            printf("   %s\n\n", menu->options[i].description);
        }
        
        print_footer();
        choice = get_valid_input(menu->option_count);
        
        if (choice == -1) {
            break;
        }
        
        if (choice > 0 && choice <= menu->option_count) {
            if (menu->options[choice - 1].function != NULL) {
                menu->options[choice - 1].function();
            } else if (strcmp(menu->options[choice - 1].title, "Back") == 0) {
                return;
            } else if (strcmp(menu->options[choice - 1].title, "Exit") == 0) {
                running = 0;
                return;
            } else {
                printf("\nThis feature is not yet implemented.\n");
                wait_for_key();
            }
        }
    } while (running && choice != menu->option_count);
}

// Show threads menu
static void show_threads_menu(void) {
    show_menu(&threads_menu);
}

// Show process menu
static void show_process_menu(void) {
    show_menu(&process_menu);
}

// Show synchronization menu
static void show_sync_menu(void) {
    show_menu(&sync_menu);
}

int main(void) {
    // Set up signal handlers
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    
    // Show main menu
    show_menu(&main_menu);
    
    // Clean up ncurses
    endwin();
    
    return EXIT_SUCCESS;
} 