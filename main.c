#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
// Function prototypes
void readFile(const char* filename);
void listBooks(const char* filename);
int verifyLogin(const char* username, const char* password, const char* filename);
void borrowBook(const char* filename, int bookToRemove, const char* username);
void returnBook(const char* username, int bookID);
void recordTransaction(const char* username, const char* book, const char* action);
char* initializeUserHistory(const char* username);
void displayMenu();

int main() {
    const char* filename = "users.txt";
    const char* file2 = "library.txt";
    char* userFile;

    int choice = 0;
    char username[50];
    char password[50];
    char book[50];

    printf("Enter Username: ");
    scanf("%s", username);
    //printf("the username was %s\n", username);
    printf("Enter Password: ");
    scanf("%s", password);
    //printf("the password was %s\n", password);
    
    choice = verifyLogin(username, password, filename);
    //printf("\nChoice was %d", choice);
    if (choice == -1) {
        choice = 0;
        userFile = initializeUserHistory(username);
    } else {
        choice = 5;
    }


    while (choice != 5) {
        //printf("the file name is '%s'", userFile);
        displayMenu();
        //reorderFile(file2);
        printf("Select an option: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                printf("The contents of the file are: \n");
                readFile(file2);
                printf("\n");
                break;
            case 2: {
                int bookID;
                printf("Enter Book to Borrow: ");
                scanf("%d", &bookID);
                borrowBook(file2, bookID, username);
                break;
            }
            case 3:
                int bookID;
                printf("Enter Book to Return: ");
                scanf("%d", &bookID);
                returnBook(username, bookID);
                break;
            case 4:
                readFile(userFile);
                break;
            case 5:
                printf("Exiting program.\n");
                return 0;
            default:
                printf("Invalid option, please try again.\n");
        }
    }

    return 0;
}

void displayMenu() {
    printf("\n1. List Books\n");
    printf("2. Borrow a Book\n");
    printf("3. Return a Book\n");
    printf("4. Show My Borrowing History\n");
    printf("5. Exit\n");
}

void readFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file for reading.\n");
    }


    char buffer[100];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }


    fclose(file);
}

int verifyLogin(const char* username, const char* password, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file for reading.\n");
        return 0;
    }

    char line[100];
    char buffer[150];
    snprintf(line, sizeof(line), "%s %s\n", username, password);
    //printf("Expected line: %s", line);
    
    char* p = line;
    while (isspace((unsigned char)*p)) p++;
    size_t len = strlen(p);
    while (len > 0 && isspace((unsigned char)p[len - 1])) len--;
    p[len] = '\0';
    
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        p = buffer;
        while (isspace((unsigned char)*p)) p++;
        len = strlen(p);
        while (len > 0 && isspace((unsigned char)p[len - 1])) len--;
        p[len] = '\0';
        if (strcmp(buffer, line) == 0) {
            fclose(file);
            printf("Login successful.\n");
            return -1;
        }
    }
    
    fclose(file);
    printf("Login failed.\n");
    return 0;   
}

void borrowBook(const char* filename, int bookToRemove, const char* username) {
    printf("Book ID is %d\n", bookToRemove);
    FILE* fr = fopen(filename, "r");
    if (fr == NULL) {
        printf("Error opening file %s for reading.\n", filename);
        return;
    }

    FILE* fw = fopen("temp.txt", "w");
    if (fw == NULL) {
        printf("Error opening temp.txt file for writing.\n");
        fclose(fr);
        return;
    }

    char buffer[300];
    int found = 0;

    while (fgets(buffer, sizeof(buffer), fr) != NULL) {
        int currentBook;
        if (sscanf(buffer, "%d.", &currentBook) == 1 && currentBook == bookToRemove) {
            found = 1;
            recordTransaction(username, buffer, "Borrowed:");
            continue;
        }
        fputs(buffer, fw);
    }

    fclose(fr);
    fclose(fw);

    if (!found) {
        printf("Book with ID %d not found in the file.\n", bookToRemove);
        remove("temp.txt");
    } else {
        printf("The Book with ID %d has been borrowed.\n", bookToRemove);
    }
    
    remove(filename);
    rename("temp.txt", filename);
}


void returnBook(const char* username, int bookID) {
    char bookIDStr[20];
    snprintf(bookIDStr, sizeof(bookIDStr), "%d", bookID);
    
    char historyFilename[100];
    snprintf(historyFilename, sizeof(historyFilename), "%s_history.txt", username);

    FILE* historyFile = fopen(historyFilename, "r");
    if (historyFile == NULL) {
        printf("Error opening user history file %s for reading.\n", historyFilename);
        return;
    }

    FILE* tempFile = fopen("temp.txt", "w");
    if (tempFile == NULL) {
        printf("Error opening temp.txt file for writing.\n");
        fclose(historyFile);
        return;
    }

    char buffer[300];
    char foundLine[300];
    int found = 0;

    while (fgets(buffer, sizeof(buffer), historyFile) != NULL) {
        if (strstr(buffer, bookIDStr) != NULL) {
            found = 1;
            char* borrowedPrefix = strstr(buffer, "Borrowed: ");
            if (borrowedPrefix != NULL) {
                strcpy(foundLine, borrowedPrefix + strlen("Borrowed: "));
            } else {
                strcpy(foundLine, buffer);
            }
        } else {
            fputs(buffer, tempFile);
        }
    }

    fclose(historyFile);
    fclose(tempFile);

    if (!found) {
        printf("Book with ID %d not found in the user's history.\n", bookID);
        remove("temp.txt");
        return;
    }
    

    FILE* libraryFile = fopen("library.txt", "a");
    if (libraryFile == NULL) {
        printf("Error opening library.txt file for appending.\n");
        return;
    }


    fprintf(libraryFile, "\n%s", foundLine);


    fclose(libraryFile);
    
    recordTransaction(username, foundLine, "Returned:");
}

char* initializeUserHistory(const char* username) {
    char filename[100];
    snprintf(filename, sizeof(filename), "%s_history.txt", username);

    FILE* historyFile = fopen(filename, "a");
    if (historyFile == NULL) {
        printf("Error opening user history file for writing.\n");
        return NULL;
    }

    fclose(historyFile);

    char* createdFilename = malloc(strlen(filename) + 1);
    strcpy(createdFilename, filename);

    return createdFilename;
}


void recordTransaction(const char* username, const char* book, const char* action) {
    char filename[100];
    snprintf(filename, sizeof(filename), "%s_history.txt", username);

    FILE* historyFile = fopen(filename, "a");
    if (historyFile == NULL) {
        printf("Error opening or creating user history file for appending.\n");
        return;
    }

    fprintf(historyFile, "%s %s", action, book);

    fclose(historyFile);
}
