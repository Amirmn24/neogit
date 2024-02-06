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