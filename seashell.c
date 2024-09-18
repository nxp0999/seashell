#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>

char** env_path = NULL;
int env_path_size = 0;


void internal_exec(char** args);
void exit_exec();
void cd_exec(char** args);
void path_exec(char** args);
char** getArgs(char* line);
int parallel_cmd_check(char* line);
void parallel_exec(char* line);

int main(int argc, char* argv[]){
	
	if (argc == 1 ){

		char* line = NULL;
		size_t allocated = 0;
		char *myargs[2];

		while(1){
		
			printf("\ndash>> ");
			//getline(&line, &allocated, stdin);
			if (getline(&line, &allocated, stdin) == -1) {
                		perror("Error reading input");
			}

			line[strcspn(line, "\n")] = 0;
			
			int ps = parallel_cmd_check(line);
			
			if(ps == 0){
				//printf("Inside ps check in main");
				parallel_exec(line);
			}
			else{
				char** args = getArgs(line);
				internal_exec(args);
			}
		}
	}

	else if (argc == 2){

		printf("batch mode\n");
		char* line = NULL;
		size_t allocated = 0;
		FILE *batch_file; 
		batch_file = fopen(argv[1], "r");
		
		if(batch_file == NULL){
			perror("Error reading file input");
		}

		while(getline(&line, &allocated, batch_file) != -1){
			printf("%s", line);
		
			line[strcspn(line, "\n")] = 0;

			int ps = parallel_cmd_check(line);

                        if(ps == 0){
                                parallel_exec(line);
                        }
                        else{
                                char** args = getArgs(line);
                                internal_exec(args);
                        }
		}

		fclose(batch_file);	
	}
	
	
	return 0;
}

int parallel_cmd_check(char* line){

	if (strstr(line,"&")!=NULL){
               	return 0;
        }
        return -1;
}

void parallel_exec(char* line){

        char* token;
        char* saveptr;
        const char delimiter[] = "&";
        token = strtok_r(line, delimiter, &saveptr);

        while(token!=NULL){
                
		char** args = getArgs(token);
		internal_exec(args);		
	
	/*printf("Arguments:\n");
	
	int i = 0;
	while (args[i] != NULL) {
	printf("Arg[%d]: %s\n", i, args[i]);  // Print each arg
		i++;
	}*/
        
		token = strtok_r(NULL, delimiter, &saveptr);
        }
}

void internal_exec(char** args){

	if (strcmp(args[0], "exit")==0){
		exit_exec();
	}

	if (strcmp(args[0], "cd")==0){
		cd_exec(args);
		return;
	}

	if (strcmp(args[0], "path")==0){
		path_exec(args);
		return;
	}
	
	int rc = fork();
	
	if (rc < 0) {
        	perror("Fork failed");
	}
	else if( rc == 0 ){
		execvp(args[0], args);
	}else{
	        int wc = wait(NULL);
		if( wc<0 ){
			perror("Wait failed");
		}
	}
}

void exit_exec(){
	exit(0);
}

void cd_exec(char** args){
	
	chdir(args[1]);	
}

void path_exec(char** args){
	
	if(args[1] == NULL){
		return;
	}
	printf("In path");
}



char** getArgs(char* line){
	
	char** args = malloc(10 * sizeof(char*));

	char* token;
	char* saveptr;
	const char delimiter[] = " ";
	int index = 0;
	token = strtok_r(line, delimiter, &saveptr);

	while(token!=NULL){
		args[index] = malloc((strlen(token) + 1) * sizeof(char));
		strcpy(args[index], token);
		index++;
		token = strtok_r(NULL, delimiter, &saveptr);	
	}

	args[index] = NULL;
	return args;
}












