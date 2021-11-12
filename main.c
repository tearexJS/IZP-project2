#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct Set{
    char setType;
    int length;
    char** content;
};

void AddIntoSet(struct Set *set, char content[]){
    set->length++; // increase length of set
    char **tmp = set->content; // copy original content into a temporary variable


    set->content = malloc(1 * sizeof(*set->content));
    if (set->content)
    {
        char** tmp2 = realloc(set->content, 2 * sizeof(*set->content)); // creating a new array with bigger size
        if (tmp2) // if it was actually created (memory allocated)
        {
            set->content = tmp2;
            for(int i = 0; i < set->length-1; i++){ // copy all from the previous array
                set->content[i]=tmp[i];
            }
//            char nContent[101];
//            for(int i = 0; i < 100 && content[i]!='\0'; i++){
//                nContent[i] = content[i];
//                nContent[i+1] = '\0';
//            }
            set->content[set->length-1] = content; // finally the new value
        }
    }
    // found at https://stackoverflow.com/questions/12917727/resizing-an-array-in-c, edited for my purpose
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
        printf ("%s ", set->content[i]);
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

int main()
{
    struct Set sets[1000];
    int setsCount = 0;
    char *inputFileName = "sets.txt";
    FILE *file;
    file = fopen(inputFileName, "r");
    if(file != NULL){
        struct Set now;
        bool named = false;
        char s[100];
        char ch = ' '; // will be space ' ' after each word, or '\n' after line ... for recognizing line endings
        while(fscanf(file, "%99s%c", &s, &ch) > 0){
            printf("Prvni slovo souboru je '%s'\n", s);
            if(!named){
                sets[setsCount].setType = s[0];
                sets[setsCount].length = 0;
                named = true;
            }
            else{
                AddIntoSet(&sets[setsCount],s); // PROBLEM: s changes every time, so if s is added, it is changed the next iteration. in the end, the last s is everywhere
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
