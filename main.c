#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define debug(x) printf("%s", x);

int checkdir();
bool check_file_directory_exists(char *filepath);
int global_config(char *username, char *email);
int local_config(char *username, char *email);
int initialize(int argc, char* argv[]);
int add(char *filepath);
int add_depth(char * dirname);
int reset(char *filepath);
int reset_redo();



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
   fprintf(file,"username: %s\n",username);
   fprintf(file,"email: %s\n",email);
   fclose(file);
}


int initialize(int argc, char* argv[])
{
    char cwd[1024];
    if(getcwd(cwd, sizeof(cwd))==NULL) return 1;
    char tmp_cwd[1024];
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

   char line[1000];
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

int run_commit(int argc, char *argv[])
{
   char message[100];
   strcpy(message,argv[3]);
   if(strlen(message)>72)
      perror("message is too long!\n");
   
}

int main(int argc, char *argv[])
{
   if(argc < 2){
      printf("Invalid Command!\n");
   }
   else if(strcmp(argv[1],"config")==0 && strcmp(argv[2],"-global")==0){
      return global_config(argv[3],argv[4]);
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
}