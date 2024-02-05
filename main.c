#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define MAX_FILENAME_LENGTH 1000
#define MAX_COMMIT_MESSAGE_LENGTH 2000
#define MAX_LINE_LENGTH 1000
#define MAX_MESSAGE_LENGTH 1000
#define MAX_PATH_LENGTH 1000


#define debug(x) printf("%s", x);

int checkdir();
bool check_file_directory_exists(char *filepath);
int global_config(char *username, char *email);
int local_config(char *username, char *email);
int initialize(int argc, char* const argv[]);
int add(char *filepath);
int add_depth(char * dirname);
int reset(char *filepath);
int reset_redo();
int run_commit(int argc, char * const argv[]);
int inc_last_commit_ID() ;
bool check_file_directory_exists(char *filepath) ;
int commit_staged_file(int commit_ID, char* filepath) ;
int track_file(char *filepath) ;
bool is_tracked(char *filepath);
int create_commit_file(int commit_ID, char *message);
int find_file_last_commit(char* filepath) ;
int log_command(int argc, char * const argv[]);
int make_branch(int argc, char * const argv[]);
int run_checkout(int argc, char * const argv[]);
int find_file_last_change_before_commit(char *filepath, int commit_ID);
int checkout_file(char *filepath, int commit_ID);
int status(int argc, char * argv[]);
bool is_staged(char *filename);


int checkdir()
{
   char cwd[1024];
   if(getcwd(cwd,sizeof(cwd))==NULL) return 1;
   int check =0;
   while(strcmp(cwd,".neogit")!=0){
      if(chdir("..") != 0) return 1;
   }
}

bool check_file_directory_exists(char *filepath) {
    DIR *dir = opendir(".neogit/files");
    struct dirent *entry;
    if (dir == NULL) {
        perror("Error opening current directory");
        return 1;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, filepath) == 0) return true;
    }
    closedir(dir);

    return false;
}

int global_config(char *username, char *email)
{
   FILE *file = fopen("Project/global_config","w");
   if(file == NULL) return 1;
   fprintf(file,"username: %s\n",username);
   fprintf(file,"email: %s\n",email);
   fclose(file);
   return 0;
}


int local_config(char *username, char *email)
{
   FILE *file = fopen(".neogit/config","w");
   
    char line[1000];
    while (fgets(line, sizeof(line), file) != NULL) {
      int length = strlen(line);

      // remove '\n'
      if (length > 0 && line[length - 1] == '\n') {
         line[length - 1] = '\0';
      }
   }
   fprintf(file,"username: %s\n",username);
   fprintf(file,"email: %s\n",email);
   fprintf(file, "last_commit_ID: %d\n", 0);
   fprintf(file, "current_commit_ID: %d\n", 0);
   fprintf(file, "branch: %s", "master");
   fclose(file);
   // create commits folder
    if (mkdir(".neogit/commits", 0755) != 0) return 1;

    // create files folder
    if (mkdir(".neogit/files", 0755) != 0) return 1;

    // create branches folder
    if (mkdir(".neogit/branches", 0755) != 0) return 1;

    file = fopen(".neogit/staging", "w");
    fclose(file);

    file = fopen(".neogit/tracks", "w");
    fclose(file);

    return 0;
}


int initialize(int argc, char* const argv[])
{
    char cwd[MAX_FILENAME_LENGTH];
    if(getcwd(cwd, sizeof(cwd))==NULL) return 1;
    char tmp_cwd[MAX_FILENAME_LENGTH];
    int exists = 0;
    struct dirent *entry;

    //checking current and parent dirs
    do{
        DIR *dir = opendir(".");
        if(dir == NULL){
            perror("error oppening current directory\n");
            return 1;
        }
        // finding .neogit file
        while((entry = readdir(dir))!=NULL){
            if(entry->d_type == DT_DIR && (strcmp(entry->d_name,".neogit")==0))
                exists = 1;
        }
        closedir(dir);

        if(getcwd(tmp_cwd,sizeof(tmp_cwd)) == NULL) return 1;

        // change dir to parent
        if(strcmp(tmp_cwd,"/")!=0){
            if(chdir("..") != 0) return 1;
        }

    }while(strcmp(tmp_cwd,"/")!=0);
     
    if(chdir(cwd) != 0) 
        return 1;
    //make .neogit dir
     if(!exists){
        if(mkdir(".neogit",0755)) return 1;
     }
     else{
        perror("neogit repository has already initialized\n");
     }
}

int add(char *filepath)
{
   FILE *file = fopen(".neogit/staging","r");
   if(file == NULL) return 1;
   char line[1000];
   while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }

        if (strcmp(filepath, line) == 0) return 0;
    }
    fclose(file);
    
    file = fopen(".neogit/staging","a");
    if (file == NULL) return 1;

    fprintf(file, "%s\n", filepath);
    fclose(file);

    return 0;
   
}

int add_depth(char * dirname)
{
   FILE *file = fopen(".neogit/staging","r");
   char line[1000];
   struct dirent *entry;
   DIR *dir = opendir(dirname);
   int b=0; 
   while((entry = readdir(dir))!=NULL){
      while(fgets(line,sizeof(line),file)!=NULL){
         int length = strlen(line);
         if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
         }
         if(strcmp(line, entry->d_name)==0){
            printf("%s exists in staging area\n",entry->d_name);
            b=1;
         }
      }
      if(!b)
         printf("%s is not staged\n", entry->d_name);
   }
}

int reset(char *filepath)
{
   FILE *file = fopen(".neogit/staging", "r");
   if (file == NULL) return 1;
    
   FILE *tmp_file = fopen(".neogit/new_staging", "w");
   if (tmp_file == NULL) return 1;

   char line[MAX_LINE_LENGTH];
   while (fgets(line, sizeof(line), file) != NULL) {
      int length = strlen(line);

      // remove '\n'
      if (length > 0 && line[length - 1] == '\n') {
         line[length - 1] = '\0';
      }

      if (strcmp(filepath, line) != 0) 
         fputs(line, tmp_file);
   }
   fclose(file);
   fclose(tmp_file);

   remove(".neogit/staging");
   rename(".neogit/new_staging", ".neogit/staging");
   return 0;
}

int reset_redo()
{
   FILE *file = fopen(".neogit/staging", "r");
   if (file == NULL) return 1;
    
   FILE *tmp_file = fopen(".neogit/new_staging", "w");
   if (tmp_file == NULL) return 1;

   char line[1000];
   int n=0;
   while (fgets(line, sizeof(line), file) != NULL) {
      n++;
   }
   int a=0;
   while (fgets(line, sizeof(line), file) != NULL) {
      int length = strlen(line);

      // remove '\n'
      if (length > 0 && line[length - 1] == '\n') {
         line[length - 1] = '\0';
      }
      if(a != n-1)
         fputs(line, tmp_file);
   }
   fclose(file);
   fclose(tmp_file);

   remove(".neogit/staging");
   rename(".neogit/new_staging", ".neogit/staging");
   return 0;
}

int run_commit(int argc, char * const argv[]) 
{
    if (argc < 4) {
        perror("please use the correct format");
        return 1;
    }
    
    char message[MAX_MESSAGE_LENGTH];
    strcpy(message, argv[3]);

    int commit_ID = inc_last_commit_ID();
    if (commit_ID == -1) return 1;
    
    FILE *file = fopen(".neogit/staging", "r");
    if (file == NULL) return 1;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        
        if (!check_file_directory_exists(line)) {
            char dir_path[MAX_FILENAME_LENGTH];
            strcpy(dir_path, ".neogit/files/");
            strcat(dir_path, line);
            if (mkdir(dir_path, 0755) != 0) return 1;
        }
        printf("commit %s\n", line);
        commit_staged_file(commit_ID, line);
        track_file(line);
    }
    fclose(file); 
    
    // free staging
    file = fopen(".neogit/staging", "w");
    if (file == NULL) return 1;
    fclose(file);

    create_commit_file(commit_ID, message);
    fprintf(stdout, "commit successfully with commit ID %d", commit_ID);
    
    return 0;
}

// returns new commit_ID
int inc_last_commit_ID() 
{
    FILE *file = fopen(".neogit/config", "r");
    if (file == NULL) return -1;
    
    FILE *tmp_file = fopen(".neogit/tmp_config", "w");
    if (tmp_file == NULL) return -1;

    int last_commit_ID;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "last_commit_ID", 14) == 0) {
            sscanf(line, "last_commit_ID: %d\n", &last_commit_ID);
            last_commit_ID++;
            fprintf(tmp_file, "last_commit_ID: %d\n", last_commit_ID);

        } else fprintf(tmp_file, "%s", line);
    }
    fclose(file);
    fclose(tmp_file);

    remove(".neogit/config");
    rename(".neogit/tmp_config", ".neogit/config");
    return last_commit_ID;
}

int commit_staged_file(int commit_ID, char* filepath) 
{
    FILE *read_file, *write_file;
    char read_path[MAX_FILENAME_LENGTH];
    strcpy(read_path, filepath); 
    char write_path[MAX_FILENAME_LENGTH];
    strcpy(write_path, ".neogit/files/");
    strcat(write_path, filepath);
    strcat(write_path, "/");
    char tmp[10];
    sprintf(tmp, "%d", commit_ID);
    strcat(write_path, tmp);

    read_file = fopen(read_path, "r");
    if (read_file == NULL) return 1;

    write_file = fopen(write_path, "w");
    if (write_file == NULL) return 1;

    char buffer;
    buffer = fgetc(read_file);
    while(buffer != EOF) {
        fputc(buffer, write_file);
        buffer = fgetc(read_file);
    }
    fclose(read_file);
    fclose(write_file);

    return 0;
}

int track_file(char *filepath) 
{
    if (is_tracked(filepath)) return 0;

    FILE *file = fopen(".neogit/tracks", "a");
    if (file == NULL) return 1;
    fprintf(file, "%s\n", filepath);
    return 0;
}

bool is_tracked(char *filepath) 
{
    FILE *file = fopen(".neogit/tracks", "r");
    if (file == NULL) return false;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        
        if (strcmp(line, filepath) == 0) return true;

    }
    fclose(file); 

    return false;
}

int create_commit_file(int commit_ID, char *message)
 {
    char commit_filepath[MAX_FILENAME_LENGTH];
    strcpy(commit_filepath, ".neogit/commits/");
    char tmp[10];
    sprintf(tmp, "%d", commit_ID);
    strcat(commit_filepath, tmp);

    FILE *file = fopen(commit_filepath, "w");
    if (file == NULL) return 1;

    fprintf(file, "message: %s\n", message);

    FILE* file2 = fopen(".neogit/config","r");
    char line[MAX_LINE_LENGTH];
    char author[1000];
    while(fgets(line,sizeof(line),file2) != NULL){
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }

        if(strncmp(line,"username:",9)==0){
            sscanf(line,"username: %s",author);
        }
    }
    fclose(file2);
    fprintf(file, "author: %s\n",author);

    time_t currentTime;
    struct tm *localTime;
    currentTime = time(NULL);
    localTime = localtime(&currentTime);
    fprintf(file, "time: %02d:%02d:%02d\n", localTime->tm_hour, localTime->tm_min, localTime->tm_sec);

    fprintf(file, "files:\n");
    DIR *dir = opendir(".");
    struct dirent *entry;
    if (dir == NULL) {
        perror("Error opening current directory");
        return 1;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && is_tracked(entry->d_name)) {
            int file_last_commit_ID = find_file_last_commit(entry->d_name);
            fprintf(file, "%s %d\n", entry->d_name, file_last_commit_ID);
        }
    }
    closedir(dir); 
    fclose(file);
    return 0;
}

int find_file_last_commit(char* filepath) 
{
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir, ".neogit/files/");
    strcat(filepath_dir, filepath);

    int max = -1;
    
    DIR *dir = opendir(filepath_dir);
    struct dirent *entry;
    if (dir == NULL) return 1;

    while((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            int tmp = atoi(entry->d_name);
            max = max > tmp ? max: tmp;
        }
    }
    closedir(dir);

    return max;
}

int log_command(int argc, char * const argv[]){
    FILE *file = fopen(".neogit/config", "r");
    if (file == NULL) return -1;

    int commit_count;
    char line[MAX_LINE_LENGTH], tmp[10];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "last_commit_ID", 14) == 0) {
            sscanf(line, "last_commit_ID: %d\n", &commit_count);
        }
    }
    char address[MAX_PATH_LENGTH];
    strcpy(address,".neogit/commit/");
    for(int i=commit_count; i>0; i--){
        sprintf(tmp,"%d",i);
        strcat(address,tmp);
        FILE* file = fopen(address,"r");
        int n=-1;
        char line[MAX_LINE_LENGTH], msg[MAX_MESSAGE_LENGTH], author[1000];
        int h, m, s;
        while (fgets(line, sizeof(line), file) != NULL) {
            int length = strlen(line);

            // remove '\n'
            if (length > 0 && line[length - 1] == '\n') {
                line[length - 1] = '\0';
            }

            if(strncmp(line,"message:",8)==0){
                sscanf(line,"message: %s", msg);
            }
            else if(strncmp(line,"author: ",7)==0){
                sscanf(line, "author: %s", author);
            }

            else if(strncmp(line,"time:",5)==0){
                sscanf(line, "time: %d:%d:%d", &h, &m, &s);
            }

            else
                n++;
        }
        printf("Commit: ID '%d'\nAuthor: %s\nTime: %d:%d:%d\nMessage: '%s'\n", i, author, h, m, s, msg);
    }
}

int status(int argc, char * argv[])
{
    struct dirent *entry;
    DIR *dir = opendir(".");
    if(dir == NULL){
        perror("error oppening current directory\n");
        return 1;
    }
    // searching files
    while((entry = readdir(dir))!=NULL){
        if(is_tracked(entry->d_name) && is_staged(entry->d_name))
            printf("%s state is: +M\n", entry->d_name);
        else if(~(is_tracked(entry->d_name)) && is_staged(entry->d_name))
            printf("%s state is: +A\n", entry->d_name);
        else if(is_tracked(entry->d_name) && ~(is_staged(entry->d_name)))
            printf("%s has not changed\n", entry->d_name);
    }
    closedir(dir);

}

bool is_staged(char *filename)
{
    FILE *file = fopen(".neogit/staging", "r");
    if (file == NULL) return false;
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) != NULL) {
        int length = strlen(line);

        // remove '\n'
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }
        
        if (strcmp(line, filename) == 0) 
            return true;

    }
    fclose(file); 

    return false;
}

int make_branch(int argc, char * const argv[])
{
    if(argc == 2){

    }
    else{
        char branch[1000];
        strcpy(branch, argv[2]);
        struct dirent *entry;
        DIR *dir = opendir(".neogit/branches");
        if(dir == NULL){
            perror("error oppening current directory\n");
            return 1;
        }
        // searching files
        while((entry = readdir(dir))!=NULL){
            if(strcmp(branch,entry->d_name)==0){
                printf("branch already exists\n");
                return 0;
            }
        }

    }
}

int run_checkout(int argc, char * const argv[]) {
    if (argc < 3) return 1;
    
    int commit_ID = atoi(argv[2]);

    DIR *dir = opendir(".");
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && is_tracked(entry->d_name)) {
            checkout_file(entry->d_name, find_file_last_change_before_commit(entry->d_name, commit_ID));
        }
    }
    closedir(dir);

    return 0;
}

int find_file_last_change_before_commit(char *filepath, int commit_ID) {
    char filepath_dir[MAX_FILENAME_LENGTH];
    strcpy(filepath_dir, ".neogit/files/");
    strcat(filepath_dir, filepath);

    int max = -1;
    
    DIR *dir = opendir(filepath_dir);
    struct dirent *entry;
    if (dir == NULL) return 1;

    while((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            int tmp = atoi(entry->d_name);
            if (tmp > max && tmp <= commit_ID) {
                max = tmp;
            }
        }
    }
    closedir(dir);

    return max;
}

int checkout_file(char *filepath, int commit_ID) {
    char src_file[MAX_FILENAME_LENGTH];
    strcpy(src_file, ".neogit/files/");
    strcat(src_file, filepath);
    char tmp[10];
    sprintf(tmp, "/%d", commit_ID);
    strcat(src_file, tmp);

    FILE *read_file = fopen(src_file, "r");
    if (read_file == NULL) return 1;
    FILE *write_file = fopen(filepath, "w");
    if (write_file == NULL) return 1;
    
    char line[MAX_LINE_LENGTH];

    while (fgets(line, sizeof(line), read_file) != NULL) {
        fprintf(write_file, "%s", line);
    }
    
    fclose(read_file);
    fclose(write_file);

    return 0;
}

int main(int argc, char *argv[])
{
   if(argc < 2){
      printf("Invalid Command!\n");
   }
   else if(strcmp(argv[1],"config")==0 && strcmp(argv[2],"-global")==0){
      return global_config(argv[3],argv[4]);
   }

   else if(strcmp(argv[1],"config")==0){
        return local_config(argv[3],argv[4]);
   }

   else if(strcmp(argv[1],"init")==0){
      return initialize(argc, argv);
   }

   else if(strcmp(argv[1],"add")==0){
      if (argc < 3) {
        perror("please specify a file");
        return 1;
      }
      else if(strcmp(argv[2],"-f")==0){
         for(int i=3; i<argc-3; i++){
            return add(argv[i]);
         }
      }
      else if(strcmp(argv[2],"-n")==0){
         char cwd[1024];
         if(getcwd(cwd,sizeof(cwd))==NULL) return 1;
         return add_depth(cwd);
      }
      else
         return add(argv[2]);
   }

   else if(strcmp(argv[1],"reset")==0){
      if (argc < 3) {
        perror("please specify a file");
        return 1;
      }
      else if(strcmp(argv[2],"-f")==0){
         for(int i=3; i<argc-3; i++){
            return reset(argv[i]);
         }
      }

      else if(strcmp(argv[2],"-undo")==0)
         return reset_redo();

      else
         return reset(argv[2]);
   }

   else if(strcmp(argv[1],"status")==0){
      return status(argc,argv);
   }

   else if(strcmp(argv[1],"commit")==0){
      return run_commit(argc,argv);
   }

   else if(strcmp(argv[1],"log")==0){
      return log_command(argc,argv);
   }

   else if(strcmp(argv[1],"branch")==0){
      return make_branch(argc,argv);
   }

   else if(strcmp(argv[1],"checkout")==0){
      return run_checkout(argc,argv);
   }
}