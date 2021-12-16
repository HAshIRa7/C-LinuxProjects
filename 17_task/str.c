#include <stdio.h> 
#include <string.h> 

int main(int argc, char** argv) { 
    char str[256]; 
    snprintf(str, sizeof(argv[1]), "/%s", argv[1]); 
    printf("%s\n", str);
    return 0;
}