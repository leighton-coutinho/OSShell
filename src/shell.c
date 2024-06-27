#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h> 
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "interpreter.h"
#include "shellmemory.h"

#define x fSize
#define y vSize

int MAX_USER_INPUT = 1000;
int parseInput(char ui[]);
// int x = fSize;
// int y = vSize;

int main(int argc, char *argv[]) {
	printf("%s\n", "Shell version 1.2 Created January 2023 \n");
    printf("Frame Store Size = %d; Variable Store Size = %d\n",x,y);
	//help();
    DIR* dir = opendir("backingStore");
    int check;
    if (dir) {
    /* Directory exists. */
        // when to close directory?
        //closedir(dir);
        // These are data types defined in the "dirent" header
        struct dirent *next_file;

        while ( (next_file = readdir(dir)) != NULL )
        {   
            if (strcmp(next_file->d_name, ".") == 0 || strcmp(next_file->d_name, "..") == 0) {
                continue;
            }
            char mypath[256] = "";
            strcpy(mypath,"./backingStore/");
            strcat(mypath, next_file->d_name);
            //printf("removing\n");
            //printf("%s\n", mypath);
            remove(mypath);
        };
    } else if (ENOENT == errno) {
        /* Directory does not exist. */
        check = mkdir("backingStore",0777);
        
    } else {
        printf("failed to open directory for other reason\n");
    /* opendir() failed for some other reason. */
    }
    closedir(dir);
	char prompt = '$';  				// Shell prompt
	char userInput[MAX_USER_INPUT];		// user's input stored here
	int errorCode = 0;					// zero means no error, default


	//init user input
	for (int i=0; i<MAX_USER_INPUT; i++)
		userInput[i] = '\0';
	
	//init shell memory
	mem_init();
    dataStoreinit();
    

	while(1) {						
        if (isatty(fileno(stdin))) printf("%c ",prompt);
		fgets(userInput, MAX_USER_INPUT-1, stdin);
		if (feof(stdin)) {
			freopen("/dev/tty", "r", stdin);
		}
        //return 0;
        if (*userInput != '\0') {
            errorCode = parseInput(userInput);
        }
		//errorCode = parseInput(userInput);
		if (errorCode == -1) exit(99);	// ignore all other errors
		memset(userInput, 0, sizeof(userInput));
	}
    // when to close?
    closedir(dir);
	return 0;

}

int parseInput(char *ui) {
    char tmp[200];
    char *words[100];                            
    int a = 0;
    int b;                            
    int w=0; // wordID    
    int errorCode;
    for(a=0; ui[a]==' ' && a<1000; a++);        // skip white spaces
    
    while(ui[a] != '\n' && ui[a] != '\0' && a<1000 && a<strlen(ui)) {
        while(ui[a]==' ') a++;
        if(ui[a] == '\0') break;
        for(b=0; ui[a]!=';' && ui[a]!='\0' && ui[a]!='\n' && ui[a]!=' ' && a<1000; a++, b++) tmp[b] = ui[a];
        tmp[b] = '\0';
        if(strlen(tmp)==0) continue;
        words[w] = strdup(tmp);
        if(ui[a]==';'){
            w++;
            errorCode = interpreter(words, w);
            if(errorCode == -1) return errorCode;
            a++;
            w = 0;
            for(; ui[a]==' ' && a<1000; a++);        // skip white spaces
            continue;
        }
        w++;
        a++; 
    }
    errorCode = interpreter(words, w);
    return errorCode;
}

//old solution
/*#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include "interpreter.h"
#include "shellmemory.h"


int MAX_USER_INPUT = 1000;
int parseInput(char ui[]);

int main(int argc, char *argv[]) {
	printf("%s\n", "Shell version 1.2 Created January 2023");
	//help();

	char prompt = '$';  				// Shell prompt
	char userInput[MAX_USER_INPUT];		// user's input stored here
	int errorCode = 0;					// zero means no error, default

	//init user input
	for (int i=0; i<MAX_USER_INPUT; i++)
		userInput[i] = '\0';
	
	//init shell memory
	mem_init();
	while(1) {						
		//printf("%c ",prompt);
        //here you should check the unistd library 
        //so that you can find a way to not display $ in the batch mode
		fgets(userInput, MAX_USER_INPUT-1, stdin);
		if (feof(stdin)){
			freopen("/dev/tty", "r", stdin);
		}	
		errorCode = parseInput(userInput);
		if (errorCode == -1) exit(99);	// ignore all other errors
		memset(userInput, 0, sizeof(userInput));
	}

	return 0;

}

int parseInput(char ui[]) {
    char tmp[200];
    char *words[100];  

    int a = 0;
    int b;                            
    int w=0; // wordID    
    int errorCode;
    int noCommands = 0;

    for(a=0; ui[a]==' ' && a<1000 && ui[a]!=';'; a++); // skip white spaces

    while(ui[a] != '\n' && ui[a] != '\0' && a<1000 && ui[a]!=';') {
        for(b=0; ui[a]!=';' && ui[a]!='\0' && ui[a]!='\n' && ui[a]!=' ' && a<1000; a++, b++){
            tmp[b] = ui[a];                   
            // extract a word
        }
        
        tmp[b] = '\0'; 
        words[w] = strdup(tmp); 
        w++;

        if(ui[a] == '\0') break;

        if(ui[a] == ';'){
            errorCode = interpreter(words, w);
            w = 0;
            a++;
            words[w] = '\0';
        }

        a++; 
        
    }
    //printf("running intepreter\n");
    //printf("%s\n", *words);
    errorCode = interpreter(words, w);
    
    return errorCode;
}
*/
