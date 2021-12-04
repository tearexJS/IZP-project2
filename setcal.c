#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_NUMBER_OF_ROWS 1000
#define ROWS_TO_ALLOCATE 3
#define CHUNK 30

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
enum State
{
    waitForLParentheses,
    readFirstElement,
    waitForSecondElement,
    readSecondElement,
    waitForRParentheses
};
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
        CommandProperties command;
    };
    char *line;
} Row;
const Command commandList[9] = 
{
    {"empty", empty, 1},
    {"complement", complement, 1},
    {"card", card, 1},
    {"union", unionSets, 2},
    {"intersect", intersect, 2},
    {"minus", minus, 2},
    {"subseteq", subseteq, 2},
    {"subset", subset, 2},
    {"equals", equals, 2}

};
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

void printSetContent(char **content, int length)
{
    for(int i = 0; i < length; i++)
    {
        printf("%s ", content[i]);
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
            free(rows[i].relation.content);
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
void replaceSpaceWithZero(char *line)
{
    for(int i = 0; line[i] != '\0'; i++)
    {
        if(line[i] == ' ' || line[i] == ')')
            line[i] = '\0';
    }
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
    
    enum State state = waitForLParentheses; 
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
    for(int i = 1; line[i]!='\0'; i++)
    {
        if(line[i] == ' ' && isalpha(line[i+1]))
            row->command.name = &line[i+1];
        else if (line[i] == ' ' && isdigit(line[i+1]))
        {
            if(!row->command.arg1)
                row->command.arg1 = &line[i+1];
            else if (!row->command.arg2)
            {
                row->command.arg2 = &line[i+2];
            }
            else
                ERROR("Too many args", TOO_MANY_ARGUMENTS)
        }
        
    }
    replaceSpaceWithZero(line);
    return 1;
}
// setting the type of row to universe, set, relation or command 
// if there is something else than the mentioned returns INVALID_ARGUMENT
// TODO: implement command parsing and function pointers
// int parseCommand()
// {

// }
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
        if(!strcmp(set->content[i],str))
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

//commands for relations
//returns true if relation is reflexive, otherwise returns false
bool isRelationReflexive(int universeLength, Relation *relation)
{
    int matchCount = 0;
    for(int i = 0; i < relation->contentSize; i++)
    {
        if(!strcmp(relation->content[i].first, relation->content[i].second)) matchCount++;
    }
    if(matchCount == universeLength) return true;
    else return false;
}

//returns true if relation is symmetric, otherwise returns false
bool isRelationSymmetric(Relation *relation)
{
    bool symmetry;
    for(int i = 0; i < relation->contentSize; i++)
    {
        symmetry = false;
        for(int j = 0; j < relation->contentSize && !symmetry; j++)
        {
            if(!strcmp(relation->content[i].first, relation->content[j].second) && !strcmp(relation->content[i].second, relation->content[j].first)) symmetry = true;
        }
        if(!symmetry) return false;
    }
    return true;
}

//returns true if relation is antisymmetric, otherwise returns false
bool isRelationAntisymmetric(Relation *relation)
{
    bool antisymmetry;
    for(int i = 0; i < relation->contentSize; i++)
    {
        antisymmetry = true;
        for(int j = 0; j < relation->contentSize && antisymmetry; j++)
        {
            if(!strcmp(relation->content[i].first, relation->content[j].second) && !strcmp(relation->content[i].second, relation->content[j].first) && strcmp(relation->content[i].first, relation->content[i].second)) antisymmetry = false;
        }
        if(!antisymmetry) return false;
    }
    return true;
}

//returns true if relation is transitive, otherwise returns false
bool isRelationTransitive(Relation *relation)
{   
    bool transitivity;
    for(int i = 0; i < relation->contentSize; i++)
    {
        transitivity = true;
        for(int j = 0; j < relation->contentSize; j++)
        {
            if(strcmp(relation->content[i].second, relation->content[j].first) == 0)
            {
                transitivity = false;
                for(int k = 0; k < relation->contentSize && !transitivity; k++)
                {
                    if(!strcmp(relation->content[i].first, relation->content[k].first) && !strcmp(relation->content[j].second, relation->content[k].second)) transitivity = true;
                }
            }
        }
        if(!transitivity) return false;
    }
    return true;
}

//returns true if relation is a function, otherwise returns false
bool isRelationFunction(Relation *relation)
{
    for(int i = 0; i < relation->contentSize; i++)
    {
        for(int j = 0; j < relation->contentSize; j++)
        {
            if(!strcmp(relation->content[i].first, relation->content[j].first) && strcmp(relation->content[i].second, relation->content[j].second)) return false;
        }
    }
    return true;
}

//returns the domain of the relation
Set *findRelationDomain(Relation *relation)
{
    Set *domain = (Set *) malloc(sizeof(Set));
    domain->length = 0;
    domain->content = (char **) malloc(relation->contentSize * sizeof(char *));
    for(int i = 0; i < relation->contentSize; i++)
    {
        if(!setContainsString(domain, relation->content[i].first))
        {
            domain->content[domain->length] = (char *) malloc(CHUNK * sizeof(char));
            strcpy(domain->content[domain->length], relation->content[i].first);
            domain->length++;
        }
    }
    return domain;
}

//returns the codomain of the relation
Set *findRelationCodomain(Relation *relation)
{
    Set *codomain = (Set *) malloc(sizeof(Set));
    codomain->length = 0;
    codomain->content = (char **) malloc(relation->contentSize * sizeof(char *));
    for(int i = 0; i < relation->contentSize; i++)
    {
        if(!setContainsString(codomain, relation->content[i].second))
        {
            codomain->content[codomain->length] = (char *) malloc(CHUNK * sizeof(char));
            strcpy(codomain->content[codomain->length], relation->content[i].second);
            codomain->length++;
        }
    }
    return codomain;
}

//returns true if relation is injective, otherwise returns false
bool isRelationInjective(Relation *relation, Set *setA, Set *setB)
{
    for(int i = 0; i < relation->contentSize; i++)
    {
        if(!setContainsString(setA, relation->content[i].first) || !setContainsString(setB, relation->content[i].second)) return false;
        for(int j = i + 1; j < relation->contentSize; j++)
        {
            if(!strcmp(relation->content[i].first, relation->content[j].first) || !strcmp(relation->content[i].second, relation->content[j].second)) return false;
        }
    }
    return true;
}

//returns true if relation is surjective, otherwise returns false
bool isRelationSurjective(Relation *relation, Set *setA, Set *setB)
{
    if(!setEquals(findRelationCodomain(relation), setB)) return false;
    for(int i = 0; i < relation->contentSize; i++)
    {
        if(!setContainsString(setA, relation->content[i].first) || !setContainsString(setB, relation->content[i].second))
        {
            return false;
        }
    }
    return true;
}

//returns true if relation is injective (both injective and surjective), otherwise returns false
bool isRelationBijective(Relation *relation, Set *setA, Set *setB)
{
    if(isRelationInjective(relation, setA, setB) && isRelationSurjective(relation, setA, setB)) return true;
    return false;
}

int main(int argc, char **argv)
{
    char *randomInput[] = {"ahoj", "prd", "test", "nevimnecovelmidlouheho"};
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

    if (loadSetsFromFile(&rows, file, &rowsCount, &allocatedRowsCount) != 1)
        return EMPTY_FILE;

    print(rows, rowsCount);
    freeAll(rows, rowsCount);
    fclose(file);
    return 0;
}
