#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_NUMBER_OF_ROWS 1000
#define CHUNK 30

#define NOT_ENOUGH_ARGUMENTS 2
#define CANNOT_OPEN_FILE 3
#define EMPTY_FILE 4
#define FILE_TOO_LONG 5
#define INVALID_ARGUMENT 6

#define ERROR(msg, errCode)       \
    {                             \
        do                        \
        {                         \
            fprintf(stderr, msg); \
            return errCode;       \
        } while (0);              \
    }

enum Type
{
    UNI,
    SET,
    REL,
    COMMAND
};

typedef struct
{
    char *first;
    char *second;
} Pair;

typedef struct
{
    Pair *content;
    int contentSize;
} Relation;

typedef struct
{
    int length;
    char **content;
} Set;

typedef struct
{
    char *name;

    void *arg1;
    void *arg2;

} CommandProperties;

typedef struct
{
    char *name;
    int (*func)(CommandProperties);
    int argc;
} Command;

typedef struct
{
    enum Type type;
    union
    {
        Set set;
        Relation relation;
        Command command;
    };
    char *line;
} Row;

// void printSet(Set *set)
// {
//     printf("%c ", set->type);
//     for (int i = 0; i < set->length; i++)
//     {
//         char *now = set->content[i];
//         printf("%s ", now);
//     }
//     printf("\n");
// }

// void addIntoSet(Set *set, char content[])
// {
//     printf("Adding into set %s: length=%d, contentSize=%d\n", content, set->length, set->contentSize);
//     printSet(set);
//     set->length++; // increase length of set
//     if (set->length == 1)
//     {
//         set->contentSize = 1;
//         ;
//     }
//     char **tmp = set->content; // copy original content into a temporary variable

//     set->content = malloc(1 * sizeof(*set->content));
//     if (set->content)
//     {
//         bool needToResize = true;
//         if (set->contentSize <= set->length)
//             needToResize = true;
//         char **tmp2 = needToResize ? realloc(set->content, 2 * sizeof(*set->content)) : set->content; // creating a new array with bigger size
//         if (tmp2)                                                                                     // if it was actually created (memory allocated)
//         {
//             if (needToResize)
//                 set->contentSize *= 2;
//             printf("Created tmp2 while adding %s: length=%d, contentSize=%d\n", content, set->length, set->contentSize);
//             printSet(set);
//             set->content = tmp2;
//             for (int i = 0; i < set->length - 1; i++)
//             { // copy all from the previous array
//                 set->content[i] = tmp[i];
//             }

//             set->content[set->length - 1] = content; // finally the new value
//                                                      //            free(tmp2);
//             printf("Added into set %s: length=%d, contentSize=%d\n", content, set->length, set->contentSize);
//             printSet(set);
//         }
//         //        free(tmp);
//         printf("Freed tmp while adding %s: length=%d, contentSize=%d\n", content, set->length, set->contentSize);
//         printSet(set);
//     }
//     // found at https://stackoverflow.com/questions/12917727/resizing-an-array-in-c, edited for my purpose

//     printSet(set);
// }

// bool isInSet(char *content, Set *set)
// {
//     for (int i = 0; i < set->length; i++)
//     {
//         if (compareStrings(content, set->content[i]))
//             return true;
//     }
//     return false;
// }

// void removeFromSet(Set *set, char *content)
// {
//     if (!isInSet(content, set))
//     {
//         return;
//     }
//     set->length--;
//     int rem = 0;
//     for (int i = 0; i < set->length + rem; i++)
//     {
//         if (i == set->length)
//         {
//             set->content[i] = ""; // set the remaining last values (which were moved to the left) to ""
//         }
//         if (i < set->length)
//         {
//             if (compareStrings(content, set->content[i]))
//                 rem++;
//             if (rem > 0)
//                 set->content[i] = set->content[i + rem]; // PROBLEM: not reducing the size of the array, only removing the member and reducing length variable
//         }
//     }
// }

// bool isSubset(Set *smaller, Set *bigger)
// {
//     for (int i = 0; i < smaller->length; i++)
//     {
//         bool isThere = false;
//         for (int j = 0; j < bigger->length; j++)
//         {
//             if (compareStrings(smaller->content[i], bigger->content[j]))
//             {
//                 isThere = true;
//                 break;
//             }
//         }
//         if (!isThere)
//             return false;
//     }
//     return true;
// }

// bool areThereDupliciteMembersInSet(Set *set)
// {
//     for (int i = 0; i < set->length; i++)
//     {
//         for (int j = i + 1; j < set->length; j++)
//         {
//             if (compareStrings(set->content[i], set->content[j]))
//                 return true;
//         }
//     }
//     return false;
// }

// bool isSet(Set *set)
// {
//     if (areThereDupliciteMembersInSet(set))
//         return false;

//     return true;
// }

// // check if the set is empty
// bool isEmpty(Set *s)
// {
//     return false;
// }

// // Print the number of members in the set
// int card(Set *s)
// {
//     return s->length;
// }

// void complement(Set *ret, Set *s, Set *u)
// {
//     ret->length = 0;
//     ret->type = 'S';
//     for (int i = 0; i < s->length; i++)
//     {
//         if (!isInSet(s->content[i], u))
//             addIntoSet(ret, s->content[i]);
//     }
// }
// parsing the input arguments
void printContent(char **content, int length)
{
    for(int i = 0; i < length; i++)
    {
        printf("%s ", content[i]);
    }
    printf("\n");
}
void print(Row *rows)
{
    for(int i = 0; i < MAX_NUMBER_OF_ROWS; i++)
    {
        if(rows[i].type == SET || rows[i].type == UNI)
            printContent(rows[i].set.content, rows[i].set.length);
        else
            break;

    } 
}
int parseArgs(int argc, char **argv, char **filename)
{
    (void)filename;
    if (argc == 2)
    {
        *filename = argv[1];
        return 1;
    }
    ERROR("Not enough arguments", NOT_ENOUGH_ARGUMENTS);
}
// opens file and returns 1 if not succesful returns CANNOT_OPEN_FILE
int openFile(FILE **file, char *filename)
{
    *file = fopen(filename, "r");
    if (*file)
        return 1;
    ERROR("Could not open given file", CANNOT_OPEN_FILE);
}

int getContentSize(char *line)
{
    int elementCounter = 0;
    for (int i = 0; line[i] != '\0'; i++)
    {
        if (line[i] == ' ')
            elementCounter++;
    }
    return elementCounter+1;
}
// frees all dynamicaly allocated memory by utilizing the char *line attribute in data type Row
int freeAll(Row *rows)
{
    for(int i; i < MAX_NUMBER_OF_ROWS; i++)
    {
        free(rows[i].line);
        if(rows[i].type == UNI || rows[i].type == SET)
        {    
            free(rows[i].set.content);
        }
        else if (rows[i].type == REL)
        {
            free(rows[i].relation.content[0].first);
        }
        // else if (rows[i].type == COMMAND)
        // {
        //     free(rows[i].command.)
        // }
    }
    return 1;
}
// parsing the loaded line to extract elements and store them as a Set
int parseSet(Row *row, char *line)
{

    int contentSize = getContentSize(line);
    char **setContent = (char **)malloc(contentSize * sizeof(char*));
    int elementCounter = 0;
    int previousPosition = 2;

    for (int i = 2; line[i] != '\0'; i++)
    {
        if(line[i] == ' ')
        {
            line[i] = '\0';
            setContent[elementCounter] = &line[previousPosition];
            previousPosition = i+1; 
            elementCounter++;
        }
    }
    setContent[contentSize] = &line[previousPosition];
    row->set.content = setContent;
    row->set.length = elementCounter;
    return 1;
}
// int parseRelation(Row *row, char *line)
// {

// }
// setting the type of row to universe, set, relation or command 
// if there is something else than the mentioned returns INVALID_ARGUMENT
int parseType(Row *row, char *line)
{
    if (line[0] == 'U')
    {
        row->type = UNI;
        return parseSet(row, line);
    }
    else if (line[0] == 'S')
    {
        row->type = SET;
        return parseSet(row, line);
    }
    else if (line[0] == 'R')
    {
        row->type = REL;
    }
    else if (line[0] == 'C')
    {
        row->type = COMMAND;
    }
    ERROR("Invalid argument", INVALID_ARGUMENT);
}

// loads the sets from file and further parses them by calling the parseType function
// returns FILE_TOO_LONG if there are more than 1000 lines in the file
// returns EMPTY_FILE if there are no lines the file
// returns 1 if everything went ok
int loadSetsFromFile(Row *rows, FILE *fileptr, int *rowsCount)
{
    int lineCounter = 0;
    int counter = 1;
    char *line = (char *)malloc(CHUNK);
    while (fgets(line, CHUNK, fileptr))
    {
        if (lineCounter < MAX_NUMBER_OF_ROWS && line[strlen(line) - 1] == '\n')
        {
            
            if (parseType(&rows[lineCounter], line) == INVALID_ARGUMENT)
            {
                freeAll(rows);
                free(line);
                return INVALID_ARGUMENT;
            }
            
            rows[lineCounter].line = line;
            line = (char *)malloc(CHUNK);
            lineCounter++;
            counter = 1;
            continue;
        }
        else if (lineCounter > MAX_NUMBER_OF_ROWS)
            ERROR("File is too long", FILE_TOO_LONG);

        counter++;
        line = realloc(line, counter * CHUNK);
    }
    free(line);
    if (!lineCounter)
        ERROR("Empty file", EMPTY_FILE)

    *rowsCount = ++lineCounter;
    return 1;
}

int main(int argc, char **argv)
{
    Row rows[MAX_NUMBER_OF_ROWS];
    int rowsCount = 0;
    char *filename;
    FILE *file = NULL;

    if (parseArgs(argc, argv, &filename) == NOT_ENOUGH_ARGUMENTS)
        return NOT_ENOUGH_ARGUMENTS;

    if (openFile(&file, filename) == CANNOT_OPEN_FILE)
        return CANNOT_OPEN_FILE;

    if (loadSetsFromFile(rows, file, &rowsCount) == EMPTY_FILE)
        return EMPTY_FILE;
    print(rows);
    freeAll(rows);
    fclose(file);
    return 0;
}
