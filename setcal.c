#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_NUMBER_OF_ROWS 1000
#define ROWS_TO_ALLOCATE 3
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

void printContentNoLength(char **content){
//    int l = 0;
//    int i = 0;
//    while(l < sizeof(content)){
//        printf("%s",content[i]);
//        l+=sizeof(content[i]);
//        i++;
//    }
    int l = sizeof(content)/sizeof(*content);
    for(int i = 0; i <l; i++)
        printf("%s",content[i]);
}
void printContent(char **content, int length)
{
    for(int i = 0; i < length; i++)
    {
        printf("%s ", content[i]);
    }
    printf("\n");
}
void print(Row *rows, int rowsCount)
{
    for(int i = 0; i < rowsCount; i++)
    {
        printContent(rows[i].set.content, rows[i].set.length);
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
    return elementCounter;
}
// frees the dynamically allocated array of rows
// takes 2 inputs the allocated array and the number of used elements
// loops through the used elements and frees the allocated memory
int freeAll(Row *rows, int rowsCount)
{
    for(int i = 0; i < rowsCount; i++)
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
    free(rows);
    return 1;
}
void removeEndLine(char *str)
{
    str[strlen(str)-1] = '\0';
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
    removeEndLine(&line[previousPosition]);
    setContent[contentSize-1] = &line[previousPosition];
    row->set.content = setContent;
    row->set.length = ++elementCounter;
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
// returns EMPTY_FILE if there are no lines in the file
// returns 1 if everything went ok
int loadSetsFromFile(Row **rows, FILE *fileptr, int *rowsCount, int *allocatedRowsCount)
{
    int lineCounter = 0;
    int counter = 1;
    char *line = (char *)malloc(CHUNK);
    while (fgets(line, CHUNK, fileptr))
    {
        if (lineCounter < MAX_NUMBER_OF_ROWS && line[strlen(line) - 1] == '\n')
        {
            if(lineCounter == *allocatedRowsCount-1)
            {
                *allocatedRowsCount += ROWS_TO_ALLOCATE;
                *rows = realloc(*rows, *allocatedRowsCount * sizeof(Row));
            }
            if (parseType(&(*rows)[lineCounter], line) == INVALID_ARGUMENT)
            {
                freeAll(*rows, lineCounter);
                free(line);
                return INVALID_ARGUMENT;
            }

            (*rows)[lineCounter].line = line;
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

    *rowsCount = lineCounter;
    return 1;
}

//commands for sets
//is the set empty?
int setIsEmpty(Set *set){
    if(set->length == 0)
        return true;
    return false;
}
//number of strings in the set
int setCard(Set *set){
    return set->length;
}
int setContainsString(Set *set, char *str){
    for(int i = 0; i < set->length; i++){
        if(strcmp(set->content[i],str))
            return true;
    }
    return false;
}
//return string array of the complement, dont forget to free() after printing
char **setComplement(Set *set, Set *uni){
    char **ret = (char**)malloc(uni->length*sizeof(char*));
    int index = 0;
    for(int i = 0; i < set->length; i++){
        if(setContainsString(uni, set->content[i]) == 0)
        {
            ret[index] = set->content[i];
            index++;
        }
    }
    ret = (char**)realloc(ret, index*(sizeof(char*)));
    return ret;
}
//return a+b mixed, dont forget to free() after printing
char **setUnion(Set *a, Set *b){
    char **ret = (char**)malloc((a->length+b->length)*sizeof(char*));
    int index = 0;
    for(int i = 0; i < a->length; i++){
        ret[index] = a->content[i];
        index++;
    }
    for(int i = 0; i < b->length; i++){
        if(setContainsString(a, b->content[i]) == 0)
        {
            ret[index] = b->content[i];
            index++;
        }
    }
    ret = (char**)realloc(ret, index*(sizeof(char*)));
    return ret;
}
//return intersect (a && b), dont forget to free() after printing
char **setIntersect(Set *a, Set *b){
    char **ret = (char**)malloc(a->length*sizeof(char*));
    int index = 0;
    for(int i = 0; i < b->length; i++){
        if(setContainsString(a, b->content[i]) == 1)
        {
            ret[index] = b->content[i];
            index++;
        }
    }
    ret = (char**)realloc(ret, index*(sizeof(char*)));
    return ret;
}
//return a\b
char **setMinus(Set *a, Set *b){
    char **ret = (char**)malloc(a->length*sizeof(char*));
    int index = 0;
    for(int i = 0; i < a->length; i++){
        if(setContainsString(b, a->content[i]) == 0)
        {
            ret[index] = a->content[i];
            index++;
        }
    }
    ret = (char**)realloc(ret, index*(sizeof(char*)));
    return ret;
}
//is a subset or equal b
bool setIsSubsetOrEq(Set *a, Set *b){
    for(int i = 0; i < a->length; i++){
        if(setContainsString(b, a->content[i]) == 0)
        {
            return false;
        }
    }
    return true;
}
//is a a subset of b, but not equal to b
bool setIsSubset(Set *a, Set *b){
    return a->length != b->length && setIsSubsetOrEq(a, b);
}
//are the two sets equal
bool setEquals(Set *a, Set *b){
    return a->length == b->length && setIsSubsetOrEq(a, b);
}

int main(int argc, char **argv)
{
    char *randomInput[] = {"ahoj", "prd", "test", "nevímnìcodlouhého"};
    printContentNoLength(randomInput);

    Row *rows = (Row *)malloc(ROWS_TO_ALLOCATE * sizeof(Row));
    int allocatedRowsCount = ROWS_TO_ALLOCATE;
    int rowsCount = 0;
    char *filename = {'\0'};
    FILE *file = NULL;

    if (parseArgs(argc, argv, &filename) == NOT_ENOUGH_ARGUMENTS)
        return NOT_ENOUGH_ARGUMENTS;

    if (openFile(&file, filename) == CANNOT_OPEN_FILE)
        return CANNOT_OPEN_FILE;

    if (loadSetsFromFile(&rows, file, &rowsCount, &allocatedRowsCount) == EMPTY_FILE)
        return EMPTY_FILE;

    print(rows, rowsCount);
    freeAll(rows, rowsCount);
    fclose(file);
    return 0;
}
