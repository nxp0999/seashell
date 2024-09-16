#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>

/*#define exit 1
#define cd 2
#define path 3


typedef struct { char *cmd; int val; } t_symstruct;

static t_symstruct lookuptable[] = {
    { "exit", exit }, { "cd", cd }, { "path", path }
    };*/

void internal_exec(char** args);
void exit_exec();
void cd_exec(char** args);
void path_exec();
char** getArgs(char* line);

int main(int argc, char* argv[]){
	
	if (argc == 1 ){

		char* line = NULL;
		size_t allocated = 0;
		char *myargs[2];

		while(1){
		
			printf("\ndash>> ");
			getline(&line, &allocated, stdin);
			line[strcspn(line, "\n")] = 0;

			char** args = getArgs(line);
        		int i=0;
			//while(args[i]!=NULL){
			//	printf("\n %d : %s", i, args[i]);
			//	i++;
			//}
			
			internal_exec(args);

		}

	}

	else if (argc == 2){

		printf("batch mode\n");
		char* line = NULL;
		size_t allocated = 0;
		FILE *batch_file; 
		batch_file = fopen(argv[1], "r");

		while(getline(&line, &allocated, batch_file) != -1){
			printf("%s", line);
			//internal_exec(cmd);
		
			line[strcspn(line, "\n")] = 0;

			char** args = getArgs(line);
			internal_exec(args);
		}

		fclose(batch_file);
		

	}
	
	
	return 0;
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
		path_exec();
		return;
	}
	
	int rc = fork();
	 
	if( rc == 0 ){
		execvp(args[0], args);
	}else{
	        int wc = wait(NULL);
	}
}

void exit_exec(){
	exit(0);
}

void cd_exec(char** args){
	
	chdir(args[1]);	
}

void path_exec(){
	//HOW
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












