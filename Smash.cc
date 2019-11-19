#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <glob.h>

using namespace std;
extern char **environ;
int flag = 0;

char** splitString(char* unsplitString, char* token){ //http://stackoverflow.com/questions/15472299/split-string-into-tokens-and-save-them-in-an-array
	char **answer = new char*[1024];
	int inc = 0;
	char *d = strtok(unsplitString, token);
	while (d != NULL){
		answer[inc++] = d;
		d = strtok(NULL, token);
	}
	return answer;
}

void error(bool flag) {
	if (flag) {
		perror("Bad open");
		exit(1);
	}
}

void my_function(int sig) { 
	flag = 1;
}

void execlThis(const char *path, const char*arg) {
	execl(path, arg, (char*)0);
	perror("Execl error");
}

void execvThis(const char *path, char *const arg[]) {
	execv(path, arg);
	perror("Execv error");
}

bool CheckWord(char* filename, char* search) { //https://stackoverflow.com/questions/13482464/checking-if-word-exists-in-a-text-file-c
	int offset;
	string line;
	ifstream Myfile;
	Myfile.open(filename);

	if (Myfile.is_open()) {
		while (!Myfile.eof()) {
			getline(Myfile, line);
			if ((offset = line.find(search, 0)) != string::npos) {
				return true;
			}
		}
		Myfile.close();
	}
	else
		cout << "Unable to open this file." << endl;
	return false;
}



int main(int argc, char **argv){

	using_history();
	read_history ("CH.txt");
	char *ps = new char[1024];
	string path, user, username;
	const char *pch;

	for (int i = 0; environ[i] != NULL; i++) {
		if (environ[i][0] == 'P' && environ[i][1] == 'A' && environ[i][2] == 'T' && environ[i][3] == 'H') {
			path = environ[i];
			//environ[i] = const_cast<char*>(change.c_str());
		}
		if (environ[i][0] == 'U' && environ[i][1] == 'S' && environ[i][2] == 'E' && environ[i][3] == 'R') {
			user = environ[i];
			username = (user.substr(5, user.length() + 1) + ": ");
			pch = username.c_str();
		}
	}

	char *paths = new char[path.length() + 1];
	strcpy(paths, path.c_str());
	setenv("PS1", pch, 1); //Sets the PS1 command prompt.

	char **PathsArray = splitString(paths, (char*)":");
	char **AliasValues = new char*[1024];
	char **AliasCommands = new char*[1024];
	int aliasIndex = 0;

	while(true){
		ps = getenv("PS1");
		char * holdingArg = readline(ps); //Gets user input from the PS1 command prompt and stores in holdingArg.
		
		if (flag) {   //https://stackoverflow.com/questions/17766550/ctrl-c-interrupt-event-handling-in-linux
			cout << "\n Signal caught!\n" << endl;;
			flag = 0;
		}

		if (holdingArg[0] == 'q' && holdingArg[1] == 'u' && holdingArg[2] == 'i' && holdingArg[3] == 't')
			break;

		if(aliasIndex != 0){ //If user changed PS1, change user input to alias command.
			for(int i = 0; i < aliasIndex; i++){
				if(strcmp(holdingArg, AliasValues[i]) == 0){
					holdingArg = AliasCommands[i];
				}
			}
		}

		add_history(holdingArg);
		append_history(1, "CH.txt");

		if (holdingArg[0] == 'c' && holdingArg[1] == 'd' && holdingArg[2] == ' ') {
			char **ArgsArray = new char*[1024];
			ArgsArray = splitString(holdingArg, (char*)" ");
			int test = chdir(ArgsArray[1]);
			if (test == -1) {
				cout << "No such directory exists." << endl;
			}
			strcpy(holdingArg, " ");
		}

		if (holdingArg[0] == '$' && holdingArg[1] == 'P' && holdingArg[2] == 'A' && holdingArg[3] == 'T'
			&& holdingArg[4] == 'H') {
			//Used for testing if environs changed.
			for (int i = 0; environ[i] != NULL; i++)
				cout << environ[i] << endl;
		}

		if (holdingArg[0] == 'a' && holdingArg[1] == 'l' && holdingArg[2] == 'i' && holdingArg[3] == 'a'
			&& holdingArg[4] == 's' && holdingArg[5] == ' ') {
			char **ArgsArray = new char*[1024];
			holdingArg[5] = '=';
			ArgsArray = splitString(holdingArg, (char*)"=");
			AliasValues[aliasIndex] = ArgsArray[1];
			AliasCommands[aliasIndex] = ArgsArray[2];
			aliasIndex++;
		}


		int looper = 0;
		while (holdingArg[looper]) {
			//if (holdingArg[looper] == '|') {
			//}
			//if (holdingArg[looper] == ';') { // TODO queueing
			//}
			//if (holdingArg[looper] == '*') { // TODO globbing
			//}
			if (holdingArg[looper] == '=') { //sets env vars like PS1

				/* char **arrayOfEq = new char*[1024];				//Following 4 lines are for setting env vars with setenv()
				arrayOfEq = splitString(holdingArg, (char*)"=");
				cout << arrayOfEq[0] << endl;
				setenv(arrayOfEq[0], arrayOfEq[1], 1); */

				bool flag2 = false;
				for (int i = 0; environ[i] != NULL; i++) {	//Own code for setting env vars. Doesn't use setenv().
					if ((holdingArg[0] == environ[i][0]) && (holdingArg[1] == environ[i][1])
						&& (holdingArg[3] == environ[i][3])) {
						flag2 = true;
						environ[i] = holdingArg;
						cout << environ[i] << endl;
						//break;
					}
				}

				if (flag2 == false) {
					char **arrayOfEq = splitString(holdingArg, (char*)"=");
					cout << arrayOfEq[0] << " does not exist!" << endl;
				}

			}
			looper++;
		}


		pid_t pid = fork();
		if (pid < 0) {
			cout << "Forking Error" << endl;
			exit(1);
		}

		else if (pid == 0) {
			//child

			looper = 0;
			char *childArg = new char[1024];
			childArg = holdingArg;
			bool lscheck = false;

			while (childArg[looper]) {
				if (childArg[looper] == '>') {
					char ** fd2 = splitString(childArg, (char*)" > ");
					int fdout = open(fd2[1], O_WRONLY);
					error(fdout == -1);
					dup2(fdout, STDOUT_FILENO);
					close(fdout);
					execlp(fd2[0], fd2[0], NULL);
				}
				if (childArg[looper] == '<') {
					char ** fd2 = splitString(childArg, (char*)" < ");
					int fdin = open(fd2[0], O_WRONLY);
					error(fdin == -1);
					dup2(fdin, STDIN_FILENO);
					close(fdin);
					execlp(fd2[1], fd2[1], NULL);
					//dup(fd);
					//execvp(fd2[1], fd2);
					//execl(fd2[1], "/ls", (char*)0);
				}
				if (childArg[looper] == '$') {
					if (childArg[0] == 'l' && childArg[1] == 's') lscheck = true;
					char **ArgsArray = splitString(childArg, (char*)"$");
					char *paths2 = getenv(ArgsArray[1]);
					//cout << paths2 << endl;
					//cout << ArgsArray[0] << endl;
					execlThis(paths2, ArgsArray[0]);
				}
				looper++;
			}

			char **ChildArgsArray = splitString(childArg, (char*)" ");

			looper = 0;
			while(childArg[looper]){		//handles executables.
				if(childArg[0] == '/'){
					string file = "approved.txt";
					char *file2 = new char[file.length() + 1];
					strcpy(file2, file.c_str());
					CheckWord(file2, childArg);
					if (CheckWord(file2, childArg)) {
						//execv(childArg, ChildArgsArray);
						execvThis(childArg, ChildArgsArray);
						break;
					}
					else {
						cout << childArg << " is not in the approved executables list." << endl;
						break;
					}
				}
				looper++;
			}

			looper = 0;
			if (lscheck == false) {
				while (PathsArray[looper]) {
					char apendSplitPaths[1024];
					strcpy(apendSplitPaths, PathsArray[looper]);
					strcat(apendSplitPaths, "/");
					strcat(apendSplitPaths, childArg);
					execv(apendSplitPaths, ChildArgsArray);
					//execvThis(apendSplitPaths, ChildArgsArray);
					looper++;
				}
			}
			return 0;
		}
		else{
			//parent
			int status;
			wait (&status);
		}
	}
	return 0;

}
