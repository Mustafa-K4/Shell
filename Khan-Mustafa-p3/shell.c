#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFFSIZE 4096

int main()
{
	check();
	setbuf(stdout, NULL); // makes printf() unbuffered
	int n;
	char cmd[BUFFSIZE];
	pid_t pid;

	
	char* home = getenv("HOME");
	chdir(home);
	char* location = home;
	char cwd[BUFFSIZE];

	// inifite loop that repeated prompts the user to enter a command
	while (1) {
		printf("shell:");
		
		getcwd(cwd, BUFFSIZE);
		location = cwd;
		if (strcmp(home, location) == 0) location = "~";
		char * homep = strstr(location, home);
       		if(homep != NULL) {
			int hlength = strlen(home);
			*(location + hlength - 1) = '~';
			location = location + hlength - 1;
		}

		printf("%s", location);
		printf("$ ");
		n = read(STDIN_FILENO, cmd, BUFFSIZE);

				if (n > 1) {
			cmd[n-1] = '\0'; // replaces the final '\n' character with '\0' to make a proper C string


			int count = 1;
			for (int i = 0; (cmd[i] != '\0'); i++) {

				if (cmd[i] == ' ') count++;

			} // this for loop determines the number of arguments separated by ' '

		   	char* tcmd[count + 1]; // tcmd stands for temporary commands (i/O need to be removed)
			char* token;
			char s[2] = {' ', '\0'};

			token = strtok(cmd,s);

			int i = 0;
			while ((token != NULL) && (i < count)) {

				tcmd[i] = token;
				token = strtok(NULL, s);
				i++;

			} // while loop that adds each token into the array of formatted commands

			tcmd[count] = 0; // ensures that the fcmd array finishes with a 0

		
			int tempcount = 0;
			for (int i = 0; i < count; i++) {

				if ((*tcmd[i] == '<')||(*tcmd[i] == '>')){

					tempcount = tempcount + 2; // once for the operator and once for the filename

				}

			}

			char* fcmd[count + 1 - tempcount]; // fcmd stands for formatted commands
			char* redirection[tempcount];

			int floc = 0; // location / index of fcmd
			int rloc = 0; // location / index of redirection

			if (tempcount > 0) {
				for (int i = 0; i < count; i++) {

					if ((*tcmd[i] == '<')||(*tcmd[i] == '>')) {

						redirection[rloc] = tcmd[i];
						redirection[rloc+1] = tcmd[i +1];
						rloc = rloc + 2;


					} else if ((*tcmd[i-1] != '<') && (*tcmd[i-1] != '>')){

						fcmd[floc] = tcmd[i];
						floc++;

					}

				}

				fcmd[count-tempcount] = 0;

			} else {

				for (int i = 0; i < count + 1; i++) {

					fcmd[i] = tcmd[i];

				}

			}


			// Lab 06 TODO: if the command is "exit", quit the program

			if (strcmp("exit", fcmd[0]) == 0) exit(0);
			// if statement compares all letters/chars of the first command with e x i t
			// *fcmd[0] returns the first char in the string/first command so *(fcmd[0] + 1) returns
			// the next letter in the first command

			if (strcmp("cd", fcmd[0]) == 0) {

				if (*fcmd[1] == '~') {

					int length = strlen(fcmd[1]) - 1; // length of command - ~
					int hlength = strlen(home);
					char final [length + hlength];

					for (int i = 0; i < hlength; i ++) {
						final[i] = *(home + i);
					}

					for (int i = 0; i < length; i++) {
						final[hlength + i] = *(fcmd[1] + 1 + i);
					}

					if(chdir(final) < 0) {
						perror("change directory");
					}

				} else if (strcmp("..", fcmd[1]) == 0) {

					getcwd(cwd, BUFFSIZE);

					char * temp;
					temp = strrchr(cwd, '/');
					*temp = '\0';

					if (chdir(cwd) < 0) {
						perror("change directory");
					}

				} else {

					if(chdir(fcmd[1]) < 0) {
						perror("change directory");
					}

				}

				continue;

			}

			int fd = STDIN_FILENO;
			int fd2 = STDOUT_FILENO;
			int fd3 = STDOUT_FILENO;

			if ((pid = fork()) < 0) perror("fork");
			else if (pid == 0) {

				if (tempcount > 0) {

					for (int i = 0; i < tempcount; i++) {

						if (*redirection[i] == '<') {

							if((fd = open(redirection[i + 1],O_RDONLY)) == -1) perror("open");
							dup2(fd,STDIN_FILENO);

						} else if ((*redirection[i] == '>') && (*(redirection[i]+1) == '>')){

							if((fd2 = open(redirection[i + 1], O_WRONLY|O_CREAT|O_APPEND, 0644)) == -1) perror("open2");
							dup2(fd2, STDOUT_FILENO);

						} else if ((*redirection[i] == '>')) {

							if((fd3 = open(redirection[i + 1], O_WRONLY|O_CREAT|O_TRUNC, 0644)) == -1) perror("open3");
							dup2(fd3, STDOUT_FILENO);
						}

					}
				}

				if ((execvp(fcmd[0], fcmd) < 0)) {

					perror("execvp");
					return EXIT_FAILURE;
				}

				if (fd > 2) close(fd);
				if (fd2 > 2) close(fd2);
				if (fd3 >2) close(fd3);

			} else {

				int status;
				wait(&status);
				if (WIFSIGNALED(status)) printf("child abnormal termination \n");

			}


		} // if
	} // while

} // main
