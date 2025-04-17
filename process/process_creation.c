/**
 * @file process_creation.c
 * @brief Demonstration of process creation using fork()
 * 
 * This program demonstrates the creation of child and grandchild processes
 * using the fork() system call. It shows how process IDs are inherited
 * and how the process hierarchy works in Unix-like systems.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief Main function demonstrating process creation
 * 
 * Creates a child process and a grandchild process using fork().
 * Demonstrates the process hierarchy and PID inheritance.
 * 
 * @return int Exit status (0 on success)
 */
int main(void)
{
    printf("Enter of the program\n");
    printf("Before fork\n");
    
    // Create a child process
    pid_t child_process = fork();
    printf("After fork\n");

    printf("Child Process ID: %d\n", child_process);

    if (child_process < 0)
    {
        // Error handling for fork failure
        printf("Error: Fork failed\n");
    }
    else if (child_process == 0)
    {
        // Code executed by child process
        printf("Child Process\n");
        
        // Create a grandchild process
        pid_t grand_child_process = fork();
        printf("After fork of fork\n");
        printf("Grand Child Process ID: %d\n", grand_child_process);

        if (grand_child_process < 0)
        {
            // Error handling for fork failure in child
            printf("Error: Fork failed in child process\n");
        }
        else if (grand_child_process == 0)
        {
            // Code executed by grandchild process
            printf("Grand Child Process\n");
        }
        else
        {
            // Code executed by child process after creating grandchild
            printf("Child Process (after creating grandchild)\n");
        }
    }
    else
    {
        // Code executed by parent process
        printf("Parent Process\n");
    }
    
    printf("End of the program\n");
    return 0;
}