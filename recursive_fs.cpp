#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <string.h> 
#include <iomanip>
#include <iostream>

using namespace std;   

void recursive_walk(const string& dirname, int indent, int concatinate) {
    cout << right << setw(indent + dirname.substr(concatinate, dirname.size()).size()) << setfill('-') << dirname.substr(concatinate, dirname.size()) << "\n"; 
    DIR* dir_fd = opendir(dirname.c_str()); 
    if(dir_fd == nullptr) {
        cerr << " problems " << "\n";
        return; 
    } 
    struct dirent* entry;  
    indent++; 
    while((entry = readdir(dir_fd)) != NULL) { 
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; 
        }  
        if(entry->d_type == DT_DIR) {
            recursive_walk(dirname + "/" + entry->d_name, indent, dirname.size() + 1); 
        } else {
            cout << right << setw(indent + strlen(entry->d_name)) << setfill('-') << entry->d_name << "\n";
        }
    }  
    closedir(dir_fd);
} 

int main(int argc, char** argv) { 

    const string dirname = argc > 1 ? argv[1] : "."; 
    recursive_walk(dirname, 0, 0);  

    return 0;
}