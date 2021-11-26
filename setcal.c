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
} Row;

void printSet(Set *set)
{
    printf("%c ", set->type);
    for (int i = 0; i < set->length; i++)
    {
        char *now = set->content[i];
        printf("%s ", now);
    }
    printf("\n");
}

void addIntoSet(Set *set, char content[])
{
    printf("Adding into set %s: length=%d, contentSize=%d\n", content, set->length, set->contentSize);
    printSet(set);
    set->length++; // increase length of set
    if (set->length == 1)
    {
        set->contentSize = 1;
        ;
    }
    char **tmp = set->content; // copy original content into a temporary variable

    set->content = malloc(1 * sizeof(*set->content));
    if (set->content)
    {
        bool needToResize = true;
        if (set->contentSize <= set->length)
            needToResize = true;
        char **tmp2 = needToResize ? realloc(set->content, 2 * sizeof(*set->content)) : set->content; // creating a new array with bigger size
        if (tmp2)                                                                                     // if it was actually created (memory allocated)
        {
            if (needToResize)
                set->contentSize *= 2;
            printf("Created tmp2 while adding %s: length=%d, contentSize=%d\n", content, set->length, set->contentSize);
            printSet(set);
            set->content = tmp2;
            for (int i = 0; i < set->length - 1; i++)
            { // copy all from the previous array
                set->content[i] = tmp[i];
            }

            set->content[set->length - 1] = content; // finally the new value
                                                     //            free(tmp2);
            printf("Added into set %s: length=%d, contentSize=%d\n", content, set->length, set->contentSize);
            printSet(set);
        }
        //        free(tmp);
        printf("Freed tmp while adding %s: length=%d, contentSize=%d\n", content, set->length, set->contentSize);
        printSet(set);
    }
    // found at https://stackoverflow.com/questions/12917727/resizing-an-array-in-c, edited for my purpose

    printSet(set);
}

bool isInSet(char *content, Set *set)
{
    for (int i = 0; i < set->length; i++)
    {
        if (compareStrings(content, set->content[i]))
            return true;
    }
    return false;
}

void removeFromSet(Set *set, char *content)
{
    if (!isInSet(content, set))
    {
        return;
    }
    set->length--;
    int rem = 0;
    for (int i = 0; i < set->length + rem; i++)
    {
        if (i == set->length)
        {
            set->content[i] = ""; // set the remaining last values (which were moved to the left) to ""
        }
        if (i < set->length)
        {
            if (compareStrings(content, set->content[i]))
                rem++;
            if (rem > 0)
                set->content[i] = set->content[i + rem]; // PROBLEM: not reducing the size of the array, only removing the member and reducing length variable
        }
    }
}

bool isSubset(Set *smaller, Set *bigger)
{
    for (int i = 0; i < smaller->length; i++)
    {
        bool isThere = false;
        for (int j = 0; j < bigger->length; j++)
        {
            if (compareStrings(smaller->content[i], bigger->content[j]))
            {
                isThere = true;
                break;
            }
        }
        if (!isThere)
            return false;
    }
    return true;
}

bool areThereDupliciteMembersInSet(Set *set)
{
    for (int i = 0; i < set->length; i++)
    {
        for (int j = i + 1; j < set->length; j++)
        {
            if (compareStrings(set->content[i], set->content[j]))
                return true;
        }
    }
    return false;
}

bool isSet(Set *set)
{
    if (areThereDupliciteMembersInSet(set))
        return false;

    return true;
}

// check if the set is empty
bool isEmpty(Set *s)
{
    return false;
}

// Print the number of members in the set
int card(Set *s)
{
    return s->length;
}

void complement(Set *ret, Set *s, Set *u)
{
    ret->length = 0;
    ret->type = 'S';
    for (int i = 0; i < s->length; i++)
    {
        if (!isInSet(s->content[i], u))
            addIntoSet(ret, s->content[i]);
    }
}
// parsing the input arguments
int parseArgs(int argc, char **argv, char *filename)
{
    if (argc == 2)
    {
        filename = argv[1];
        return 1;
    }
    ERROR("Not enough arguments", NOT_ENOUGH_ARGUMENTS);
}
// opens file and returns 1 if not succesful returns CANNOT_OPEN_FILE
int openFile(FILE *file, char *filename)
{
    file = fopen(filename, "r");
    if (file)
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

int parseSet(Row *row, char *line)
{

    int rows = getContentSize(line);
    char **setContent = (char **)malloc(rows * CHUNK);
    for (int i = 0; line[i] != '\0'; i++)
    {
    }
}
int parseRelation(Row *row, char *line)
{

}
int parseType(Row *row, char *line)
{
    if (line[0] == 'U')
    {
        row->type = UNI;
        parseSet(row, &line[2]);
    }
    else if (line[0] == 'S')
    {
        row->type = SET;
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

// loads the sets from file
int loadSetsFromFile(Row *rows, FILE *fileptr)
{
    int setCounter = 0;
    int counter = 1;
    char *line = (char *)malloc(CHUNK);
    while (fgets(line, CHUNK, fileptr))
    {
        if (setCounter < MAX_NUMBER_OF_ROWS && line[strlen(line) - 1] == '\n')
        {
            strcpy(sets[setCounter].content, line);
            sets[setCounter].length = (strlen(line));
            if (parseType(&rows[setCounter], line) == INVALID_ARGUMENT)
            {
                free(line);
                return INVALID_ARGUMENT;
            }
            setCounter++;
            continue;
        }
        else if (setCounter > MAX_NUMBER_OF_ROWS)
            ERROR("File is too long", FILE_TOO_LONG);

        counter++;
        line = realloc(line, counter * CHUNK);
    }
    free(line);

    if (!setCounter)
        ERROR("Empty file", EMPTY_FILE)

    return 1;
}
int main(int argc, char **argv)
{
    Row rows[MAX_NUMBER_OF_ROWS];
    int setsCount = 0;
    char filename[100];
    FILE *file = NULL;

    if (parseArgs(argc, argv, filename) == NOT_ENOUGH_ARGUMENTS)
        return NOT_ENOUGH_ARGUMENTS;

    if (openFile(file, filename) == CANNOT_OPEN_FILE)
        return CANNOT_OPEN_FILE;

    if (loadSetsFromFile(rows, file) == EMPTY_FILE)
        return EMPTY_FILE;

    return 0;
}
