#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <libgen.h>
#include <dirent.h>
#include <signal.h>


// Function that verifies if is a text file 
int isTxt(char * filename){
	char * aux;
  	aux = strrchr(filename,'.');
	if(aux == NULL)
		return 0;
	else
  		return (!strcmp(aux, ".txt"));
}


int main(int argc, char * argv[]){
	
	if(argc != 2){
		printf("Usage: %s <dir> \n", argv[0]);
		return 1;
	}
	// This program ignores CTRL+C signal
	signal(SIGINT,SIG_IGN);

	// Diretory passed in argument	
	DIR *dir;
	struct dirent *dentry;
	struct stat stat_entry;

	// Prepare directory path
	char * dirPath = argv[1];
	if(strcmp(dirPath, "pwd") == 0){
		char pwd[PATH_MAX];
		getcwd(pwd,PATH_MAX);
		dirPath = pwd;
	}
	else if((dirPath = realpath(dirPath,NULL)) == NULL){
		perror("Directory path does not exist");
		return 2; 
	}

	// Prepare execution path
	char * execPath = realpath(argv[0],NULL);
	execPath = dirname(execPath);
	strcat(execPath, "/sw");

	// Open directory path
	if ((dir = opendir(dirPath)) == NULL) {
		
  		perror("Can not open the specified directory");
		return 3; 
	}

	char wordsPath[PATH_MAX];
	strcpy(wordsPath, dirPath);
	strcat(wordsPath, "/words.txt");
	int fdWords = open(wordsPath, O_RDONLY);
	if(fdWords == -1){
		perror("Words File missing:");
		return 4;
	}
	close(fdWords);

	// Cycle used to launch process sw in every valid files, multi-processing
	char filePath[PATH_MAX];
	int status;
	while ((dentry = readdir(dir)) != NULL) {
		stat(dentry->d_name, &stat_entry);
		if (isTxt(dentry->d_name)) {
			if (strstr(dentry->d_name, "ReadMe") == NULL && strstr(dentry->d_name, "index") == NULL && 					strstr(dentry->d_name, "words") == NULL){
				strcpy(filePath, dirPath);
				strcat(filePath, "/");
				strcat(filePath, dentry->d_name);
  				if(fork() == 0){
  					execlp(execPath, execPath, filePath, NULL);
  				}
  				else{
  					waitpid(-1,&status,0);
  				}
  				memset(filePath,0,sizeof(filePath));
  			}
  		}

	}

	// Launch process csc, to concatenate, sort and clean every temporary files  
	execPath = dirname(execPath);
	strcat(execPath, "/csc");
	int pid = fork();
	if(pid == 0){
		execlp(execPath, execPath, dirPath, NULL);
	}
	else if (pid == -1){
		perror("Fork error");
		return 5; 
	}
	else{
		waitpid(-1,&status,0);
	}
	return 0;
}
