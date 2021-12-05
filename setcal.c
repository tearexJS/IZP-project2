#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_NUMBER_OF_ROWS 1000
#define ROWS_TO_ALLOCATE 3
#define CHUNK 30
#define NUMBER_OF_COMMANDS 19

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
    int arg4;
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
            if(i != length-1)
                printf("%s ", content[i]);
            else
                printf("%s", content[i]);
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
        if(i != (pairCount-1))
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
            rows[i].set.length != 0 ? printf("U ") : printf("U");
            printSetContent(rows[i].set.content, rows[i].set.length);
        }
        else if (rows[i].type == SET)
        {
            rows[i].set.length != 0 ? printf("S ") : printf("S");
            printSetContent(rows[i].set.content, rows[i].set.length);
        }
        else if (rows[i].type == REL)
        {
            rows[i].relation.contentSize != 0 ? printf("R ") : printf("R");
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
    char *arg3 = NULL;
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
            }else if (!arg3){
                arg3 = &line[i+1];
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
    if(arg3)
        row->command.arg3 = strToInt(arg3);
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

//commands for sets
//is the set empty?
int setIsEmpty(CommandProperties props, Row **rows)
{
    int arg = props.arg1-1; //jediny parametr mnozinove operace
    int commandPos = props.arg3; //vystupni hodnota?

    if((*rows)[arg].type == SET || (*rows)[arg].type == UNI)
    {
        (*rows)[commandPos].type = OUT;
        if((*rows)[arg].set.length == 0)
        {
            (*rows)[commandPos].outputValue = true;
        }
        else
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

    if((*rows)[arg].type == SET || (*rows)[arg].type == UNI)
    {
        (*rows)[commandPos].type = OUT_int;
        int count = (*rows)[arg].set.length;
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

bool setsEqual(Set a, Set b)
{
    for(int i = 0; i < a.length; i++){
        if(!setContainsString(b, a.content[i]))
        {
            return false;
        }
    }
    if(a.length == b.length) return true;
    else return false;
}

// return string array of the complement, dont forget to free() after printing
int setComplement(CommandProperties props, Row **rows){
    int arg = props.arg1-1; //jediny parametr mnozinove operace
    if(!props.arg1)
        ERROR("Not enough arguments", NOT_ENOUGH_ARGUMENTS);

        if((*rows)[0].type == UNI && ((*rows)[arg].type == SET || (*rows)[arg].type == UNI)){
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
    int len = lenA;
    int newLen = lenA;
   for(int i = 0; i < lenB; i++)
   {
       contentA[len+i] = contentB[i];
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
    if(!props.arg1 || !props.arg2)
    {
        ERROR("Not enough arguments", NOT_ENOUGH_ARGUMENTS);
    }
        
    if((*rows)[props.arg1-1].type == SET && (*rows)[props.arg2-1].type == SET)
    {
        char **newSetContent = (char **)malloc((a.length+b.length + 1)*sizeof(char*));
        if(a.length > 0)
            newLen = copyContent(newSetContent, a.content, newLen, a.length);
        else if(b.length > 0)
        {
           newLen = copyContent(newSetContent, b.content, newLen, b.length);
        }
        int len = newLen;
        for(int i = 0; i < b.length; i++)
        {
            if(a.length > 0)
                if(!contentContainsElement(newSetContent, newLen, b.content[i]))
                {
                    newSetContent[len+i] = b.content[i];
                    newLen++;
                }
        }
        (*rows)[commandPos].type = SET;
        (*rows)[commandPos].set.content = newSetContent;
        (*rows)[commandPos].set.length = newLen;
        return 1;
    }
    ERROR("Invalid argument", INVALID_ARGUMENT);
}

//return intersect (a && b), dont forget to free() after printing
int setIntersect(CommandProperties props, Row **rows){
    //printf("bezi interssect\n");
    int arg;
    arg = props.arg1-1; //prvni parametr mnozinove operace
    Set a = (*rows)[arg].set;
    arg = props.arg2-1; //druhy parametr mnozinove operace
    Set b = (*rows)[arg].set;
    if(!props.arg1 || !props.arg2)
        ERROR("Not enough arguments", NOT_ENOUGH_ARGUMENTS);
        
    if((*rows)[props.arg1-1].type == SET && (*rows)[props.arg2-1].type == SET)
    {
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
    ERROR("Invalid argument", INVALID_ARGUMENT);
}
//return a\b
int setMinus(CommandProperties props, Row **rows){
    int arg;
    arg = props.arg1-1; //prvni parametr mnozinove operace
    Set a = (*rows)[arg].set;
    arg = props.arg2-1; //druhy parametr mnozinove operace
    Set b = (*rows)[arg].set;
    if(!props.arg1 || !props.arg2)
        ERROR("Not enough arguments", NOT_ENOUGH_ARGUMENTS);
    if((*rows)[props.arg1-1].type == SET && (*rows)[props.arg2-1].type == SET)
    {
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
    ERROR("Invalid argument", INVALID_ARGUMENT);
}
//is a subset or equal b
int setIsSubsetOrEq(CommandProperties props, Row **rows){
    int arg1 = props.arg1-1;
    int arg2 = props.arg2-1;
    int commandPos = props.arg3;
    Set a = (*rows)[arg1].set;
    Set b = (*rows)[arg2].set;
    if(!props.arg1 || !props.arg2)
        ERROR("Not enough arguments", NOT_ENOUGH_ARGUMENTS);

    if((*rows)[arg1].type == SET && (*rows)[arg2].type == SET)
    {
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
    ERROR("Invalid argument", INVALID_ARGUMENT);
}
//is a a subset of b, but not equal to b
int setIsSubset(CommandProperties props, Row **rows){
    
    Set a = (*rows)[props.arg1-1].set;
    Set b = (*rows)[props.arg2-1].set;
    if(!props.arg1 || !props.arg2)
        ERROR("Not enough arguments", NOT_ENOUGH_ARGUMENTS);
    if((*rows)[props.arg1-1].type == SET && (*rows)[props.arg2-1].type == SET)
    {
        if(b.length > a.length)
            setIsSubsetOrEq(props, rows);
        else
        {
            (*rows)[props.arg3].type = OUT;
            (*rows)[props.arg3].outputValue = false;    
        }
        return 1;
    }
    ERROR("Invalid argument", INVALID_ARGUMENT);
}
//are the two sets equal
int setEquals(CommandProperties props, Row **rows){
    Set a = (*rows)[props.arg1-1].set;
    Set b = (*rows)[props.arg2-1].set;
    if(!props.arg1 || !props.arg2)
        ERROR("Not enough arguments", NOT_ENOUGH_ARGUMENTS);
    if((*rows)[props.arg1-1].type == SET && (*rows)[props.arg2-1].type == SET)
    {
        if(a.length == b.length)
            setIsSubsetOrEq(props, rows);
        else
        {
            (*rows)[props.arg3].type = OUT;
            (*rows)[props.arg3].outputValue = false;    
        }
        return 1;
    }
    ERROR("Invalid argument", INVALID_ARGUMENT);
}

//commands for relations
//returns true if relation is reflexive, otherwise returns false
int relationIsReflexive(CommandProperties props, Row **rows)
{
    int arg = props.arg1-1;
    if(props.arg2 > 0) ERROR("There is no relation on this line", INVALID_ARGUMENT);
    int commandPos = props.arg3;
    int matchCount = 0;
    if((*rows)[arg].type == REL)
    {
        (*rows)[commandPos].type = OUT;
        for(int i = 0; i < (*rows)[arg].relation.contentSize; i++)
        {
            if(!strcmp((*rows)[arg].relation.content[i].first, (*rows)[arg].relation.content[i].second)) matchCount++;
        }
        if(matchCount == (*rows)[0].set.length) (*rows)[commandPos].outputValue = true;
        else (*rows)[commandPos].outputValue = false;
        return 1;
    }
    ERROR("There is no relation on this line", INVALID_ARGUMENT);
}

//returns true if relation is symmetric, otherwise returns false
int relationIsSymmetric(CommandProperties props, Row **rows)
{
    int arg = props.arg1-1;
    if(props.arg2 > 0) ERROR("There is no relation on this line", INVALID_ARGUMENT);
    int commandPos = props.arg3;
    bool symmetry;
    if((*rows)[arg].type == REL)
    {
        (*rows)[commandPos].type = OUT;
        for(int i = 0; i < (*rows)[arg].relation.contentSize; i++)
        {
            symmetry = false;
            for(int j = 0; j < (*rows)[arg].relation.contentSize && !symmetry; j++)
            {
                if(!strcmp((*rows)[arg].relation.content[i].first, (*rows)[arg].relation.content[j].second) && !strcmp((*rows)[arg].relation.content[i].second, (*rows)[arg].relation.content[j].first)) symmetry = true;
            }
            if(!symmetry)
            {
                (*rows)[commandPos].outputValue = false;
                break;
            }
        }
        if(symmetry) (*rows)[commandPos].outputValue = true;
        return 1;
    }
    ERROR("There is no relation on this line", INVALID_ARGUMENT);
}

//returns true if relation is antisymmetric, otherwise returns false
int relationIsAntisymmetric(CommandProperties props, Row **rows)
{
    int arg = props.arg1-1;
    if(props.arg2 > 0) ERROR("There is no relation on this line", INVALID_ARGUMENT);
    int commandPos = props.arg3;
    bool antisymmetry;
    if((*rows)[arg].type == REL)
    {
        (*rows)[commandPos].type = OUT;
        for(int i = 0; i < (*rows)[arg].relation.contentSize; i++)
        {
            antisymmetry = true;
            for(int j = 0; j < (*rows)[arg].relation.contentSize && antisymmetry; j++)
            {
                if(!strcmp((*rows)[arg].relation.content[i].first, (*rows)[arg].relation.content[j].second) && !strcmp((*rows)[arg].relation.content[i].second, (*rows)[arg].relation.content[j].first) && strcmp((*rows)[arg].relation.content[i].first, (*rows)[arg].relation.content[i].second)) antisymmetry = false;
            }
            if(!antisymmetry) 
            {
                (*rows)[commandPos].outputValue = false;
                break;
            }
        }
        if(antisymmetry) (*rows)[commandPos].outputValue = true;
        return 1;
    }
    ERROR("There is no relation on this line", INVALID_ARGUMENT);
}

//returns true if relation is transitive, otherwise returns false
int relationIsTransitive(CommandProperties props, Row **rows)
{
    int arg = props.arg1-1;
    if(props.arg2 > 0) ERROR("There is no relation on this line", INVALID_ARGUMENT);
    int commandPos = props.arg3;
    bool transitivity;
    if((*rows)[arg].type == REL)
    {
        (*rows)[commandPos].type = OUT;
        for(int i = 0; i < (*rows)[arg].relation.contentSize; i++)
        {
            transitivity = true;
            for(int j = 0; j < (*rows)[arg].relation.contentSize; j++)
            {
                if(strcmp((*rows)[arg].relation.content[i].second, (*rows)[arg].relation.content[j].first) == 0)
                {
                    transitivity = false;
                    for(int k = 0; k < (*rows)[arg].relation.contentSize && !transitivity; k++)
                    {
                        if(!strcmp((*rows)[arg].relation.content[i].first, (*rows)[arg].relation.content[k].first) && !strcmp((*rows)[arg].relation.content[j].second, (*rows)[arg].relation.content[k].second)) transitivity = true;
                    }
                }
            }
            if(!transitivity)
            {
                (*rows)[commandPos].outputValue = false;
                break;
            }
        }
        if(transitivity) (*rows)[commandPos].outputValue = true;
        return 1;
    }
    ERROR("There is no relation on this line", INVALID_ARGUMENT);
}

//returns true if relation is a function, otherwise returns false
int relationIsFunction(CommandProperties props, Row **rows)
{
    int arg = props.arg1-1;
    if(props.arg2 > 0) ERROR("There is no relation on this line", INVALID_ARGUMENT);
    int commandPos = props.arg3;
    bool notFunction = false;
    if((*rows)[arg].type == REL)
    {
        (*rows)[commandPos].type = OUT;
        for(int i = 0; i < (*rows)[arg].relation.contentSize && !notFunction; i++)
        {
            for(int j = 0; j < (*rows)[arg].relation.contentSize && !notFunction; j++)
            {
                if(!strcmp((*rows)[arg].relation.content[i].first, (*rows)[arg].relation.content[j].first) && strcmp((*rows)[arg].relation.content[i].second, (*rows)[arg].relation.content[j].second))
                {
                    (*rows)[commandPos].outputValue = false;
                    notFunction = true;
                }
            }
        }
        if(!notFunction) (*rows)[commandPos].outputValue = true;
        return 1;
    }
    ERROR("There is no relation on this line", INVALID_ARGUMENT);
}

//returns the domain of the relation
Set findRelationDomain(Relation relation)
{
    Set domain;
    domain.length = 0;
    domain.content = (char **) malloc(relation.contentSize * sizeof(char *));
    for(int i = 0; i < relation.contentSize; i++)
    {
        if(!setContainsString(domain, relation.content[i].first))
        {
            domain.content[domain.length] = (char *) malloc(CHUNK * sizeof(char));
            strcpy(domain.content[domain.length], relation.content[i].first);
            domain.length++;
        }
    }
    return domain;
}



// returns the codomain of the relation
Set findRelationCodomain(Relation relation)
{
    Set codomain;
    codomain.length = 0;
    codomain.content = (char **) malloc(relation.contentSize * sizeof(char *));
    for(int i = 0; i < relation.contentSize; i++)
    {
        if(!setContainsString(codomain, relation.content[i].second))
        {
            codomain.content[codomain.length] = (char *) malloc(CHUNK * sizeof(char));
            strcpy(codomain.content[codomain.length], relation.content[i].second);
            codomain.length++;
        }
    }
    return codomain;
}

// command function that is used to print codomain of the relation
int printRelationCodomain(CommandProperties props, Row **rows)
{
    int arg = props.arg1-1;
    if(props.arg2 > 0) ERROR("There is no relation on this line", INVALID_ARGUMENT);
    int commandPos = props.arg3;
    if((*rows)[arg].type == REL)
    {
        Set codomain = findRelationCodomain((*rows)[arg].relation);

        (*rows)[commandPos].type = SET;
        (*rows)[commandPos].set.content = codomain.content;
        (*rows)[commandPos].set.length = codomain.length;
        return 1;
    }
    ERROR("There is no relation on this line", INVALID_ARGUMENT);
}

//returns true if relation is injective, otherwise returns false
int relationIsInjective(CommandProperties props, Row **rows)   // (Relation *relation, Set *setA, Set *setB)
{
    int arg1 = props.arg1-1;
    int arg2 = props.arg2-1;
    int arg3 = props.arg3-1;
    int commandPos = props.arg4;
    bool injectivity = true;
    if((*rows)[arg1].type == REL && (*rows)[arg2].type == SET && (*rows)[arg3].type == SET)
    {
        (*rows)[commandPos].type = OUT;
        for(int i = 0; i < (*rows)[arg1].relation.contentSize && injectivity; i++)
        {
            // printf("Iterace: %d\n",i);
            if(!setContainsString((*rows)[arg2].set, (*rows)[arg1].relation.content[i].first) || !setContainsString((*rows)[arg3].set, (*rows)[arg1].relation.content[i].second))
            {
                (*rows)[commandPos].outputValue = false;
                injectivity = false;
                break;
            }
            for(int j = i + 1; j < (*rows)[arg1].relation.contentSize; j++)
            {
                if(!strcmp((*rows)[arg1].relation.content[i].first, (*rows)[arg1].relation.content[j].first) || !strcmp((*rows)[arg1].relation.content[i].second, (*rows)[arg1].relation.content[j].second))
                {
                (*rows)[commandPos].outputValue = false;
                injectivity = false;
                break;
                }
            }
        }
        if(injectivity) (*rows)[commandPos].outputValue = true;
        return 1;
    }
    ERROR("There is no relation or set on this line", INVALID_ARGUMENT);
}

//returns true if relation is surjective, otherwise returns false
int relationIsSurjective(CommandProperties props, Row **rows)
{
    int arg1 = props.arg1-1;
    int arg2 = props.arg2-1;
    int arg3 = props.arg3-1;
    int commandPos = props.arg4;
    bool surjectivity = true;
    if((*rows)[arg1].type == REL && (*rows)[arg2].type == SET && (*rows)[arg3].type == SET)
    {
        (*rows)[commandPos].type = OUT;
        if(!setsEqual(findRelationCodomain((*rows)[arg1].relation), (*rows)[arg3].set))
        {
            (*rows)[commandPos].outputValue = false;
            surjectivity = false;
        }
        for(int i = 0; i < (*rows)[arg1].relation.contentSize && surjectivity; i++)
        {
            if(!setContainsString((*rows)[arg2].set, (*rows)[arg1].relation.content[i].first) || !setContainsString((*rows)[arg3].set, (*rows)[arg1].relation.content[i].second))
            {
            (*rows)[commandPos].outputValue = false;
            surjectivity = false;
            }
        }
        if(surjectivity) (*rows)[commandPos].outputValue = true;
        return 1;
    }
    ERROR("There is no relation on this line", INVALID_ARGUMENT);
}

//returns true if relation is injective (both injective and surjective), otherwise returns false
int relationIsBijective(CommandProperties props, Row **rows)
{
    int arg1 = props.arg1-1;
    int arg2 = props.arg2-1;
    int arg3 = props.arg3-1;
    int commandPos = props.arg4;
    bool bijectivity = true;
    if((*rows)[arg1].type == REL && (*rows)[arg2].type == SET && (*rows)[arg3].type == SET)
    {
        (*rows)[commandPos].type = OUT;
        for(int i = 0; i < (*rows)[arg1].relation.contentSize && bijectivity; i++)
        {
            if(!setContainsString((*rows)[arg2].set, (*rows)[arg1].relation.content[i].first) || !setContainsString((*rows)[arg3].set, (*rows)[arg1].relation.content[i].second))
            {
                (*rows)[commandPos].outputValue = false;
                bijectivity = false;
                break;
            }
            if(!setsEqual(findRelationCodomain((*rows)[arg1].relation), (*rows)[arg3].set))
            {
                (*rows)[commandPos].outputValue = false;
                bijectivity = false;
                break;
            }
            for(int j = i + 1; j < (*rows)[arg1].relation.contentSize; j++)
            {
                if(!strcmp((*rows)[arg1].relation.content[i].first, (*rows)[arg1].relation.content[j].first) || !strcmp((*rows)[arg1].relation.content[i].second, (*rows)[arg1].relation.content[j].second))
                {
                (*rows)[commandPos].outputValue = false;
                bijectivity = false;
                break;
                }
            }

        }
        if(bijectivity) (*rows)[commandPos].outputValue = true;
        return 1;
    }
    ERROR("There is no relation on this line", INVALID_ARGUMENT);
}

const Command commandList[NUMBER_OF_COMMANDS] = 
{
    {"empty", setIsEmpty, 1},
    {"complement", setComplement, 1},
    {"card", setCard, 1}, //dopsat vypis do konzole
    {"union", setUnion, 2},
    {"intersect", setIntersect, 2},
    {"minus", setMinus, 2},
    {"subseteq", setIsSubsetOrEq, 2},
    {"subset", setIsSubset, 2},
    {"equals", setEquals, 2},

    {"reflexive", relationIsReflexive, 1},
    {"symmetric", relationIsSymmetric, 1},
    {"antisymmetric", relationIsAntisymmetric, 1},
    {"transitive", relationIsTransitive, 1},
    {"function", relationIsFunction, 1},
    {"domain", printRelationDomain, 1},
    {"codomain", printRelationCodomain, 1},
    {"injective", relationIsInjective, 3},
    {"surjective", relationIsSurjective, 3},
    {"bijective", relationIsBijective, 3}
};
// executes the commands utilizing function pointers 
// loops through the const array where the func pointers are stored and executes the command which needs to be executed
// returns the received value from the called function when something went wrong
// returns 1 if everything went ok
int executeCommands(Row **rows, int *rowsCount, int *allocatedRowsCount)
{
    for(int i = 0; i < rowsCount; i++)
    {
            for(int j = 0; j < NUMBER_OF_COMMANDS; j++)
            {
                if((*rows)[i].type == COMMAND)
                {
                    if(!strcmp(commandList[j].name, (*rows)[i].command.name))
                    {
                        if(commandList[j].argc <= 2) (*rows)[i].command.arg3 = i;
                        else (*rows)[i].command.arg4 = i;
                        int recVal = commandList[j].func((*rows)[i].command, rows);
                        if(recVal != 1)
                            return recVal;
                    }
                }
            }
    }
    return 1;
}
int checkSet(Set set)
{
    for(int i = 0; i < NUMBER_OF_COMMANDS; i++)
    {
        if(setContainsString(set, commandList[i].name))
        {
            ERROR("Invalid argument", INVALID_ARGUMENT);
        }
    }
    if(setContainsString(set, "true") || setContainsString(set, "false"))
        ERROR("Invalid argument", INVALID_ARGUMENT);

    return 1;
}
int checkSetElementLen(Set set)
{
    for(int i = 0; i < set.length; i++)
    {
        if(strlen(set.content[i]) > MAX_ELEMENT_LENGTH)
            ERROR("Element too long", INVALID_ARGUMENT);
    }
    return checkSet(set);
}
int isSet(Set set)
{
    for(int i = 0; i < set.length; i++)
    {
        for(int j = (i+1); j < set.length; j++)
        {
            if(!strcmp(set.content[j], set.content[i]))
                ERROR("Not a set", INVALID_ARGUMENT);
        }
    }
    return checkSetElementLen(set);
}
int setSubsetOfUni(Set uni, Set set)
{
    for(int i = 0; i < set.length; i++)
    {
        if(!setContainsString(uni, set.content[i]))
            ERROR("Set is not subset of universe", INVALID_ARGUMENT);
    }
    return isSet(set);
}

int checkRel(Relation rel)
{
    for(int i = 0; i < rel.contentSize; i++)
    {
        for(int j = 0; i < MAX_NUMBER_OF_ROWS; j++)
        {
            if(!strcmp(commandList[j].name, rel.content->first) || !strcmp(commandList[j].name, rel.content->second))
                ERROR("Relation cannot contain command identificator", INVALID_ARGUMENT);
        }
    }
    if(!strcmp(rel.content->first, "true")||!strcmp(rel.content->first, "false"))
        ERROR("Relation cannot contain function output", INVALID_ARGUMENT);

    if(!strcmp(rel.content->second, "true")||!strcmp(rel.content->second, "false"))
        ERROR("Relation cannot contain function output", INVALID_ARGUMENT);

    return 1;
}
int relContainsOnlyUniElements(Set uni, Relation rel)
{
    for(int i = 0; i < rel.contentSize; i++)
    {
        if(setContainsString(uni, rel.content->first) && setContainsString(uni, rel.content->first))
            ERROR("All elements of relation must be from defined universe", INVALID_ARGUMENT);
    }
    return checkRel(rel);
}
int checkRows(Row *rows, int rowCount)
{
    if(rows[0].type != UNI)
    {
        ERROR("The first line is not defined as a universe", INVALID_ARGUMENT);
    }
    for(int i = 0; i < rowCount; i++)
    {
        if(rows[i].type == SET || rows[i].type == UNI)
        {
            int retVal = setSubsetOfUni(rows[0].set,rows[i].set);
            if(retVal != 1)
                return retVal;
        }
        if(rows[i].type == REL)
        {
            int retVal = relContainsOnlyUniElements(rows[0].set, rows[i].relation);
            if(retVal != 1)
                return retVal;
        }
    }
    return 1;
}

// loads the sets from file and further parses them by calling the parseType function
// returns FILE_TOO_LONG if there are more than 1000 lines in the file
// returns EMPTY_FILE if there are no lines in the file
// returns 1 if everything went ok
int loadSetsFromFile(Row **rows, FILE *fileptr, int *rowsCount, int *allocatedRowsCount)
{
    int lineCounter = 0;
    int allocedLineLen = MAX_ELEMENT_LENGTH;
    int usedLineLen = 0;
    char buffer[MAX_ELEMENT_LENGTH];
    char *line = (char *)malloc(MAX_ELEMENT_LENGTH);
    while (fgets(buffer, MAX_ELEMENT_LENGTH, fileptr))
    {
        if(allocedLineLen <= (usedLineLen+1))
        {
            allocedLineLen += MAX_ELEMENT_LENGTH;
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
            line = (char *)malloc(MAX_ELEMENT_LENGTH);
            lineCounter++;
            allocedLineLen = MAX_ELEMENT_LENGTH;
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
    return checkRows(*rows, *rowsCount);
}

int main(int argc, char **argv)
{

    Row *rows = (Row *)malloc(ROWS_TO_ALLOCATE * sizeof(Row));
    int allocatedRowsCount = ROWS_TO_ALLOCATE;
    int rowsCount = 0;
    char *filename = {'\0'};
    int retVal = 0;
    FILE *file = NULL;

    if (parseArgs(argc, argv, &filename) == NOT_ENOUGH_ARGUMENTS)
        return NOT_ENOUGH_ARGUMENTS;

    if (openFile(&file, filename) == CANNOT_OPEN_FILE)
        return CANNOT_OPEN_FILE;

    if (loadSetsFromFile(&rows, file, &rowsCount, &allocatedRowsCount) != 1)
        return EMPTY_FILE;
    retVal = executeCommands(&rows, rowsCount);
    if( retVal == 1)
    {
        print(rows, rowsCount);
    }
    else
    {
        freeAll(rows, rowsCount);
        fclose(file);
        return retVal;
    }    
    
    freeAll(rows, rowsCount);
    fclose(file);
    return 0;
}
