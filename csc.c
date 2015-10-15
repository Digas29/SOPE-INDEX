#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>

#define MAX_BUFFER 1024
#define READ 0
#define WRITE 1

// Function used to read a line from file descriptor to a buffer
int readLine(int fd, char * buffer){
	char character[1];
	int i = 0;
	while(read(fd,character,1) > 0){
		if(character[0] == '\n'){
			buffer[i] = '\0';
			return i;
		}
		buffer[i] = character[0];
		i++;
	}
	return i;
}

// Function used to format index.txt with a format requested
void formatIndex(int fdRead, int fdIndex, char * word){
	char *word2;
	char *lineNumber;
	char line[MAX_BUFFER];
	char lineCopy[MAX_BUFFER];
	while(1){
		if(readLine(fdRead, line) == 0){
			return;
		}
		strcpy(lineCopy,line);
		word2 = strtok(line, ":");
		lineNumber = strtok(NULL, ":");
		if(strcmp(word, word2) != 0){
			write(fdIndex, "\n\n", 2);
			write(fdIndex, lineCopy, strlen(lineCopy));
			break;
		}
		else{
			write(fdIndex, ",", 1);
			write(fdIndex, lineNumber, strlen(lineNumber));
		}
		memset(lineCopy,0,sizeof(lineCopy));
	}
	formatIndex(fdRead, fdIndex, word2);
}

int main(int argc, char * argv[]){

	if(argc != 2){
		printf("Usage: %s <dir_name> \n", argv[0]);
		return 1;
	}

	// Prepare and open directory 
	DIR *dir;
	struct dirent *dentry;
	struct stat stat_entry;

	if ((dir = opendir(argv[1])) == NULL) {
  		perror(argv[1]);
		return 2; 
	}
	chdir(argv[1]);

	// Create and open index.txt
	char * indexFile = "index.txt";
	int fdIndex = open(indexFile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if(fdIndex == -1){
		perror("Cannot creat or edit index.txt...\n");
		return 3;
	}

	// Cycle used to concatenate and delete every temporary files. Concatenation result is on index.txt
	int fdCat[2];
	pipe(fdCat);
	int i = 0;
	int pid;
	int status;
	while ((dentry = readdir(dir)) != NULL) {
		stat(dentry->d_name, &stat_entry);
  		if (S_ISREG(stat_entry.st_mode) && strstr(dentry->d_name, "temp") != NULL) {
			pid = fork();
  			if(pid == 0){
  				close(fdCat[READ]);
  				dup2(fdCat[WRITE], STDOUT_FILENO);
  				execlp("cat", "cat", dentry->d_name, NULL);
  			}
			else if(pid == -1){
				perror("Fork error");
				return 3; 
			}
  			else{
  				waitpid(-1,&status,0);
  				unlink(dentry->d_name);
  			}
  			i++;
  		}
	}
	close(fdCat[WRITE]);

	// Sort acording alphabetic and numeric order 
	int fdSort[2];
	pipe(fdSort);
	pid = fork();
	if(pid == 0){
		close(fdSort[READ]);
		dup2(fdCat[READ], STDIN_FILENO);
		dup2(fdSort[WRITE], STDOUT_FILENO);
		execlp("sort", "sort", "-V", "-f" ,NULL);
	}
	else if(pid == -1){
		perror("Fork error");
		return 4; 
	}
	else{
		// Format every index in format requereted 
		char *line = malloc(MAX_BUFFER);
		close(fdSort[WRITE]);
		write(fdIndex,"INDEX \n \n", 9);
		readLine(fdSort[READ], line);
		write(fdIndex, line, strlen(line));
		char * word;
		word = strtok(line,":");
		formatIndex(fdSort[READ], fdIndex, word);
	}
	return 0;
}
