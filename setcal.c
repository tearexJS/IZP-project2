#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_NUMBER_OF_ROWS 1000
#define ROWS_TO_ALLOCATE 3
#define CHUNK 30
#define NUMBER_OF_COMMANDS 9
#define NOT_ENOUGH_ARGUMENTS 2
#define CANNOT_OPEN_FILE 3
#define EMPTY_FILE 4
#define FILE_TOO_LONG 5
#define INVALID_ARGUMENT 6
#define TOO_MANY_ARGUMENTS 7

#define ERROR(msg, errCode)       \
    {                             \
        do                        \
        {                         \
            fprintf(stderr, msg); \
            return errCode;       \
        } while (0);              \
    }
enum StateRelation
{
    waitForLParentheses,
    readFirstElement,
    waitForSecondElement,
    readSecondElement,
    waitForRParentheses
};
enum StateSet
{
    waitForSpace,
    readElement
};
enum Type
{
    UNI,
    SET,
    REL,
    OUT,
    OUT_int,
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

    int arg1;
    int arg2;
    int arg3;
} CommandProperties;

typedef struct
{
    enum Type type;
    union
    {
        Set set;
        Relation relation;
        CommandProperties command;
        char outputValue;
        //int outputCountElements; // zmena, pocet prvku
    };
    char *line;
} Row;

typedef struct
{
    char *name;
    int (*func)(CommandProperties, Row **);
    int argc;
} Command;

void replaceSpaceWithZero(char *line)
{
    for(int i = 0; line[i] != '\0'; i++)
    {
        if(line[i] == ' ' || line[i] == ')')
            line[i] = '\0';
    }
}

int strToInt(char *str)
{
    char *endptr;
    int num = strtol(str, &endptr, 10);
    if (endptr != NULL)
    {
        return num;
    }
    return -1;
}
void printSetContent(char **content, int length)
{
    if(length > 0)
    {
        for(int i = 0; i < length; i++)
        {
            printf("%s ", content[i]);
        }
    }
    printf("\n");
}
void printRelContent(Pair *pair, int pairCount)
{
    for(int i = 0; i < pairCount; i++)
    {
        printf("(%s ", pair[i].first);
        printf("%s)", pair[i].second);
        printf(" ");
    }
    printf("\n");
}
void print(Row *rows, int rowsCount)
{
    for(int i = 0; i < rowsCount; i++)
    {
        if (rows[i].type == UNI)
        {
            printf("U ");
            printSetContent(rows[i].set.content, rows[i].set.length);
        }
        else if (rows[i].type == SET)
        {
            printf("S ");
            printSetContent(rows[i].set.content, rows[i].set.length);
        }
        else if (rows[i].type == REL)
        {
            printf("R ");
            printRelContent(rows[i].relation.content, rows[i].relation.contentSize);
        }
        else if (rows[i].type == OUT)
        {
            rows[i].outputValue ? printf("true\n") : printf("false\n");
        }
        else if (rows[i].type == OUT_int)
        {
            int count = rows[i].outputValue;
            printf("%d\n",count);

        }
        else if (rows[i].type == COMMAND)
        {
            printf("comand\n");

        }
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
// returns the content size of the set
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
            free(rows[i].relation.content);
        }
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
    enum StateSet state = waitForSpace;
    int elementCounter = 0;

    for (int i = 0; line[i] != '\0'; i++)
    {
        if(state == waitForSpace)
        {
            switch (line[i])
            {
                case ' ':
                    state = readElement;
                    break;
                default:
                    state = waitForSpace;
                    break;
            }
        }
        else if(state == readElement && isalnum(line[i]))
        {
            setContent[elementCounter] = &line[i];
            state = waitForSpace;
            elementCounter++;
        }
    }

    row->set.content = setContent;
    if(elementCounter == 0)
        row->set.length = elementCounter;
    else
    {
        row->set.length = elementCounter;
        replaceSpaceWithZero(line);
        removeEndLine(setContent[elementCounter-1]);
    }
    return 1;
}
// parsing relations utilizing a "state machine" and saving them as an array of Pairs
// looping through the line and checking the parentheses and the spaces
// calling the replaceSpaceWithZero at the end to ensure that every element of the pair is a signle string without spaces or )
// returns 1 if everything went ok
// returns INVALID_ARGUMENT if something is wrong with the relation format
int parseRelation(Row *row, char *line)
{
    int pairCount = 1;
    Pair *pair = (Pair *)malloc(pairCount * sizeof(Pair));
    int iter = 2; // to begin at the first element of the relation

    enum StateRelation state = waitForLParentheses;
    while(line[iter] != '\n')
    {
        if(state == waitForLParentheses)
        {
            switch(line[iter])
            {
                case '(':
                    state = readFirstElement; // signaling to read the first element of the pair in the next iteration
                    break;
                case ' ':
                    state = waitForLParentheses;
                    break;
                default:
                    ERROR("Invalid relation format", INVALID_ARGUMENT);
            }
        }
        else if (state == readFirstElement)
        {
            if(isalnum(line[iter]))
            {
                pair[pairCount-1].first = &line[iter];
                state = waitForSecondElement; // signaling to wait until the second element is reached
            }
            else
                ERROR("Invalid relation format", INVALID_ARGUMENT);

        }
        else if (state == waitForSecondElement)
        {
            switch(line[iter])
            {
                case ' ':
                    state = readSecondElement; // signalig to read the second element in the next iteration
                    break;
                case ')':
                    ERROR("Relation is not a pair", INVALID_ARGUMENT);
                default:
                    break;
            }
        }
        else if (state == readSecondElement)
        {
            if(isalnum(line[iter]))
            {
                pair[pairCount-1].second = &line[iter];
                state = waitForRParentheses; // signalit to expect ) in the next interation
            }
        }
        else if (state == waitForRParentheses)
        {
            switch(line[iter])
            {
                case ')':
                    pairCount++;
                    pair = realloc(pair, pairCount*sizeof(Pair));
                    state = waitForLParentheses; // signaling to wait until ( is reached
                    break;
                case ' ':
                    ERROR("Invalid relation format", INVALID_ARGUMENT);
            }
        }
        iter++;
    }
    replaceSpaceWithZero(line);
    row->relation.content = pair;
    row->relation.contentSize = pairCount-1;
    return 1;
}

int parseCommand(Row *row, char *line)
{
    char *arg1 = NULL;
    char *arg2 = NULL;
    for(int i = 1; line[i]!='\0'; i++)
    {
        if(line[i] == ' ' && isalpha(line[i+1]))
            row->command.name = &line[i+1];
        else if (line[i] == ' ' && isdigit(line[i+1]))
        {
            if(!arg1){
                arg1 = &line[i+1];
            }else if (!arg2){
                arg2 = &line[i+1]; //ne plus dva
            }else
                ERROR("Too many args", TOO_MANY_ARGUMENTS)
        }
    }
    replaceSpaceWithZero(line);
    if(arg1)
        row->command.arg1 = strToInt(arg1);
    else
        ERROR("Not enough args", NOT_ENOUGH_ARGUMENTS);
    if(arg2)
        row->command.arg2 = strToInt(arg2);
    return 1;
}
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
        return parseRelation(row, line);
    }
    else if (line[0] == 'C')
    {
        row->type = COMMAND;
        return parseCommand(row, line);
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
    int allocedLineLen = CHUNK;
    int usedLineLen = 0;
    char buffer[CHUNK];
    char *line = (char *)malloc(CHUNK);
    while (fgets(buffer, CHUNK, fileptr))
    {
        if(allocedLineLen <= (usedLineLen+1))
        {
            allocedLineLen += CHUNK;
            line = realloc(line, allocedLineLen);
            memcpy((line+usedLineLen), buffer, strlen(buffer));
            usedLineLen = strlen(line);
        }
        else
        {
            strcpy(&line[usedLineLen], buffer);
            usedLineLen = strlen(line);
        }

        if (lineCounter < MAX_NUMBER_OF_ROWS && line[strlen(line) - 1] == '\n')
        {
            if(lineCounter == *allocatedRowsCount-1)
            {
                *allocatedRowsCount += ROWS_TO_ALLOCATE;
                *rows = realloc(*rows, *allocatedRowsCount * sizeof(Row));
            }
            if (parseType(&(*rows)[lineCounter], line) != 1)
            {
                freeAll(*rows, lineCounter);
                free(line);
                return INVALID_ARGUMENT;
            }

            (*rows)[lineCounter].line = line;
            line = (char *)malloc(CHUNK);
            lineCounter++;
            allocedLineLen = CHUNK;
            usedLineLen = 0;
            continue;
        }
        else if (lineCounter > MAX_NUMBER_OF_ROWS)
            ERROR("File is too long", FILE_TOO_LONG);

    }
    free(line);
    if (!lineCounter)
        ERROR("Empty file", EMPTY_FILE)

    *rowsCount = lineCounter;
    return 1;
}

//commands for sets
//is the set empty?
int setIsEmpty(CommandProperties props, Row **rows)
{
    int arg = props.arg1-1; //jediny parametr mnozinove operace
    int commandPos = props.arg3; //vystupni hodnota?

    if((*rows)[arg].type == SET)
    {
        (*rows)[commandPos].type = OUT;
        if((*rows)[arg].set.length == 0)
        {
            (*rows)[commandPos].outputValue = true;
        }
        (*rows)[commandPos].outputValue = false;
        return 1;
    }
    ERROR("There is no set on this line", INVALID_ARGUMENT);
}
//number of strings in the set
int setCard(CommandProperties props, Row **rows)
{
    int arg = props.arg1-1; //jediny parametr mnozinove operace
    int commandPos = props.arg3; //vystupni hodnota?

    if((*rows)[arg].type == SET)
    {
        (*rows)[commandPos].type = OUT_int;
        int count = (*rows)[arg].set.length;
        printf("set card count %d",count);
        (*rows)[commandPos].outputValue = count;
        return 1;
    }
    ERROR("There is no set on this line", INVALID_ARGUMENT);
}

//pomocna funkce
int setContainsString(Set set, char *str){
    for(int i = 0; i < set.length; i++){
        if(!strcmp(set.content[i],str)){
            return true;
        }
    }
    return false;
}
// return string array of the complement, dont forget to free() after printing
int setComplement(CommandProperties props, Row **rows){
    int arg = props.arg1-1; //jediny parametr mnozinove operace
    if((*rows)[0].type == UNI){
        Set set = (*rows)[arg].set;
        Set uni = (*rows)[0].set;

        char **ret = (char **)malloc(uni.length * sizeof(char *));
        int index = 0;//pocet vystupnich prvku
        for(int i = 0; i < uni.length; i++){ //prochazim prvky universa
            if(!setContainsString(set, uni.content[i])) //hledam prvek z univerza v mnozine
            {//kdyz neni prvek v nozine, dam do komplementu
                ret[index++] = uni.content[i];

            }
        }
        int commandPos = props.arg3; //vystupni hodnota?
        (*rows)[commandPos].type = SET;
        (*rows)[commandPos].set.content = ret;
        (*rows)[commandPos].set.length = index;
        return 1;
    }
    ERROR("There is no set on this line", INVALID_ARGUMENT); //nedefinovane universum
}
int copyContent(char **contentA, char ** contentB, int lenA, int lenB)
{
    int newLen = lenA;
   for(int i = 0; i < lenB; i++)
   {
       contentA[newLen+i] = contentB[i];
       newLen++;
   } 
   return newLen;
}

int contentContainsElement(char ** content, int contentSize, char *element)
{
    for(int i = 0; i < contentSize; i++)
    {
        if(!strcmp(content[i], element))
            return true;
    }
    return false;
}

// return a+b mixed, dont forget to free() after printing
int setUnion(CommandProperties props, Row **rows)
{
    int commandPos = props.arg3;
    int newLen = 0;
    Set a = (*rows)[props.arg1-1].set;
    Set b = (*rows)[props.arg2-1].set;
    char **newSetContent = (char **)malloc((a.length)+(b.length)*sizeof(char*));
    newLen = copyContent(newSetContent, a.content, newLen, a.length);
    for(int i = 0; i < b.length; i++)
    {
        if(!contentContainsElement(newSetContent, newLen, b.content[i]))
        {
            newSetContent[newLen+i] = b.content[i];
            newLen++;
        }
    }
    (*rows)[commandPos].type = SET;
    (*rows)[commandPos].set.content = newSetContent;
    (*rows)[commandPos].set.length = newLen;
    return 1;
}

//return intersect (a && b), dont forget to free() after printing
int setIntersect(CommandProperties props, Row **rows){
    //printf("bezi interssect\n");
    int arg;
    arg = props.arg1-1; //prvni parametr mnozinove operace
    Set a = (*rows)[arg].set;
    arg = props.arg2-1; //druhy parametr mnozinove operace
    Set b = (*rows)[arg].set;

    char **ret = (char**)malloc(a.length*sizeof(char*)); //predbezna velikost pole je dostatecne velka (delam prunik A B), vysledek bude kratsi
    int index = 0;
    for(int i = 0; i < b.length; i++){ //prochazime prvky druhe mnoziny
        if(setContainsString(a, b.content[i])) //a pro kazdy prvek se ptame, jeslti je v druhe mnozine
        {
            ret[index++] = b.content[i]; //kdyz je v obou mnozinach, vlozime.

        }
    }
    //ret = (char**)realloc(ret, index*(sizeof(char*)));
    int commandPos = props.arg3; //vystupni hodnota?
    (*rows)[commandPos].type = SET;
    (*rows)[commandPos].set.content = ret;
    (*rows)[commandPos].set.length = index;
    return 1;
}
//return a\b
int setMinus(CommandProperties props, Row **rows){
    int arg;
    arg = props.arg1-1; //prvni parametr mnozinove operace
    Set a = (*rows)[arg].set;
    arg = props.arg2-1; //druhy parametr mnozinove operace
    Set b = (*rows)[arg].set;

    char **ret = (char**)malloc(a.length*sizeof(char*));
    int index = 0;
    for(int i = 0; i < a.length; i++){
        if(!setContainsString(b, a.content[i]))
        {
            ret[index++] = a.content[i];
        }
    }

    int commandPos = props.arg3; //vystupni hodnota
    (*rows)[commandPos].type = SET;
    (*rows)[commandPos].set.content = ret;
    (*rows)[commandPos].set.length = index;
    return 1;
}
//is a subset or equal b
int setIsSubsetOrEq(CommandProperties props, Row **rows){
    int arg1 = props.arg1-1;
    int arg2 = props.arg2-1;
    int commandPos = props.arg3;
    Set a = (*rows)[arg1].set;
    Set b = (*rows)[arg2].set;
    for(int i = 0; i < a.length; i++){
        if(!setContainsString(b, a.content[i]))
        {  
            (*rows)[commandPos].outputValue = false;
            (*rows)[commandPos].type = OUT;
            return 1;
        }
    }
    (*rows)[commandPos].outputValue = true;
    (*rows)[commandPos].type = OUT;
    return 1;
}
//is a a subset of b, but not equal to b
int setIsSubset(CommandProperties props, Row **rows){
    
    Set a = (*rows)[props.arg1-1].set;
    Set b = (*rows)[props.arg2-1].set;
    if(b.length > a.length)
        setIsSubsetOrEq(props, rows);
    else
    {
        (*rows)[props.arg3].type = OUT;
        (*rows)[props.arg3].outputValue = false;    
    }
    return 1;
}
//are the two sets equal
int setEquals(CommandProperties props, Row **rows){
    Set a = (*rows)[props.arg1-1].set;
    Set b = (*rows)[props.arg2-1].set;
    if(a.length == b.length)
        setIsSubsetOrEq(props, rows);
    else
    {
        (*rows)[props.arg3].type = OUT;
        (*rows)[props.arg3].outputValue = false;    
    }
    return 1;
    
}

const Command commandList[NUMBER_OF_COMMANDS] = 
{
    {"empty", setIsEmpty, 1},
    {"complement", setComplement, 1},
    {"card", setCard, 1}, //dopsat vypis do konzole
    // {"union", setUnion, 2},
    {"intersect", setIntersect, 2},
    {"minus", setMinus, 2},
    // {"subseteq", setIsSubsetOrEq, 2},
    // {"subset", setIsSubset, 2},
    // {"equals", setEquals, 2}
};
// executes the commands utilizing function pointers
// loops through the const array where the func pointers are stored and executes the command which needs to be executed
// returns the received value from the called function when something went wrong
// returns 1 if everything went ok
int executeCommands(Row **rows, int rowsCount)
{
    for(int i = 0; i < rowsCount; i++)
    {
            for(int j = 0; j < NUMBER_OF_COMMANDS; j++)
            {
                if((*rows)[i].type == COMMAND)
                {
                    if(!strcmp(commandList[j].name, (*rows)[i].command.name))
                    {
                        int arg1 = (*rows)[i].command.arg1-1;
                        //int arg2 = rows[i].command.arg2;
                        if((*rows)[arg1].type == REL || (*rows)[arg1].type == SET)
                        {

                            (*rows)[i].command.arg3 = i;
                            int recVal = commandList[j].func((*rows)[i].command, rows);
                            if(recVal != 1)
                                return recVal;

                        }
                    }
                }
            }
    }
    return 1;
}

int main(int argc, char **argv)
{
    Row *rows = (Row *)malloc(ROWS_TO_ALLOCATE * sizeof(Row));
    int allocatedRowsCount = ROWS_TO_ALLOCATE;
    int rowsCount = 0;
    char *filename = {'\0'};
    FILE *file = NULL;

    if (parseArgs(argc, argv, &filename) == NOT_ENOUGH_ARGUMENTS)
        return NOT_ENOUGH_ARGUMENTS;

    if (openFile(&file, filename) == CANNOT_OPEN_FILE)
        return CANNOT_OPEN_FILE;

    if (loadSetsFromFile(&rows, file, &rowsCount, &allocatedRowsCount) != 1)
        return EMPTY_FILE;
    if(executeCommands(&rows, rowsCount) == 1)
    {
        print(rows, rowsCount);
    }

    freeAll(rows, rowsCount);
    fclose(file);
    return 0;
}
