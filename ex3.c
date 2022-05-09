#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>


#define LEN 510
#define READ 0
#define WRITE 1

//New functions from Ex3
int howManyPipe(char *);
void commandWithPipe(char *, int, int);
char *splitCommand(char *, int, int);
int indexPipe(char *);
void commandWithoutPipe(char *);
int indexPipe2(char *);
void commandWithTwoPipe(char *, int, int, int);
bool examinationOfTheCommand(char *);
bool ifTwoPipesTogether(char *);
char *clearCommand(char *);
void freeMemory(char **);

//Ex2
void printUserAndCurrentDir();
void printAllData(int, int);
void exeCommand(char *);

//Ex1.
int howManyWords(char *);
int howManyLetters(char *);

//In the main We will receive the string and divide them according to the pipe that exists in the string and
// send them to methods built according to cases.
//Of course we will save the data so we can print at the end (number of commands, number of pipes).
//And we also check the integrity of the string that was inserted.
int main() {
    char string[LEN];
    int numOfCommands = 0;
    int numOfPipe = 0;
    int pipe;
    int sizeCommand;
    int pipeFirstIndex;
    int pipeSecondIndex;
    while (1) {
        printUserAndCurrentDir();
        fgets(string, LEN, stdin);
        if (howManyWords(string) == 0 || examinationOfTheCommand(string) == false) {
            printf("Invalid command...\nPlease try again\n");
            continue;
        } else {
            clearCommand(string);
            numOfCommands++;
            pipe = howManyPipe(string);
            sizeCommand = howManyLetters(string);
            pipeFirstIndex = indexPipe(string);
            numOfPipe += pipe;
        }
        if (strcmp(string, "done\n") == 0) {
            printAllData(numOfCommands, numOfPipe);
            exit(EXIT_SUCCESS);
        }
        if (ifTwoPipesTogether(string) == true) {
            pipe = 0;
        }
        switch (pipe) {
            case 0:
                if (ifTwoPipesTogether(string) == true) {
                    char *newString = splitCommand(string, indexPipe(string), 0);
                    commandWithoutPipe(newString);
                    break;
                }
                commandWithoutPipe(string);
                break;
            case 1:
                commandWithPipe(string, sizeCommand, pipeFirstIndex);
                break;
            case 2:
                pipeSecondIndex = indexPipe2(string);
                commandWithTwoPipe(string, sizeCommand, pipeFirstIndex, pipeSecondIndex);
                break;
            default:
                printf("In the program it does not support more than a double pipe\n");
                break;
        }
    }
}


//Ex3 Functions.
//This method will run on the command without Pipe, Here a process is produced but "Ex2"  is used.
void commandWithoutPipe(char *command) {
    //create fork
    pid_t son;
    son = fork();
    //Check if the process creation fails if it is not sent to the Exercise 2 method that builds the array corresponding to "execvp".
    switch (son) {
        case -1:
            printf("Error creating process (Fork)\n");
            exit(EXIT_FAILURE);
        case 0:
            exeCommand(command);
            exit(EXIT_SUCCESS);
            //The father process will wait for the son process to end.
        default:
            wait(NULL);
    }
}

//This method will run on the command with Pipe.
void commandWithPipe(char *command, int size, int indexPipe) {
    //Creating a pipe and we will check if we were able to create the pipe.
    int pipe_fp[2];
    if (pipe(pipe_fp) == -1) {
        perror("Failed to create pipe\n");
        exit(EXIT_FAILURE);
    }
    //After we have all the commands together, we will now split it by a method we built.
    //We will not enter the second command yet as long as we do not know that there is one pipe because,
    //if there are more than 1 variable indexes.
    char *commandLeft = splitCommand(command, indexPipe, 0);
    char *commandRight;
    if (howManyPipe(command) == 1) {
        commandRight = splitCommand(command, size - indexPipe - 1, indexPipe + 1);
    } else if (howManyPipe(command) == 2) {
        commandRight = splitCommand(command, indexPipe2(command) - indexPipe - 1, indexPipe + 1);
    }
    //Create a son process to run a command left, inside the "switch" the parent will open a son process to run the command on the right.
    pid_t rightSon;
    pid_t leftSon = fork();
    //Construction of all cases of the process.
    switch (leftSon) {
        //We will run with the two processes together, first we will navigate the pipe and then we will open or close the ports of the pipe,
        // each time one person will perform an action.
        // (it is not possible to write and register at the same time).
        case -1:
            printf("Error creating process (Fork)\n");
            exit(EXIT_FAILURE);
        case 0:
            dup2(pipe_fp[WRITE], STDOUT_FILENO);
            close(pipe_fp[READ]);
            exeCommand(commandLeft);
            exit(EXIT_SUCCESS);
        default:
            rightSon = fork();
    }
    switch (rightSon) {
        case -1:
            printf("Error creating process (Fork)\n");
            exit(EXIT_FAILURE);
        case 0:
            dup2(pipe_fp[READ], STDIN_FILENO);
            close(pipe_fp[WRITE]);
            exeCommand(commandRight);
            exit(EXIT_SUCCESS);
        default:
            close(pipe_fp[WRITE]);
            close(pipe_fp[READ]);
            wait(NULL);
            wait(NULL);
            break;
    }
    free(commandRight);
    free(commandLeft);
}

//Division of commands into only one command.
//Because we come to this method when we know that there is at least one tube so after each command we will end it by inserting a "\n".
char *splitCommand(char *str, int size, int initialIndex) {
    char *command;
    command = (char *) malloc((size) * sizeof(char));
    if (command == NULL) {
        printf("Error!!\nType of problem: Allocation of memory");
        exit(EXIT_FAILURE);
    }
    strncpy(command, &str[initialIndex], size);
    command[size] = '\n';
    return command;
}

//Counting pipes to know how many commands we have and also to return at the end of the plan how many pipes were in our program.
int howManyPipe(char *command) {
    int counter = 0;
    for (int i = 0; i < strlen(command); i++) {
        if (command[i] == '|') {
            counter++;
        }
    }
    return counter;
}

//Finding a pipe index so we can divide the commands so we will build a method that will find us the location of the pipe.
int indexPipe(char *command) {
    int pipeIndex = 0;
    for (int i = 0; i < strlen(command); i++) {
        if (command[i] == '|') {
            pipeIndex = i;
            break;
        }
    }
    return pipeIndex;
}

//Find the index the second pipe, To find the three commands.
int indexPipe2(char *command) {
    int pipeIndex2 = indexPipe(command) + 1;
    for (int i = pipeIndex2; i < strlen(command); i++) {
        if (command[i] == '|') {
            pipeIndex2 = i;
            break;
        }
    }
    return pipeIndex2;
}

// The method will work in only two pipes.
// We will use the method that executes the pipe for us in two commands,
// but in this method we will execute the last command on the result that came out for us in the method of pipe only,
// i.e. we will execute in two commands,
// Then then we will execute two commands with the third.
void commandWithTwoPipe(char *command, int size, int indexPipe1, int indexPipe2) {
    char *newCommand = splitCommand(command, size - indexPipe2 - 1, indexPipe2 + 1);

    int pipe_fp[2];
    if (pipe(pipe_fp) == -1) {
        perror("Failed to create pipe\n");
        exit(EXIT_FAILURE);
    }
    pid_t rightSon;
    pid_t son = fork();
    switch (son) {
        case -1:
            printf("Error creating process (Fork)\n");
            exit(EXIT_FAILURE);
        case 0:
            dup2(pipe_fp[WRITE], STDOUT_FILENO);
            close(pipe_fp[READ]);
            commandWithPipe(command, size, indexPipe1);
            exit(EXIT_SUCCESS);
        default:
            rightSon = fork();
    }
    switch (rightSon) {
        case -1:
            printf("Error creating process (Fork)\n");
            exit(EXIT_FAILURE);
        case 0:
            dup2(pipe_fp[READ], STDIN_FILENO);
            close(pipe_fp[WRITE]);
            exeCommand(newCommand);
            exit(EXIT_SUCCESS);
        default:
            close(pipe_fp[WRITE]);
            close(pipe_fp[READ]);
            wait(NULL);
            wait(NULL);
            break;
    }
    free(newCommand);
}

//Tests!

//We will check that the input we received does not contain an invalid command, for example a number or points, etc.
bool examinationOfTheCommand(char *command) {
    for (int i = 0; i < strlen(command); i++) {
        if (command[i] >= '2' && command[i] <= '9' && command[i] != '0') {
            return false;
        }
        if ((command[i] >= 'a' && command[i] <= 'z')) {
            if (command[i] == command[i + 1]) {
                return false;
            } else {
                continue;
            }
        }
        if ((command[i] >= 'A' && command[i] <= 'Z')) {
            if (command[i] == command[i + 1]) {
                return false;
            } else {
                continue;
            }
        }
        if (command[i] == '!' || command[i] == '@' || command[i] == '#' || command[i] == '%' || command[i] == '$')
            return false;
        if (command[i] == '&' || command[i] == '*' || command[i] == '(' || command[i] == '!' || command[i] == '.' ||
            command[i] == ',') {
            return false;
        }
    }
    return true;
}

//Check if we have two pipes together, if we have reached such a situation we will run the more left command only.
bool ifTwoPipesTogether(char *command) {
    for (int i = 0; i < strlen(command); i++) {
        if (command[i] == '|' && command[i + 1] == '|') {
            return true;
        }
    }
    return false;
}

//Adjust the input in case of need If there are spaces then we will advance the next index
//and delete the space but be careful that there are commands with spaces for example: "ls -l".
char *clearCommand(char *str) {
    int i, j;
    int size = strlen(str);
    for (i = 0; i < size; i++) {
        if (str[i] == ' ' && str[i + 1] == '-') {
            continue;
        }
        if (str[i] == ' ') {
            for (j = i; j < size; j++) {
                str[j] = str[j + 1];
            }
            size--;
        }
    }
    return str;
}

//free
void freeMemory(char **str) {
    int i=0;
    while(str[i]) {
        free(str[i]);
        i++;
    }
    free(str);
}

//Function from Ex2.
void printUserAndCurrentDir() {
    struct passwd *pw;
    pw = getpwuid(getuid());
    if (pw == NULL) {
        printf("Error getting information from user\n");
        exit(1);
    }
    char *buf=(char*)malloc(100 * sizeof(char));
    getcwd(buf, 100);
    buf[0]='@';
    printf("%s%s>", pw->pw_name,buf);
    free(buf);
}

void printAllData(int counterCommands, int counterPipe) {
    printf("Number of commands: %d\nNumber of pipes: %d\n", counterCommands - 1, counterPipe);
    printf("See you next time !\n");

}

void exeCommand(char *str) {
    if (strcmp(str, "cd\n") == 0) {
        printf("command not supported (Yet)\n");
        return;
    }
    int i;
    int counterWord = howManyWords(str);
    int index = 0;
    int counterOfLetters = 0;
    char **command;
    command = (char **) malloc((counterWord + 1) * sizeof(char *));
    if (command == NULL) {
        printf("Error!!\nType of problem: Allocation of memory");
        exit(EXIT_FAILURE);
    }
    command[counterWord] = NULL;

    for (i = 0; i < strlen(str); i++) {
        if (str[i] != '\n' && str[i] != '\0' && str[i] != ' ') {
            counterOfLetters++;
        }
        if (str[i] == ' ' || str[i] == '\n') {
            if (counterOfLetters > 0) {
                command[index] = (char *) malloc(counterOfLetters * sizeof(char));
                if (command[index] == NULL) {
                    printf("Error!!\nType of problem: Allocation of memory");
                    exit(EXIT_FAILURE);
                }
                strncpy(command[index], &str[i - counterOfLetters], counterOfLetters);
                counterOfLetters = 0;
                index++;
            }
        }
    }
    if (execvp(command[0], command) == -1) {
        printf("command not found\n");
        exit(EXIT_FAILURE);
    }
    freeMemory(command);
}

//Function from Ex1.
int howManyWords(char *sentences) {
    int i;
    int counterOfWords = 0;
    int counterOfLetters = 0;
    for (i = 0; i <= strlen(sentences); i++) {
        if (sentences[i] != '\n' && sentences[i] != '\0' && sentences[i] != ' ') {
            counterOfLetters++;
        }
        if (sentences[i] == ' ' || sentences[i] == '\n' || sentences[i] == '\0') {
            if (counterOfLetters > 0) {
                counterOfWords++;
                counterOfLetters = 0;
            }
        }
    }
    return counterOfWords;
}

int howManyLetters(char *sentences) {
    int i;
    int counterOfLetters = 0;
    int counter = 0;
    for (i = 0; i < strlen(sentences); i++) {
        if (sentences[i] != '\n' && sentences[i] != '\0' && sentences[i] != ' ') {
            counterOfLetters++;
        }
        if (sentences[i] == ' ' || sentences[i] == '\n' || sentences[i] == '\0') {
            if (counterOfLetters > 0) {
                counter += counterOfLetters;
            }
        }
    }
    counter += howManyWords(sentences);
    return counter;
}

