#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <libgen.h>

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

// Function used to format a line of temporary files with the specific configuration
char * formatLine(char * buffer,char * filename){
	char *formated = malloc(MAX_BUFFER);
	char *word;
	char *line;
	line = strtok(buffer, ":");
	word = strtok(NULL, ":");
	strcpy(formated, word);
	strcat(formated, ": ");
	strcat(formated, strtok(filename, "."));
	strcat(formated, "-");
	strcat(formated, line);
	return formated;
}

int main(int argc, char * argv[]){

	if(argc != 2){
		printf("Usage: %s <file_path> \n", argv[0]);
		return 1;
	}

	// Prepares paths to be used during the execution
	char * basepath;
	basepath = basename(argv[1]);
	
	char * path = argv[1];

	char textFilename[MAX_BUFFER];
	strcpy(textFilename,path);

	path = dirname(path);

	char wordsFilename[MAX_BUFFER];
	strcpy(wordsFilename,path);
	strcat(wordsFilename, "/words.txt");
	char tempFilename[MAX_BUFFER];
	strcpy(tempFilename,path);
	strcat(tempFilename, "/temp");
	strcat(tempFilename, basepath);

	// Tries to open words file 
	int fdWords = open(wordsFilename, O_RDONLY);
	if(fdWords == -1){
		printf("%s\n", wordsFilename);
		perror("Words File missing...\n");
		return 2;
	}

	// Creates a new temporary file 
	int fdTemp = open(tempFilename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if(fdTemp == -1){
		perror("Cannot creat tempFile...\n");
		return 3;
	}

	// Initializes the pipe to be used in searching process 
	int fd[2];
	pipe(fd);
	int status;
	int pid;
	char word[MAX_BUFFER];
	while(readLine(fdWords, word) != 0){
		pid = fork();
		if(pid == 0){
			close(fd[READ]);
			dup2(fd[WRITE],STDOUT_FILENO);
			// Execute searching process, result is sent to pipe
			execlp("grep", "grep", "-n", "-o", "-w" , word,textFilename,NULL);
			return 0;
		}
		else if(pid == -1){
			perror("Fork error");
			return 3; 
		}
		else{
			// Avoids zumbie processes
			waitpid(-1, &status, WNOHANG);
		}
		memset(word,0,sizeof(word));
	}

	close(fdWords);
	close(fd[WRITE]);

	char line[MAX_BUFFER];
	char * formatedLine;

	// Puts every searched words in a temporary file
	while(readLine(fd[READ], line) != 0){
		formatedLine = formatLine(line,basepath);
		write(fdTemp, formatedLine, strnlen(formatedLine, MAX_BUFFER));
		write(fdTemp, "\n", 1);
		memset(line,0,sizeof(line));
	}
	close(fdTemp);
	return 0;
}
