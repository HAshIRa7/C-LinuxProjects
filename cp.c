#define _GNU_SOURCE
#include <stdio.h>  
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>  
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <fcntl.h>  
#include <dirent.h>    
// все происходит в рамках содержимого getcwd(cur_dir, buf) директории 
//копирование из файла src_name в dst_name 
//копирование FIFO из src_name в dst_name  
// копирование Dev из src_name в dst_name 
// копирование директорие в поддиректорию 
// реализовано это рекурсивно  

// есть два варианта dst_name exist щк

#define BLOCK_SIZE 4096 * 256 
#define TRUE 1 
#define FALSE 0

extern int errno ; 

int copy_regular_file(const char* src_name, const char* dst_name, ssize_t src_size); //done 
int cpy_regular(int src_fd, int  dst_fd, ssize_t src_size); 
int copy_fifo(const char* dst_name, mode_t src_mode); // done
int cpy_fifo(int dstdir_fd, const char* dst_name, mode_t src_mode);
int copy_dir(int src_fd,int dst_fd);  
int copy_symlink(const char* src, const char* dst, ssize_t src_size); //done 
int cpy_symlink(int srcdir_fd, int dstdir_fd, const char* src_name, const char* dst_name, ssize_t src_size);
int copy_device(const char* dst_name, dev_t src_dev_id, mode_t src_dev_mode);// done
int cpy_device(int dstdir_fd, const char* dst_name, const struct stat* dst_st);
int relocate_block(int from_fd, int to_fd, u_int8_t* buf, ssize_t bl_size); //done 
int Copy(const char* src_name,const char* dst_name, struct stat* src_st, struct stat* dst_st, int exist);  
int ComparissonDirectories(const char* srcdir_name, const char* dst_name); 
char* get_last(const char* full_name, size_t length); 
//int Remove_Directory(int dir_fd);

int main(int argc, char** argv) {  
    struct stat src_st;  
    struct stat dst_st;   
    int exist = TRUE; // есть такой файл или нет, потому что если нет то все ок, а если есть cp забивает на него remove(file)
    int code = 0;
    if(argc != 3) {
        perror("Few arguments for !\n"); 
        return EXIT_FAILURE;  
    }   
    if(lstat(argv[1], &src_st) < 0) {
        perror("lstat source file error\n");
        return EXIT_FAILURE; 
    }   

    if(lstat(argv[2], &dst_st) < 0) {
        if(errno == ENOENT) {
            exist = FALSE;  
        } else {
            perror("lstat destination file error\n");
            return EXIT_FAILURE;
        }
    }
    code = Copy(argv[1], argv[2], &src_st, &dst_st, exist);
    return code;
} 

int Copy(const char* src_name, const char* dst_name, struct stat* src_st, struct stat* dst_st, int exist) { 
    if(exist == 1) { 
        printf("HEllo");
        // проверка на вложенность 
        if((dst_st->st_mode & __S_IFMT) == __S_IFDIR && (src_st->st_mode & S_IFMT) == __S_IFDIR && ComparissonDirectories(src_name, dst_name) == FALSE) { 
            return 7;
        }  
        // провекра на соотнесение типов 
        if(((dst_st->st_mode & __S_IFMT) != (src_st->st_mode & __S_IFMT) && (dst_st->st_mode & __S_IFMT) != __S_IFDIR)) {
            return 7;
        } 
        if((dst_st->st_mode & __S_IFMT) !=__S_IFDIR) {
            remove(dst_name);
        }
    } 
    int src_fd, dst_fd, dst1_fd; 
    const char* res; 
    switch(src_st->st_mode & S_IFMT) {
        case S_IFBLK:  return copy_device(dst_name, src_st->st_rdev, src_st->st_mode); 
        case S_IFDIR:  
            src_fd = open(src_name, O_DIRECTORY);  
            if(src_fd < 0) {
                perror("Can't open source directory directory\n"); 
                return 2; 
            }
            if(exist == FALSE) {
                if(mkdir(dst_name, 0700) < 0) {
                    perror("Can't create directory\n");
                    return 2; 
                } 
            }
            dst_fd = open(dst_name, O_DIRECTORY);
            if(dst_fd < 0) {
                perror("Can't open destination directory\n"); 
                return 2; 
            }  
            res = get_last(src_name, strlen(src_name));
            printf("%s\n", res); 
            if(mkdirat(dst_fd, res, 0700) < 0) {
                perror("Can't create directory\n"); 
                return 2; 
            }  
            dst1_fd = openat(dst_fd, res, O_DIRECTORY); 
            if(dst1_fd < 0) {
                perror("Can't open source file");
                return 2;
            }
            int code = copy_dir(src_fd, dst1_fd); 
            if(close(src_fd) < 0) {
                perror("Can't close directory\n");
                return 2; 
            } 
            if(close(dst_fd) < 0) {
                perror("Can't close directory\n");
                return 2; 
            } 
            if(close(dst1_fd) < 0) {
                perror("Can't close directory\n");
                return 2; 
            }
            return code;
        case S_IFIFO:  return copy_fifo(dst_name, src_st->st_mode);
        case S_IFLNK:  return copy_symlink(src_name, dst_name, (ssize_t)src_st->st_size);
        case S_IFREG:  return copy_regular_file(src_name, dst_name, (ssize_t)src_st->st_size);
        default:       perror("can't sense this file in my cp version\n"); return 6;
    }
}  

int copy_device(const char* dst_name, dev_t src_dev_id, mode_t src_dev_mode) {
    int code = mknod(dst_name, src_dev_mode, src_dev_id); 
    if(code < 0) {
        perror("mknod\n"); 
        code = 1;  
    }
    return code; 
} 

int copy_symlink(const char* src_name, const char* dst_name, ssize_t src_size) {
    int code = 0; 
    char* target_path = malloc(src_size * sizeof(char) + 1);  
    if(target_path == NULL) {
        perror("malloc\n"); 
        code = 4; 
        return code; 
    } 
    ssize_t len = readlink(src_name, target_path, src_size); 
    target_path[len] = '\0'; 
    if(len != src_size) {
        perror("readlink\n");  
        code = 4;
    } else {
        int res = symlink(target_path, dst_name); 
        if(res == -1) {
            perror("symlink\n"); 
            code = 4; 
        } 
    }
    free(target_path); 
    return code;   
}   

int copy_fifo(const char* dst_name, mode_t src_mode) {
    int res = mkfifo(dst_name, src_mode); 
    if(res < 0) {
        perror("Error, can't create FIFO!\n"); 
        res = 3;
    }
    return res;
}  

int relocate_block(int from_fd, int to_fd, u_int8_t* buf, ssize_t bl_size)  {
    ssize_t nbytes_read = read(from_fd, buf, bl_size); 
    if(nbytes_read != bl_size) {
        perror("can't read source file\n");
        return 5;
    } 

    ssize_t nbytes_written = 0; 
    while(nbytes_written < bl_size) {
        ssize_t plus = write(to_fd, buf + nbytes_written, bl_size - nbytes_written); 
        if(plus < 0) {
            return 5;
        }
        nbytes_written += plus; 
    } 
    return 0;
} 

int copy_regular_file(const char* src_name, const char* dst_name, ssize_t src_size) {
    int code = 0;
    int src_fd = open(src_name, O_RDONLY); 
    if(src_fd < 0) {
        perror("open\n");
        return 2;
    } 
    int dst_fd = open(dst_name, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if(dst_fd < 0) {
        perror("open\n");
        close(src_fd);
        return 2;
    } 
    ssize_t left_bytes = src_size; 
    if(left_bytes > BLOCK_SIZE) {
        u_int8_t* buf = malloc(BLOCK_SIZE); 
        if(buf == NULL) {
            perror("malloc\n");
            return 2;
        }  
        while(left_bytes > BLOCK_SIZE){ 
            code = relocate_block(src_fd, dst_fd, buf, BLOCK_SIZE);
            if(code != 0) {
                return 3;
            }
            left_bytes -= BLOCK_SIZE;
        }  
        code = relocate_block(src_fd, dst_fd, buf, left_bytes);  
    } else {
        u_int8_t* buf = malloc(src_size); 
        if(buf == NULL) {
            perror("malloc\n");
            return 2; 
        } 
        code = relocate_block(src_fd, dst_fd, buf, left_bytes); 
        free(buf);  
    }
    if(close(src_fd) == -1) {
        perror("close source file failed\n");
        return 2; 
    } 
    if(close(dst_fd) == -1) {
        perror("close destination destination file failed\n");
        return 2; 
    }
    return code; 
}   

int cpy_symlink
(
    int srcdir_fd, int dstdir_fd, 
    const char* src_name, const char* dst_name, 
    ssize_t src_size
)
{
    int code = 0;
    char* buf = malloc(src_size);
    if (src_size != readlinkat(srcdir_fd, src_name, buf, src_size))
        {
            printf("Failed to read data from source %s\n", src_name);
            code = 2;
        }
    else
    {
        if (symlinkat(buf, dstdir_fd, dst_name) != 0)
        {
        perror("Error during creating a symlink\n");
        code = 2;
        }
    }
    free(buf);
    return code;
} 

int cpy_device(int dstdir_fd, const char* dst_name, const struct stat* dst_st)
{
    int code = mknodat(dstdir_fd, dst_name, dst_st->st_mode, dst_st->st_dev);
    if (code != 0) {
        perror("Error while copying device file\n");
        code = 2; 
    }
    return code;
}  

int cpy_fifo(int dstdir_fd, const char* dst_name, mode_t src_mode)
{
    int code = mkfifoat(dstdir_fd, dst_name, src_mode);
    if (code != 0) {
        perror("Error during creating FIFO\n");
        return 2;
    }
    return code;
}


int cpy_regular(int src_fd, int  dst_fd, ssize_t src_size)
{
    int code;
    ssize_t size_left = src_size;
    if (size_left >= BLOCK_SIZE)
    {
        u_int8_t* buf = malloc(BLOCK_SIZE);
        while (size_left >= BLOCK_SIZE)
        {
            if (relocate_block(src_fd, dst_fd, buf, BLOCK_SIZE) != 0)
                return 3;
            size_left -= BLOCK_SIZE;
        }
        code = relocate_block(src_fd, dst_fd, buf, size_left);
        free(buf);
    }
    else
    {
        u_int8_t* buf = malloc(size_left);
        code = relocate_block(src_fd, dst_fd, buf, size_left);
        free(buf);
    }
   if(code != 0) {
       code = 2; 
   }
    return code;
}

int ComparissonDirectories(const char* srcdir_name, const char* dstdir_name) { // возвращает True когда нет вложенности 
    int res = FALSE;
    if(strlen(srcdir_name) <= strlen(dstdir_name)) {
        for(ssize_t i = 0; i < strlen(srcdir_name); i++) {
            if(srcdir_name[i] != dstdir_name[i]) {
                res = TRUE; 
                break;
            }
        }
    } else { 
        for(ssize_t i = 0; i < strlen(dstdir_name); i++) {
            if(srcdir_name[i] != dstdir_name[i]) {
                res = TRUE; 
                break;
            }
        }
    }
    return res;
}

int copy_dir(int srcdir_fd, int dstdir_fd) { 
    DIR* src = fdopendir(srcdir_fd);  
    if(src == NULL) {
        perror("opendir return error\n"); 
        return 2;
    } 
    struct dirent* cur_entry = NULL;  
    int code = 0; 
    int src_fd, dst_fd; //  наименования относительных путей (их дескрипторы) 
    while((cur_entry = readdir(src)) != NULL) {
        struct stat file_st; 
        const char* src_name = cur_entry->d_name; 
        const char* dst_name = cur_entry->d_name;
        if(fstatat(srcdir_fd, src_name, &file_st, AT_SYMLINK_NOFOLLOW) < 0) {
            perror("fstatat return error\n"); 
            return 2; 
        } 
        switch(file_st.st_mode & S_IFMT) 
        {
            case S_IFLNK:  
                code = cpy_symlink(srcdir_fd, dstdir_fd, src_name, dst_name, (ssize_t)file_st.st_size);
                break; 
            case S_IFREG: 
                src_fd = openat(srcdir_fd, src_name, O_RDONLY);
                if (src_fd < 0)
                {
                    perror("Failed to open the source");
                    return 2;
                }
                dst_fd = openat(dstdir_fd, dst_name, O_CREAT | O_TRUNC | O_WRONLY, 0600);
                if (dst_fd < 0)
                {
                    perror("Failed to open destination");
                    printf("\t%s at %i\n", dst_name, dstdir_fd);
                    close(src_fd);
                    return 2;
                }
                code = cpy_regular(src_fd, dst_fd, file_st.st_size);
                close(src_fd);
                if (close(dst_fd))
                {
                    perror("Error durind writing file\n");
                    code = 1;
                }
                break;
            case S_IFIFO: 
                code = cpy_fifo(dstdir_fd, dst_name, file_st.st_mode); 
                break; 
            case S_IFCHR:
                code = cpy_device(dstdir_fd, dst_name, &file_st); 
                break;
            case S_IFDIR: 
                if(strcmp(src_name, ".") == 0 || strcmp(src_name, "..") == 0) {
                    break;
                } 
                src_fd = openat(srcdir_fd, src_name, O_DIRECTORY); 
                if(src_fd < 0) {
                    perror("Can't open source durectiry\n");
                    return 2; 
                }  
                int res1 = mkdirat(dstdir_fd, dst_name, 0700);
                if(res1 < 0) {
                    perror("can't create this directory\n"); 
                    return 2;  
                }
                dst_fd = openat(dstdir_fd, dst_name, O_DIRECTORY);  
                if(dst_fd < 0) {
                    perror("Can't open destination directory\n");
                    return 2; 
                }
                code = copy_dir(src_fd, dst_fd);
                if(close(src_fd) < 0) {
                    perror("Can't close file\n");
                    return 2; 
                } 
                if(close(dst_fd) < 0) {
                    perror("Can't close file");
                    return 2; 
                }
        }
    } 
}  

char* get_last(const char* full_name, size_t length)
{
    length -= 1;
    if (full_name[length] == '/')
        length--;
    while((full_name[length] != '/') && length)
        length--;
    return full_name + length + 1;
}