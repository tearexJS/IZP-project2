#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct Set{
    char setType;
    int length;
    char** content;
    int contentSize;
};

void AddIntoSet(struct Set *set, char content[]){
    printf("Adding into set %s: length=%d, contentSize=%d\n", content, set->length, set->contentSize);
        PrintSet(set);
    set->length++; // increase length of set
    if(set->length == 1){
        set->contentSize=1;
//        set->content = (char**)malloc(sizeof(char[100]));
    }
    char **tmp = set->content; // copy original content into a temporary variable
//    tmp = malloc(1 * sizeof(*set->content));
//    for(int i = 0; i < set->length-1; i++){
//        tmp[i]=set->content[i];
//    }

//    printf("Size of content: %d\n",sizeof(set->content));

//    int contentSize = sizeof(content)/sizeof(content[0]);
//    const char contentConst[contentSize];
//    strcpy(contentConst,content);
//    char contentNew[contentSize];
//    strcpy(contentNew, contentConst);
//    for (int i = 0; i < contentSize; i++){
//        contentNew[i]=content[i];
//    }
//    char string2[50];
//  const char *end = "the end";
//  strcpy(string2, content);
//  strcat(string2, end);
//  contentNew = end;

    set->content = malloc(1 * sizeof(*set->content));
    if (set->content)
    {
        bool needToResize = true;
        if(set->contentSize <= set->length)
            needToResize = true;
        char** tmp2 = needToResize ? realloc(set->content, 2 * sizeof(*set->content)) : set->content; // creating a new array with bigger size
        if (tmp2) // if it was actually created (memory allocated)
        {
            if(needToResize) set->contentSize *= 2;
            printf("Created tmp2 while adding %s: length=%d, contentSize=%d\n", content, set->length, set->contentSize);
        PrintSet(set);
            set->content = tmp2;
            for(int i = 0; i < set->length-1; i++){ // copy all from the previous array
                set->content[i]=tmp[i];
//                printf("Content item n. %d is %s\n", i, set->content[i]);
            }
//            char nContent[101];
//            for(int i = 0; i < 100 && content[i]!='\0'; i++){
//                nContent[i] = content[i];
//                nContent[i+1] = '\0';
//            }
            set->content[set->length-1] = content; // finally the new value
//            free(tmp2);
            printf("Added into set %s: length=%d, contentSize=%d\n", content, set->length, set->contentSize);
        PrintSet(set);
        }
//        free(tmp);
        printf("Freed tmp while adding %s: length=%d, contentSize=%d\n", content, set->length, set->contentSize);
        PrintSet(set);
    }
    // found at https://stackoverflow.com/questions/12917727/resizing-an-array-in-c, edited for my purpose

    PrintSet(set);
}

bool CompareStrings(char a[], char b[]){
    int i = 0;
    while(a[i] != '\0' && b[i] != '\0'){
        if(a[i] != b[i])
            return false;
        i++;
    }
    if(a[i] == '\0' && b[i] == '\0')
        return true;
    return false;
}

bool IsInSet(char *content, struct Set *set){
    for(int i = 0; i < set->length; i++){
        if(CompareStrings(content, set->content[i]))
            return true;
    }
    return false;
}

void RemoveFromSet(struct Set *set, char *content){
    if(!IsInSet(content, set)){
        return;
    }
    set->length--;
    int rem = 0;
    for(int i = 0; i < set->length+rem; i++){
        if(i == set->length)
        {
            set->content[i] = ""; // set the remaining last values (which were moved to the left) to ""
        }
        if(i < set->length){
            if(CompareStrings(content, set->content[i]))
                rem++;
            if(rem > 0)
                set->content[i] = set->content[i+rem]; // PROBLEM: not reducing the size of the array, only removing the member and reducing length variable
        }
    }
}

void PrintSet(struct Set *set){
    printf("%c ", set->setType);
    for(int i = 0; i < set->length; i++){
        char* now = set->content[i];
        printf ("%s ", now);
    }
    printf("\n");
}

bool IsSubset(struct Set *smaller, struct Set* bigger){
    for(int i = 0; i < smaller->length; i++){
        bool isThere = false;
        for(int j = 0; j < bigger->length; j++){
            if(CompareStrings(smaller->content[i], bigger->content[j])){
                isThere=true;
                break;
            }
        }
        if(!isThere)
            return false;
    }
    return true;
}

bool AreThereDupliciteMembersInSet(struct Set *set){
    for(int i = 0; i < set->length; i++){
        for(int j = i+1; j < set->length; j++){
            if(CompareStrings(set->content[i], set->content[j]))
                return true;
        }
    }
    return false;
}

bool IsSet(struct Set *set){
    if(AreThereDupliciteMembersInSet(set))
        return false;

    return true;
}

// check if the set is empty
bool IsEmpty(struct Set *s){
    return false;
}

// Print the number of members in the set
int Card(struct Set *s){
    return s->length;
}

void Complement(struct Set *ret, struct Set *s, struct Set *u){
    ret->length=0;
    ret->setType='S';
    for(int i = 0; i < s->length; i++){
        if(!IsInSet(s->content[i], u))
            AddIntoSet(ret, s->content[i]);
    }
}

int main()
{
    struct Set sets[1000];
    int setsCount = 0;
    char *inputFileName = "sets.txt";
    FILE *file;
    file = fopen(inputFileName, "r");
    if(file != NULL){
        bool named = false;
        char s[100];
        char ch = ' '; // will be space ' ' after each word, or '\n' after line ... for recognizing line endings
        while(fscanf(file, "%99s%c", &s, &ch) > 0){
            printf("Ctu ze souboru '%s'\n", s);
            if(!named){
                sets[setsCount].setType = s[0];
                sets[setsCount].length = 0;
                named = true;
            }
            else{
                if(s[0]=='x')  {
                    char test = 32;
                }
                printf("Gonna malloc s: %s: length=%d, contentSize=%d\n", s, sets[setsCount].length, sets[setsCount].contentSize);
        PrintSet(&sets[setsCount]);
                char *ns = malloc(strlen(s)+1);
                printf("Malloced s: %s: length=%d, contentSize=%d\n", s, sets[setsCount].length, sets[setsCount].contentSize);
        PrintSet(&sets[setsCount]);
                strcpy(ns,s);
                printf("Copied s: %s: length=%d, contentSize=%d\n", ns, sets[setsCount].length, sets[setsCount].contentSize);
        PrintSet(&sets[setsCount]);
                AddIntoSet(&sets[setsCount],ns); // PROBLEM: s changes every time, so if s is added, it is changed the next iteration. in the end, the last s is everywhere
//                free(ns);
            }
            if(ch == '\n') {

                printf("END OF LINE!\n");
                setsCount++;
                named = false;
            }
        }
        setsCount++;
        fclose(file);
    }

    for(int i = 0; i < setsCount; i++){
        PrintSet(&sets[i]);
    }
    return 0;


    struct Set uni;
    uni.setType = 'U';
    uni.length = 0;

    struct Set set;
    set.setType = 'S';
    set.length=0;

    AddIntoSet(&uni, "Ahoj");
    AddIntoSet(&uni, "Ondro");
    AddIntoSet(&uni, "Jak");
    AddIntoSet(&uni, "Je");


    AddIntoSet(&set, "Ahoj");
    AddIntoSet(&set, "Ondro");


    if(IsInSet("Ondro", &uni))
        printf("Is in set!\n");
    else
        printf("Is not in set!\n");

    if(IsSubset(&set, &uni))
        printf("Is subset!\n");
    else
        printf("Is not subset!\n");

    AddIntoSet(&set, "Novy clen");
    PrintSet(&set);
    if(IsSubset(&set, &uni))
        printf("Is subset!\n");
    else
        printf("Is not subset!\n");

    RemoveFromSet(&set, "Novy clen");
    PrintSet(&set);
    if(IsSubset(&set, &uni))
        printf("Is subset!\n");
    else
        printf("Is not subset!\n");


    return 0;
}
