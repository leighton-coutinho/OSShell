#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <dirent.h>
#include <unistd.h> //new included library for chdir
#include <sys/stat.h> //new included library for stat
#include <stdbool.h>
#include "shellmemory.h"
#include "shell.h"
#include <pthread.h>
//    ./src/mysh < testcases/assignment2/T_FCFS.txt
int MAX_ARGS_SIZE = 7;
// extern int x = fSize;
// extern int y = vSize;

int countPCB = 0;
pthread_mutex_t lock;
pthread_t worker_threads[2];


typedef struct PCB {
    int PID;
	// not sure if can set by default
    int loc;
    int pcounter; // between 0 - 2 
	int length;
	int jobScore;
	struct PCB *next;
	struct PCB *prev;
	char* pcbfilename;
	int worker_id;
	int numFrames;
	int currFrame; 
	int pageArray[200];
} PCB;

struct PCB createPCB(int l, int pc, int length) { //initialize PCB such that it automatically makes a unique PID
    PCB newPCB = (PCB){++countPCB, l, pc, length, length};
	newPCB.worker_id = -1;
    return newPCB;
}

typedef struct ReadyQueue{
	PCB *head;
	PCB *tail;
	//PCB *prev;
}ReadyQueue;

ReadyQueue queue;

int badcommand(){
	printf("%s\n", "Unknown Command");
	return 1;
}

int badcommandset() {
	printf("%s\n", "Bad command: Too many tokens");
	return 1;
}

// For run command only
int badcommandFileDoesNotExist(){
	printf("%s\n", "Bad command: File not found");
	return 3;
}

int badcommandMkdir(){
	printf("%s\n", "Bad command: my_mkdir");
	return 4;
}

int badcommandCd(){
	printf("%s\n", "Bad command: my_cd");
	return 5;
}

int bad_command_file_does_not_exist(){
	printf("%s\n", "Bad command: File not found");
	return 1;
}

int badcommand_scheduling_policy_error(){
	printf("%s\n", "Bad command: scheduling policy incorrect");
	return 1;
}

int badcommand_no_mem_space(){
	printf("%s\n", "Bad command: no space left in shell memory");
	return 1;
}

int badcommand_ready_queue_full(){
	printf("%s\n", "Bad command: ready queue is full");
	return 1;
}

int badcommand_same_file_name(){
	printf("%s\n", "Bad command: same file name");
	return 1;
}

int handleError(int error_code){
	//Note: badcommand-too-man-token(), badcommand(), and badcommand-same-file-name needs to be raised by programmer, not this function
	if(error_code == 11){
		return bad_command_file_does_not_exist();
	}else if (error_code == 21)
	{
		return badcommand_no_mem_space();
	}else if (error_code == 14)
	{
		return badcommand_ready_queue_full();
	}else if (error_code == 15){
		return badcommand_scheduling_policy_error();
	}else{
		return 0;
	}
}

int help();
int quit();
int set(char* var, char* value);
int print(char* var);
int run(char* script);
int echo(char* toEcho);
int my_ls();
int badcommandFileDoesNotExist();
int my_ls();
int my_mkdir(char* dirname);
int touch(char* filename);
int my_cd(char* dirname);
int exec1(char *filename, char* policy);
int exec2(char *filename, char *filename2, char* policy, int background, int MT);
int exec3(char *filename, char *filename2, char *filename3, char* policy, int background, int MT);

int exec1b(char *filename, char* policy);

int exec1MT(char *filename, char* policy);
int exec1bMT(char *filename, char* policy);



int runfromqueue();


// Interpret commands and their arguments
int interpreter(char* command_args[], int args_size){
	int i;

	if ( args_size < 1 || (args_size > MAX_ARGS_SIZE && strcmp(command_args[0], "set")!=0)){
		return badcommand();
	}

	if (args_size > MAX_ARGS_SIZE){
		return badcommandset();
	}

	for (int i=0; i<args_size; i++){ //strip spaces new line etc
		command_args[i][strcspn(command_args[i], "\r\n")] = 0;
	}

	if (strcmp(command_args[0], "help")==0){
	    //help
	    if (args_size != 1) return badcommand();
	    return help();
	
	} else if (strcmp(command_args[0], "quit")==0) {
		//quit
		if (args_size != 1) return badcommand();
		return quit();

	} else if (strcmp(command_args[0], "set")==0) {
		//set
		if (args_size > 7) return badcommandset();
		char tokens[500];	
		strcpy(tokens, command_args[2]);
		for ( i=3; i<args_size; i++){
			strcat(tokens, " ");
			strcat(tokens, command_args[i]);
		}
		return set(command_args[1], tokens);
	
	} else if (strcmp(command_args[0], "print")==0) {
		if (args_size != 2) return badcommand();
		return print(command_args[1]);
	
	} else if (strcmp(command_args[0], "run")==0) {
		if (args_size != 2) return badcommand();
		return run(command_args[1]);
	
	} else if (strcmp(command_args[0], "echo")==0) {
		//echo
		if (args_size != 2) {
			return badcommand();}
		return echo(command_args[1]);

	} else if (strcmp(command_args[0], "my_ls")==0) {
		//my_ls
		if (args_size != 1) return badcommand();
		return my_ls();

	}
	else if (strcmp(command_args[0], "my_mkdir")==0) {
		//my_ls
		if (args_size != 2) return badcommand();
		return my_mkdir(command_args[1]);
	} 
	else if (strcmp(command_args[0], "my_touch")==0) {
		//my_ls
		if (args_size != 2) return badcommand();
		return touch(command_args[1]);
	}
	else if (strcmp(command_args[0], "my_cd")==0) {
		//my_ls
		if (args_size != 2) return badcommand();
		return my_cd(command_args[1]);
	}
	else if (strcmp(command_args[0], "exec")==0) {
		//exec
		//printf("going to exec command \n");
		if (args_size == 3){
			return exec1(command_args[1],command_args[2]);
		}
		else if (args_size == 4) {
			//could be either background with 1 file or 2 files
			if (strcmp(command_args[3],"#") == 0) {
				return exec1b(command_args[1],command_args[2]);
			}
			else if (strcmp(command_args[3],"MT") == 0) {
				return exec1MT(command_args[1],command_args[2]);
			}
			else {
				return exec2(command_args[1],command_args[2],command_args[3], 0, 0);
			}
		}
		else if (args_size == 5) {
			// exec f1 policy # MT
			// exec f1 f2 Policy #
			// exec f1 f2 Policy MT
			if (strcmp(command_args[4],"#") == 0) {
				return exec2(command_args[1],command_args[2],command_args[3], 1 , 0);
			}
			else if (strcmp(command_args[4],"MT") == 0) {
				if (strcmp(command_args[3],"#") == 0) {
					return exec1bMT(command_args[1],command_args[2]);
				} else {
					return exec2(command_args[1],command_args[2],command_args[3], 0 , 1);
				}
			}
			else {
				return exec3(command_args[1],command_args[2],command_args[3], command_args[4], 0 , 0);
			}
		}
		else if (args_size == 6) {
			// exec f1 f2 policy # MT
			// exec f1 f2 f3 policy MT
			// exec f1 f2 f3 policy #
			if (strcmp(command_args[5],"#") == 0) {
				return exec3(command_args[1],command_args[2],command_args[3], command_args[4], 1, 0);
			} 
			else if (strcmp(command_args[5],"MT") == 0) {
				if (strcmp(command_args[4],"#") == 0) {
					return exec2(command_args[1],command_args[2],command_args[3], 1 , 1);

				} else {
					return exec3(command_args[1],command_args[2],command_args[3], command_args[4], 0, 1);
				}
			} 
			return exec3(command_args[1],command_args[2],command_args[3], command_args[4], 1, 0);
		} 
		else if (args_size == 7) {
			return exec3(command_args[1],command_args[2],command_args[3], command_args[4], 1, 1);
		}
		else {return badcommand();}
	}

	else return badcommand();

}

int help(){

	char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n ";
	printf("%s\n", help_string);
	return 0;
}

int multi = 0;

int quit(){
	printf("%s\n", "Bye!");
	// issue here need to not quit if thread i.e if ready queue is not empty 
	// could be exiting early since we have three threads sometimes including the thread that called quit
	for (int i = 0; i < 2; i++) {
    	pthread_join(worker_threads[i], NULL);
		//printf("joined 1\n");
	}
	//if (queue.head) {
		// if theres still a head after we need to come back and finish instead of exiting
	//	return 1;
//	}
	//return 1;
	exit(0);
}

int set(char* var, char* value){
	char *link = "=";
	char buffer[1000];
	strcpy(buffer, var);
	strcat(buffer, link);
	strcat(buffer, value);
	mem_set_value(var, value);
	return 0;

}

int print(char* var){
	char *value = mem_get_value(var);
	if(value == NULL) value = "\n";
	printf("%s\n", value); 
	return 0;
}



//helper func that loads the file into memory and returns its PCB
PCB loadfile(char* script) {
	PCB mypcb;
	// copy file to backing store
	// Build the full path to the backingStore directory.
    char* backingStorePath = "./backingStore/";
    
    // Build the full path to the input file.
    char* inputFilePath = script;
    
    // Build the full path to the output file in the backingStore directory.
    char* outputFilePath = malloc(strlen(backingStorePath) + strlen(script) + 1);
    sprintf(outputFilePath, "%s%s", backingStorePath, script);
    
    // Open the input file for reading.
    FILE* inputFile = fopen(inputFilePath, "rb");
    if (inputFile == NULL) {
        printf("BadFilename\n");
		return mypcb;
    }
	int fileversions = 1;

    FILE *file;
	char fileversionname[100] = "";
	char fileversionpath[500] = "";
	strcpy(fileversionname, inputFilePath);
	strcpy(fileversionpath, outputFilePath);
	// need to check if the file already exists in backing store
	while ((file = fopen(fileversionpath, "r")))
    {
		memset(fileversionname, 0, sizeof(fileversionname));
		memset(fileversionpath, 0, sizeof(fileversionpath));
		// concatenate filename and version number
		sprintf(fileversionname, "%d", fileversions);
		strcat(fileversionname, inputFilePath);
		// concatenate path and fileversion to obtain final path
		strcpy(fileversionpath, backingStorePath);
		strcat(fileversionpath, fileversionname);
		fileversions++;
		fclose(file);
    }

    // Open the output file in the backingStore directory for writing.
    FILE* outputFile = fopen(fileversionpath, "wb");
    if (outputFile == NULL) {
		printf("Failed to open file in backing store\n");
		return mypcb;
    }
	// add filename to pcb
	
	

    
    // Read blocks of data from the input file and write them to the output file.
    char buffer[4096];
    size_t bytesRead;
	int filelength = 0;
	//(bytesRead = fread(buffer, 1, sizeof(buffer), inputFile))
	while ((fgets(buffer,sizeof(buffer),inputFile)) != NULL) {
		filelength++;
		fputs(buffer, outputFile);
		memset(buffer, 0, sizeof(buffer));

    }
	//printf("%s file length %d\n", script, filelength);
	//while (fgets(chunk, sizeof(chunk), fp) != NULL) {
	//	fputs(chunk, stdout);
	//	fputs("|*\n", stdout);  // marker string used to show where the content of the chunk array has ended
    //}
    
    // Close the input and output files.
    fclose(inputFile);
    fclose(outputFile);
    


	char line[100] = "";
	FILE *p = fopen(outputFilePath,"rt");  // the program is in a file

	if(p == NULL){
		return mypcb;
	}
	int count = 0;
	int frameline;
	mypcb = createPCB(0, 0, 0); // myfloc: location, count: no of
	char *myfilename = malloc(strlen(fileversionname) + 1);
	strcpy(myfilename,fileversionname);
	mypcb.pcbfilename = myfilename;
	mypcb.numFrames = 0;
	int framecount = 0;

	// load 3 lines into frames into frames
	// need to change this to condition to load only 2 pages
	// for page faults could add new property to PCB to say which line
	// in the file we stopped reading from, if linenumber >= length then 
	// we are actually done! 
	while(framecount <= 1){
		int freeframe = findfreeframe();


		mypcb.pageArray[mypcb.numFrames] = freeframe;
		mypcb.numFrames += 1;
		framecount++;

		frameline = 0;
		while (frameline != 3) {
			for (int i=0; i<100; i++) {
				line[i] = '\0';
			}
			fgets(line,sizeof(line),p);
			char num[100] = "";
			count++;
			strcpy(num,"");
			sprintf(num, "%d", count);
			// concatenate line number and filename to obtain variable
			strcat(num, fileversionname);
			//load memory
			//char myend = line[strlen(line)+2];
			mem_set_value(num, line);
			// add address of frame
			if (frameline == 0) {
				// set myframes[freeframe] = location in memory
				int mylocation = mem_get_location(num);
				setframe(freeframe, mylocation);
			}
			frameline++;

			memset(line, 0, sizeof(line));
			memset(num, 0, sizeof(num));

			//free(line);
			if(feof(p)){
				// need to finish with frame and then break out
				if (frameline < 3) {
					while (frameline != 3) {
						char mynum[100] = "";
						char framenum[20] = "";
						// concatenate filename and frame number to obtain variable
						strcpy(mynum, script);
						sprintf(framenum, "%d", frameline);
						strcat(mynum, framenum);
						
						mem_set_value(mynum, "null");
						//printf("%s ? frameNO: %s\n", mem_get_value("my_frame"), mynum);
						frameline++;
						memset(num, 0, sizeof(mynum));
						memset(num, 0, sizeof(framenum));
					}
				}
				break;
			}
			//frameline++;
		}
		if(feof(p)){
			break;
		}

	}


    fclose(p);

	// edit pcb
	mypcb.length = filelength;
	mypcb.jobScore = filelength;
	mypcb.loc = mypcb.pageArray[0]; // location is first page
	mypcb.pcounter = 0; // instruction on the current frame 0, 1 or 2
	mypcb.currFrame = 0; // current frame in pageArray


	// create pcb here
	//char startname[100];
	//strcpy(startname, "1");
	//strcat(startname, script);
	//int myfloc;
	//myfloc = mem_get_location(startname);

	
	//mypcb = createPCB(myfloc, myfloc,count); // myfloc: location, count: no of
	//printf("numframes: %d\n", mypcb.numFrames);
	return mypcb;
}

int appendPCB(char *filename){ //takes a pcb and adds it to end of queue
		// PCB f1pcb = loadfile(filename);
		PCB *f1pcb = (PCB*)malloc(sizeof(PCB)); //make a new PCB dynamically, so that it doesnt go out of scope
    	*f1pcb = loadfile(filename);
		f1pcb->worker_id = -1;
		//printf("%s startinstruc: %s\n", filename, mem_get_valuei(f1pcb->pcounter));
		//PCB *mycurrent = queue.head;

		if (queue.head == NULL) {
			queue.head = f1pcb;
			queue.tail = f1pcb;
			f1pcb->prev = f1pcb;

    	} else {
        	// Otherwise, append the new node to the tail
			// issue with old tail, seems to be null (get random numbers when printing its location) once we finish with the task YY
			// might be removing tail incorrectly or memory corruption
			f1pcb->prev = queue.tail;
			queue.tail->next = f1pcb;
			queue.tail = f1pcb;
			//printf("PREV appendedfile first instruc: %s\n", mem_get_valuei(f1pcb->prev->pcounter));

    	}
		//printf("queue tail prev curr instruc: %s\n", mem_get_valuei(f1pcb->prev->pcounter));

		return 1;

} 

int appendshellpcb() {
	int MAX_USER_INPUT = 100;
	char userInput[MAX_USER_INPUT];	
	// create shell pcb
	PCB *shellpcb= (PCB*)malloc(sizeof(PCB)); //make a new PCB dynamically, so that it doesnt go out of scope
	shellpcb->worker_id = -1;
	int mycount = 0;


	while(1){
		//load line value and filename as a variable into memory with instruction as value
		//build the variable per line
		if (feof(stdin)) {
			break;
		}
		fgets(userInput, MAX_USER_INPUT-1, stdin);
		char num[100];
		mycount++;
		sprintf(num, "%d", mycount);
		// concatenate line number and filename to obtain variable
		strcat(num, "shell");
		//load memory
		mem_set_value(num, userInput);
		memset(userInput, 0, sizeof(userInput));
	}

	// create pcb here
	int myfloc;
	myfloc = mem_get_location("1shell");	
	shellpcb->length = mycount;
	shellpcb->pcounter = myfloc;
	shellpcb->loc = myfloc;
	shellpcb->worker_id = -1;
	//add pcb

	if (queue.head == NULL) {
			queue.head = shellpcb;
			queue.tail = shellpcb;
			shellpcb->prev = shellpcb;
    	} else {
        	// Otherwise, append the shell to head
			queue.head->prev = shellpcb;
			shellpcb->next = queue.head;
			queue.head = shellpcb;
			shellpcb->prev = shellpcb;
    }
	//printf("from shellpcb next instruc %s\n", mem_get_valuei(shellpcb->next->pcounter));
	return 1;

}

int appendPCB_SJF(char *filename){ // takes a pcb and adds it to head of queue if job is shortest

		// PCB f1pcb = loadfile(filename);
		PCB *f1pcb = (PCB*)malloc(sizeof(PCB)); // make a new PCB dynamically, so that it doesnt go out of scope
    	*f1pcb = loadfile(filename);

		// PCB *mycurrent = queue.head;

		if (queue.head == NULL) {
			queue.head = f1pcb;
			queue.tail = f1pcb;

    	} else {
			PCB *mycurrent = queue.head;
			PCB *prev = NULL;

			while (mycurrent != NULL && mycurrent->length <= f1pcb->length) {
				prev = mycurrent;
				mycurrent = mycurrent->next;
			}

			if (prev == NULL) {  // insert at head of list
				f1pcb->next = queue.head;
				queue.head = f1pcb;
			} else {  // insert after prev
				f1pcb->next = prev->next;
				prev->next = f1pcb;
			}

			if (mycurrent == NULL) {  // insert at tail of list
				queue.tail = f1pcb;
			}
			
    	}

		// free(f1pcb);

		return 1;

} 

void ageFunction(PCB *running){
	PCB *dec = running;
	dec = dec->next;

	while(dec != NULL){ // decrease jobScore of everything after running PCB
		dec->jobScore -= 1;
		dec = dec->next;
	}
}

int ageCount = 0;

void age(PCB *running){

	if(queue.head->length + queue.head->loc == queue.head->pcounter){
		// printf("PID of head is %d\n",queue.head->PID);
		// printf("length is %d and counter is %d\n",queue.head->length, queue.head->pcounter);
		queue.head = queue.head->next;
		// printf("in this if\n");
		ageCount++;
		return;
	}

	PCB *curr = queue.head;
	PCB *prev = curr;

	if(curr == running || running->next != NULL){
		curr = curr->next;	
		// printf("if statement in age\n");
	} else {
	    // printf("else statement in age\n");
		return;
	}
	// printf("in age, curr is running->next and prev is curr\n");
	 
	while(curr != NULL){
		if (curr->jobScore < running->jobScore) {
			// printf("in age while loop\n");
			if (curr == running) {
				prev = curr;
				curr = curr->next;
				continue;
			}

            prev->next = curr->next;		
            curr->next = running->next;
            queue.tail->next = running;
			queue.tail = running;
			queue.tail->next = NULL;
			queue.head = curr;
            return;
        }
        prev = curr;
        curr = curr->next;
		
	}

}

// for setting varibles, use the varstore size to set the variable names of a bunch
// of the slots in memory to "vars", to "reserve" space in memory for the variables
// then during loadfile, each file should gain a portion of this memory so we can 
// change the name of a bunch of the "vars" slots to "filename_vars slots", 
// then we must change the set method so that it only sets in the filename_vars slots 

int runRR(char *policy) {
	int instructionlimit = 2;
	if (strcmp(policy, "RR30") == 0) {
		instructionlimit = 30;
	}

	PCB *curr = queue.head;
	int endofscript = curr->loc + curr->length;
	int count = curr->pcounter;
	int instructionno = 0;
	PCB *deletePCB;

	// execute 2 instuctions at a time
	while (count <= endofscript) {
		if (count >= endofscript && curr->next) {
			// finished script so remove from queue and clean
			deletePCB = curr;
			if (deletePCB == queue.head) {
				queue.head = queue.head->next;
			}
			curr->prev->next = curr->next;
			mem_clean(deletePCB->loc,endofscript);
			free(deletePCB);
			// reset count and end of script
			curr = curr->next;
			count = curr->pcounter;
			endofscript = curr->loc + curr->length;
			instructionno = 0;
		}
		else if (count >= endofscript) {
			// reached the end of the tail
			// remove tail
			queue.tail = curr->prev; 
			curr->prev->next = NULL;
			if (curr == queue.head) {
				deletePCB = curr;
				mem_clean(deletePCB->loc,endofscript);
				free(deletePCB);
				// remove from queue
				queue.head = NULL;	
				return 1;
			}
			curr = queue.head;
			endofscript = curr->loc + curr->length;
			count = curr->pcounter;
			instructionno = 0;
		}
		else if (instructionno >= instructionlimit) {
			if (!curr->next) {
				//reached the end so need to start again
				curr = queue.head;
				endofscript = curr->loc + curr->length;
				count = curr->pcounter;
				instructionno = 0;
			} else {
				//printf("moving to next\n");
				curr = curr->next;
				// reset count and end of script
				count = curr->pcounter;
				endofscript = curr->loc + curr->length;
				instructionno = 0;
			}
		}
		// got none instead of echo quit?
		char line[100] = "";
		//char extraline[100] = "";
		strcpy(line, mem_get_valuei(curr->pcounter));
		
		//printf("curr instruc2: %d\n", curr->pcounter);
		count++;
		curr->pcounter++;
		instructionno++;
		//printf("instruc: %s, location: %d\n", line, (curr->pcounter-1));
		if (strcmp(line, "none") != 0) {
			//means not reached the end
			parseInput(line);
		}
		memset(line, 0, sizeof(line));

	} // remove from queue
	queue.head = NULL;
	return 1;
}

int collectpage(PCB *myprogram) {                       //either just pcounter or pcounter + 1
	int currlength = ((myprogram->currFrame * 3) + (myprogram->pcounter + 1));
	char* backingStorePath = "./backingStore/";
    //printf("file to collect page: %s\n", myprogram->pcbfilename);
    // Build the full path to the output file in the backingStore directory.
    char* outputFilePath = malloc(strlen(backingStorePath) + strlen(myprogram->pcbfilename) + 1);
    sprintf(outputFilePath, "%s%s", backingStorePath, myprogram->pcbfilename);
	//printf("currprogram: %s, curr length: %d, myprogram->length: %d\n", myprogram->pcbfilename, currlength, myprogram->length);
	if (currlength <= myprogram->length) {
		// must collect a page
		//printf("file to collect page: %s\n", myprogram->pcbfilename);
		// need and open file using pcbfilename, then iterate to get to correct line, then load a page
		FILE *p = fopen(outputFilePath,"rt");

		int freeframe = findfreeframe();
		// need to check if no free frame then must evict
		if (freeframe == -1) {
			// for now always remove frame 1
			int victimframe = findFrameToEivct(); // was 0 before
			//printf("victim frame: %d\n", victimframe);
			printf("Page fault! Victim page contents:\n\n");
			display_frame_contents(victimframe);
			printf("\nEnd of victim page contents.\n");
			mem_clean_frame(victimframe);
			setfreeframe(victimframe);
		}

		freeframe = findfreeframe();
		myprogram->pageArray[myprogram->numFrames] = freeframe;
		myprogram->numFrames += 1;
		char line[100] = "";
		
		// need to bring file pointer to correct line before extracting
		int fileline = 1;
		// currently having an issue with correct program length,
		// correct however does an extra page fault
		//printf("prog length: %d", myprogram->length);
		while (fileline < currlength) {
			fgets(line,sizeof(line),p);
			//printf("program line: %s\n", line);
			fileline++;
		}
		// now pointer should be in the right part of the file
		int frameline = 0;
		while (frameline != 3) {
			char num[100] = "";
			for (int i=0; i<100; i++) {
				line[i] = '\0';
			}
			fgets(line,sizeof(line),p);
			//printf("prog: %s new length: %d, program line: %s\n", myprogram->pcbfilename, myprogram->length, line);
			//myprogram->length++;
			strcpy(num,"");
			sprintf(num, "%d", currlength);
			// concatenate line number and filename to obtain variable
			strcat(num, myprogram->pcbfilename);
			//load memory
			//char myend = line[strlen(line)+2];
			mem_set_value(num, line);
			//printf("program line: %s\n", line);
			//int mylocation = mem_get_location(num);
			// add address of frame
			if (frameline == 0) {
				// set myframes[freeframe] = location in memory
				int mylocation = mem_get_location(num);
				//printf("num: %d, location val: %s\n", mylocation ,mem_get_valuei(mylocation));
				setframe(freeframe, mylocation);
			}
			frameline++;
			currlength++;
			//printf("num: %s, location val2: %d\n", num ,mem_get_location(num));
			memset(line, 0, sizeof(line));
			memset(num, 0, sizeof(num));

			//free(line);
			if(feof(p)){
				// need to finish with frame and then break out
				if (frameline < 3) {
					while (frameline != 3) {
						//printf("finishing frame\n");
						char mynum[100] = "";
						char framenum[20] = "";
						// concatenate filename and frame number to obtain variable
						strcpy(mynum, myprogram->pcbfilename);
						sprintf(framenum, "%d", frameline);
						strcat(mynum, framenum);
						
						mem_set_value(mynum, "null");
						//printf("%s ? frameNO: %s\n", mem_get_value("my_frame"), mynum);
						frameline++;
						memset(num, 0, sizeof(mynum));
						memset(num, 0, sizeof(framenum));
					}
				}
				break;
			}
			//frameline++;
		}
		fclose(p);


		free(outputFilePath);
		return 1;
	}
	free(outputFilePath);
	return 0;
}

int runRRP(char *policy) {
	int instructionlimit = 2;
	if (strcmp(policy, "RR30") == 0) {
		instructionlimit = 30;
	}

	PCB *curr = queue.head;
	int instructionno = 0;
	PCB *deletePCB;
	//printf("curr num frames: %d\n", curr->numFrames);
	//printf("curr frame: %d\n", curr->currFrame);
	//new vars for paging

	// execute 2 instuctions at a time
	while ((curr->currFrame + 1) <= curr->numFrames) {
		// currently starts at wrong point for last program, might be cause of when we have a page fault
		// or because we skip an instruction by adding to pcount
		// maybe should check if done with time slice before checking end of program 
		if (instructionno >= instructionlimit) {
			if (!curr->next) {
				//reached the end so need to start again
				curr = queue.head;
				instructionno = 0;
			} else {
				//printf("moving to next\n");
				curr = curr->next;
				instructionno = 0;
			}
			continue;
		} 
		if ((curr->currFrame + 1) >= curr->numFrames && curr->pcounter > 2 && curr->next) {
			// finished with pcb need to check if more pages to load
			if (collectpage(curr)) {
				//collected a page need to move to next program in queue
				curr = curr->next;
				instructionno = 0;
				continue;
			} else {
				// finished script so remove from queue and clean
				deletePCB = curr;
				if (deletePCB == queue.head) {
					queue.head = queue.head->next;
					queue.head->prev = queue.head;
				}
				// reset count and end of script
				curr->prev->next = curr->next;
				curr->next->prev = curr->prev;
				curr = curr->next;
				// need to figure out cleaning
				//mem_clean_paging(deletePCB->pageArray, deletePCB->numFrames);
				free(deletePCB);
				instructionno = 0;
			}
			continue;
			// finished script so remove from queue and clean
		}
		else if ((curr->currFrame + 1) >= curr->numFrames && curr->pcounter > 2) {
			//printf("at tail with file: %s\n", curr->pcbfilename);
			if (collectpage(curr)) {
				//collected a page need to move to next program in queue
				curr = queue.head;
				instructionno = 0;
				continue;
			} else {
				// reached the end of the tail
				// remove tail
				queue.tail = curr->prev; 
				curr->prev->next = NULL;
				if (curr == queue.head) {
					deletePCB = curr;
					//mem_clean_paging(deletePCB->pageArray, deletePCB->numFrames);
					free(deletePCB);
					// remove from queue
					queue.head = NULL;
					queue.tail = NULL;	
					return 1;
				}
				deletePCB = curr;
				//mem_clean_paging(deletePCB->pageArray, deletePCB->numFrames);
				free(deletePCB);
				curr = queue.head;
				instructionno = 0;
				
			}
			continue;
		} else if (curr->pcounter > 2) {
			// reset pcounter and move to next frame
			curr->pcounter = 0;
			curr->currFrame++;
			continue;
		} // going to extra frame by 1 iteration why?
		// got none instead of echo quit?
		char line[100] = "";
		//char extraline[100] = "";
		// get instruction from page
		int instruclocation = curr->pcounter + getframelocation(curr->pageArray[curr->currFrame]); 
		strcpy(line, mem_get_valuei(instruclocation));
		
		//printf("curr counter: %d\n", curr->pcounter);
		//printf("curr frame: %d\n", curr->currFrame);
		//count++;
		curr->pcounter++;
		instructionno++;
		incLRU(curr->pageArray[curr->currFrame]);
		//printf("file %s, instruc: %s\n", curr->pcbfilename, line);
		// if null dont execute as this is from extra instrucs for frames
		if (strcmp(line, "null") != 0 && strcmp(line, "none") != 0 ) {
			//means not reached the end
			parseInput(line);
		}
		//printf("instruc: %s\n", line);
		memset(line, 0, sizeof(line));

	} // remove from queue
	queue.head = NULL;
	return 1;
}

int runfromqueue() {
	// start iterating through instructions in the queue
	PCB *curr = queue.head;
	// infinite loop here due to recursion
	int endofscript = curr->loc + curr->length;
	//printf("start: %d\n", curr->pcounter);
	//printf("end: %d\n", endofscript);
	//int count = curr->loc;

	//return 1;
	// goes one over so doesnt enter this loop
	while (curr->pcounter <= endofscript) 
	{
		//printf("in while\n");
		if (curr->pcounter >= endofscript && curr->next) {
			curr = curr->next;
			// reset count and end of script
			endofscript = curr->loc + curr->length;
			continue;
		} else if (curr->pcounter >= endofscript && !curr->next) {
			//in tail and completed so break out
			break;
		}
		char line[100] = "";
		strcpy(line,mem_get_valuei(curr->pcounter));
		curr->pcounter++;
		//check if done with tail so no next 
		//if (strcmp(line,"none") != 0) {
		//	parseInput(line);
		//}
		//printf("intruc: %s\n", line);
		parseInput(line);
		memset(line, 0, sizeof(line));
	}
	//printf("about to start cleaning\n");
	//cleanup and remove from queue
	curr = queue.head;
	while (curr->next) {
		//clean up memory
		mem_clean(curr->loc,endofscript);
		//free PCBs
		free(curr);
		//move to next PCB
		curr = curr->next;
	}

	// clean up for last PCB
	//clean up memory
	mem_clean(curr->loc,endofscript);
	//free PCBs
	free(curr);
	//remove from ready queue
	queue.head = NULL;
	return 1;
}





struct thread_args {
    int thread_id;
    char *policy;
};
struct thread_args args[2];

void *worker(void *arg) {
    // Cast the argument to the worker's ID (0 or 1)
	struct thread_args *args = arg;
    int worker_id = args->thread_id;
    char *policy = args->policy;
	//printf("%s policy, %d workerid\n", policy,worker_id);
	PCB *curr = NULL;
	int instruclimit = 2;
	if (strcmp(policy ,"RR30") == 0) {
		instruclimit = 30;
	}

	pthread_mutex_lock(&lock);

	// Check if there are any jobs in the ready queue and select it
	// if we already set curr, means we just finished executing 2 instructions from curr
	curr = queue.head;
	// added this for when only one worker is spawned and next is spawned only when the other is finished
	if (!queue.head) {
		pthread_mutex_unlock(&lock);
		return NULL;
	}
	while (curr && curr->worker_id != -1) {
		curr = curr->next;
	}

	// Release the lock
	pthread_mutex_unlock(&lock);



	// we found a job to execute
	if (curr) {
		pthread_mutex_lock(&lock);
		int endofscript = curr->loc + curr->length;
		int instructionno = 0;
		PCB *deletePCB;
		curr->worker_id = worker_id;
		pthread_mutex_unlock(&lock);
		// try an instruction from it
		while (queue.head && curr && curr->pcounter <= endofscript) {
			char line[100] = "";
			strcpy(line, mem_get_valuei(curr->pcounter));
			// could try swapping when we increment pcounter 
			// to make sure no two can execute he same isntruc
			// execute instruction after unlocking
			if (strcmp("none", line) != 0) {
				//printf("worker: %d, instruc: %s\n", worker_id, line);
				parseInput(line);
			}
			// currently getting multiple byes printed,
			// could mean several threads executing same instruction
			// might be to do with needing to limit to 2 threads only
			memset(line, 0, sizeof(line));
			// get instruction to execute
			pthread_mutex_lock(&lock);
			curr->pcounter++;
			instructionno++;
			if (curr->pcounter >= endofscript && curr->next) {
				// finished script so remove from queue and clean
				deletePCB = curr;
				if (deletePCB == queue.head) {
					//printf("deletepcb is head\n");
					if (!queue.head) {
						pthread_mutex_unlock(&lock);
						return NULL;
					}
					queue.head = queue.head->next;
					queue.head->prev = queue.head;
				}
				
				curr->prev->next = curr->next;
				curr->next->prev = curr->prev;
				curr = curr->next;
				mem_clean(deletePCB->loc,endofscript);
				free(deletePCB);
				// reset count and end of script
				// need to make sure you dont pick a job another worker is working on
				while (curr->next && curr->worker_id != -1) {
					curr = curr->next;
				}
				//now could be at tail, an actual next or at the end
				if (curr->worker_id != -1) {
					//another worker is on it so go to head
					curr = queue.head;
					if (!curr) {
						pthread_mutex_unlock(&lock);
						return NULL;
					}
				}
				endofscript = curr->loc + curr->length;
				// if we end up on something another worker is already on then quit
				if (curr->worker_id != -1) {
					//printf("other worker on head\n");
					pthread_mutex_unlock(&lock);
					return NULL;
				}
				curr->worker_id = worker_id;
				instructionno = 0;
			}
			else if (curr->pcounter >= endofscript) {
				// reached the end of the tail
				// remove tail
				queue.tail = curr->prev; 
				curr->prev->next = NULL;
				if (curr == queue.head) {
					deletePCB = curr;
					mem_clean(deletePCB->loc,endofscript);
					free(deletePCB);
					// remove from queue
					queue.head = NULL;
					queue.tail = NULL;
					pthread_mutex_unlock(&lock);
					memset(line, 0, sizeof(line));
					return NULL;
				}
				deletePCB = curr;
				mem_clean(deletePCB->loc,endofscript);
				free(deletePCB);
				curr = queue.head;
				if (!curr) {
					pthread_mutex_unlock(&lock);
					return NULL;
				}
				if (curr->worker_id != -1) {
					//other worker is on head
					while (curr->next && curr->worker_id != -1) {
						curr = curr->next;
					}
				}
				// if we end up on something another worker is already on then quit
				if (curr->worker_id != -1) {
					pthread_mutex_unlock(&lock);
					return NULL;
				}
				endofscript = curr->loc + curr->length;
				curr->worker_id = worker_id;
				instructionno = 0;
			}
			else if (instructionno >= instruclimit) {
				if (!curr->next) {
					//reached the end so need to start again
					//we are done so set the current tasks worker id back to -1
					curr->worker_id = -1;
					curr = queue.head;
					if (!curr) {
						pthread_mutex_unlock(&lock);
						return NULL;
					}
					if (curr->worker_id != -1) {
						//other worker on head
						curr = curr->next;
					}
					// set the back to the worker
					curr->worker_id = worker_id;
					endofscript = curr->loc + curr->length;
					instructionno = 0;
				} else {
					//we are done so set the current tasks worker id back to -1
					curr->worker_id = -1;
					curr = curr->next;
					while (curr->next && curr->worker_id != -1) {
						curr = curr->next;
					}
					if (curr->worker_id != -1) {
						//other worker on tail
						curr = queue.head;
						if (!curr) {
							pthread_mutex_unlock(&lock);
							return NULL;
						}
					}
					curr->worker_id = worker_id;
					// reset count and end of script
					endofscript = curr->loc + curr->length;
					instructionno = 0;
				}
			}
			pthread_mutex_unlock(&lock);
		}
	}// remove from queue
	return NULL;
}




int runmultithreaded(char* policy) {
	// only used with RR or RR30
	// each worker will execute time slice of intructions then move to the next available one
	//printf("running multitheaded\n");
	pthread_mutex_init(&lock, NULL);
	//static int num_threads_created = 0;
    // Create the two worker threads
	//pthread_mutex_lock(&lock);
	    // If two threads have already been created, return
    //if (num_threads_created >= 2) {
      //  return 0;
    //}
    for (int i = 0; i < 2; i++) {
		args[i].thread_id = i + 1;
        args[i].policy = policy;
        pthread_create(&worker_threads[i], NULL, worker, &args[i]);
    }

	//pthread_mutex_unlock(&lock);
	//num_threads_created += 2;
	return 1;

}

void printReadyQueue(){
	PCB *curr = queue.head;
	PCB *currN;
	PCB *currNN;
	if(curr->next != NULL){
		currN = curr->next;	
	}else {
		currN = NULL;
	}
	if(curr->next->next != NULL){
		currNN = curr->next->next;	
	}else {
		currNN = NULL;	
	}

	int count = 0;

	// printf("%d,%d -> %d,%d -> %d,%d\n",curr->PID,curr->jobScore,currN->PID,currN->jobScore,currNN->PID,currNN->jobScore);

	while(curr != NULL){
		if(curr->next != NULL){
			printf("%d,%d -> %d,%d\n",curr->PID,curr->jobScore,curr->next->PID,curr->next->jobScore);
		} else {
			printf("%d,%d -> NULL\n",curr->PID,curr->jobScore);
		}

		curr = curr->next;
		count++;

		if(count > 4){
			printf("There is a loop\n");
			break;	
		}
	}
}


int run(char* script) {
	if (1) {
		appendPCB(script);
	}
	runRRP("RR");
} 

int exec1(char *filename, char* policy) {
	if (strcmp(policy, "FCFS") != 0 && strcmp(policy, "SJF") != 0 && strcmp(policy, "RR") != 0 && strcmp(policy, "AGING") != 0 && strcmp(policy, "RR30") != 0) {
		return badcommand();
	}

	if (strcmp(policy, "RR") == 0 || strcmp(policy, "RR30") == 0 ) {
		appendPCB(filename);
		runRRP(policy);
		return 1;
	} else {
		appendPCB(filename);
	}

	//maybe complete run in background function
	//if you encounter exec again then just add the files to queue and continue instead of parsing
	runfromqueue();
	return 0;
	
}

int exec2(char *filename, char *filename2, char* policy, int background, int MT) {
	//if (strcmp(filename, filename2) == 0) {
	//	printf("Bad command: same file name\n");
	//	return 1;
	//}
	if (strcmp(policy, "FCFS") != 0 && strcmp(policy, "SJF") != 0 && strcmp(policy, "RR") != 0 && strcmp(policy, "AGING") != 0 && strcmp(policy, "RR30") != 0) {
		return badcommand();
	}
	if (strcmp(policy, "FCFS") == 0) {
		//PCB f1pcb = loadfile(filename);
		//PCB f2pcb = loadfile(filename2);
		// add to queue here using michel func
		appendPCB(filename);
		appendPCB(filename2);
		if (background) {
			appendshellpcb();
		}
	}
	if (strcmp(policy, "SJF") == 0) {
		// printf("in SJF\n");
		appendPCB_SJF(filename);
		appendPCB_SJF(filename2);
		if (background) {
			appendshellpcb();
		}
	}
	
	if (strcmp(policy, "RR") == 0 || strcmp(policy, "RR30") == 0 ) {
		appendPCB(filename);
		//printf("appended %s pcb\n", filename);
		appendPCB(filename2);
		//printf("appended %s pcb\n", filename2);
		if (background) {
			appendshellpcb();
		}
		if (MT) {
			//printf("running multithreaded\n");
			runmultithreaded(policy);
			return 1;
		}
		runRRP(policy);
		return 1;
	}
	if (strcmp(policy, "AGING") == 0) {
		appendPCB_SJF(filename);
		appendPCB_SJF(filename2);
		if (background) {
			appendshellpcb();
		}

		PCB *curr = queue.head;

		int endofscript = curr->loc + curr->length;
		int count = 0;

		int executions = 0;
		PCB *calc = queue.head;
		while(calc != NULL){
			executions += calc->length;
			calc = calc->next;
		}

		// printf("executions: %d\n",executions);

		while(executions != 0){
			curr = queue.head;

			// if(curr->length == curr->pcounter){
			// 	queue.head = queue.head->next;
			// 	curr = queue.head;
			// }

			// printf("curr is now queue.head, nice\n");
			// printf("PID of curr: %d\n",curr->PID);
			PCB *dec = curr;
			dec = dec->next;

			while(dec != NULL){ // decrease jobScore of everything after running PCB
				// printf("decreasing score as we speak\n");
				if(dec->jobScore == 0){
					dec = dec->next;
				} else {
					dec->jobScore -= 1;
					dec = dec->next;
				}
			}
			// printf("finished while loop to decrease score\n");

			// parseInput(mem_get_valuei(curr->loc));
			// curr->loc += 1;
			char line[100] = "";
			strcpy(line,mem_get_valuei(curr->pcounter));
			curr->pcounter++;
			//check if done with tail so no next 
			//if (strcmp(line,"none") != 0) {
			//	parseInput(line);
			//}
			parseInput(line);
			memset(line, 0, sizeof(line));
			// printf("parsed input, increase pcounter for curr\n");

			count = curr->loc;
			endofscript = curr->loc + curr->length;
			
			age(curr);
			// printf("aged curr\n");
			executions -= 1;
			// printf("executions: %d\n",executions);
		}
		//cleanup and remove from queue
		// curr = queue.head;
		// while (curr->next) {
		// 	//clean up memory
		// 	mem_clean(curr->loc,endofscript);
		// 	//free PCBs
		// 	free(curr);
		// 	//move to next PCB
		// 	curr = curr->next;
		// }

		// // clean up for last PCB
		// //clean up memory
		// mem_clean(curr->loc,endofscript);
		// //free PCBs
		// free(curr);
		// //remove from ready queue
		// queue.head = NULL;
		return 1;
	}
	//printf("head workerid: %d\n", queue.head->worker_id);
	//printf("head instruct: %s\n", mem_get_valuei(queue.head->pcounter));
	//runmultithreaded("RR");
	runfromqueue();
	return 1;
}


int exec3(char *filename, char *filename2, char *filename3, char* policy, int background, int MT) {
	//if (strcmp(filename, filename2) == 0 || strcmp(filename, filename3) == 0 || strcmp(filename2, filename3) == 0) {
	//E	printf("Bad command: same file name\n");
	//	return 1;
	//}
	if (strcmp(policy, "FCFS") != 0 && strcmp(policy, "SJF") != 0 && strcmp(policy, "RR") != 0 && strcmp(policy, "AGING") != 0 && strcmp(policy, "RR30") != 0) {
		return badcommand();
	}
	if (strcmp(policy, "FCFS") == 0) {
		//PCB f1pcb = loadfile(filename);
		//PCB f2pcb = loadfile(filename2);
		// add to queue here using michel func
		appendPCB(filename);
		appendPCB(filename2);
		appendPCB(filename3);

		if (background) {appendshellpcb();}

	} else if (strcmp(policy, "SJF") == 0){
		//PCB f1pcb = loadfile(filename);
		//PCB f2pcb = loadfile(filename2);
		// add to queue here using michel func
		appendPCB_SJF(filename);
		appendPCB_SJF(filename2);
		appendPCB_SJF(filename3);
		if (background) {
			appendshellpcb();
		}
	} 
	if (strcmp(policy, "RR") == 0 || strcmp(policy, "RR30") == 0) {
		appendPCB(filename);
		appendPCB(filename2);
		appendPCB(filename3);
		if (background) {
			appendshellpcb();
		}
		if (MT) {
			printf("in MULTI\n");
			runmultithreaded(policy);
			return 1;
		}
		runRRP(policy);
		return 1;
	}
	if (strcmp(policy, "AGING") == 0) {
		appendPCB_SJF(filename);
		appendPCB_SJF(filename2);
		appendPCB_SJF(filename3);

		if (background) {
			appendshellpcb();
		}

		PCB *curr = queue.head;

		int endofscript = curr->loc + curr->length;
		int count = 0;

		int executions = 0;
		PCB *calc = queue.head;

		while(calc != NULL){
			executions += calc->length;
			calc = calc->next;
		}

		// printf("executions: %d\n",executions);

		while(executions != 0){
			// printf("ageCount: %d\n",ageCount);
			// if(ageCount == 3){
			// 	break;
			// }
			
			curr = queue.head;
			// printReadyQueue();
			// printf("curr is now queue.head, nice\n");
			// printf("PID of curr: %d\n",curr->PID);
			PCB *dec = curr;
			dec = dec->next;

			while(dec != NULL){ // decrease jobScore of everything after running PCB
				// printf("decreasing score as we speak\n");
				if(dec->jobScore == 0){
					dec = dec->next;
				} else {
					dec->jobScore -= 1;
					dec = dec->next;
				}
				
			}
			// printf("finished while loop to decrease score\n");

			// parseInput(mem_get_valuei(curr->loc));
			// curr->loc += 1;
			char line[100] = "";
			strcpy(line,mem_get_valuei(curr->pcounter));
			curr->pcounter++;
			//check if done with tail so no next 
			//if (strcmp(line,"none") != 0) {
			//	parseInput(line);
			//}
			parseInput(line);
			memset(line, 0, sizeof(line));
			// printf("parsed input, increase pcounter for curr\n");

			count = curr->loc;
			endofscript = curr->loc + curr->length;
			
			age(curr);
			// printf("aged curr\n");
			executions -= 1;
			// printf("executions: %d\n",executions);
		}
		//cleanup and remove from queue
		// curr = queue.head;
		// printf("seg fault here1?\n");

		// while (curr->next) {
		// 	printf("seg fault here3r?\n");
		// 	//clean up memory
		// 	mem_clean(curr->loc,endofscript);
		// 	printf("seg fault here4?\n");
		// 	//free PCBs
		// 	free(curr);
		// 	printf("seg fault here5?\n");
		// 	//move to next PCB
		// 	curr = curr->next;
		// 	printf("seg fault here6?\n");
		// }
		// printf("seg fault here2?\n");

		// // clean up for last PCB
		// //clean up memory
		// mem_clean(curr->loc,endofscript);
		// //free PCBs
		// free(curr);
		// //remove from ready queue
		// queue.head = NULL;
		return 1;
	}

	// start iterating through instructions in the queue
	runfromqueue();
	//runmultithreaded("RR");
	return 1;
}

//background execs
int exec1b(char *filename, char* policy) {

	if (strcmp(policy, "FCFS") != 0 && strcmp(policy, "SJF") != 0 && strcmp(policy, "RR") != 0 && strcmp(policy, "AGING") && strcmp(policy, "RR30") != 0) {
		return badcommand();
	}
	if (strcmp(policy, "FCFS") == 0) {
		appendPCB(filename);
	}
	//printf("about to run from queue\n");
	if (strcmp(policy, "SJF") == 0) {
		// keep shell at head even if not shortest
		appendPCB_SJF(filename);
	}
	
	if (strcmp(policy, "RR") == 0 || strcmp(policy, "RR30") == 0) {
		appendPCB(filename);
		runRR(policy);
		return 1;
	}

	// start iterating through instructions in the queue
	//printf("about to run from queue\n");
	appendshellpcb();
	runfromqueue();
	return 1; 
}

// for multithreading create two processes, then each process should handle tasks in the ready queue
// do this in the correct order
//Each worker should take a look at the ready queue and select the first process not in execution by another worker to execute. 
 //To do this, you need to use a lock and find a way to let the worker know which process is executing.
 //look at lecture 5 multi
int exec1MT(char *filename, char* policy) { 
	appendPCB(filename);
	runmultithreaded(policy);
	return 1;
};

int exec1bMT(char *filename, char* policy) {
	appendPCB(filename);
	appendshellpcb();
	runmultithreaded(policy);
	return 1;
};

int echo(char* toEcho){

	if(toEcho[0] == '$'){
		memmove(&toEcho[0], &toEcho[0 + 1], strlen(toEcho));
		char *val = mem_get_value(toEcho);
		
		if(strcmp(val,"Variable does not exist") == 0){
			printf("\n");

		} else {
			printf("%s\n",val);
			
		}

	} else {
		printf("%s\n",toEcho);
	}
}

int my_compar(const struct dirent **a, const struct dirent **b) {
    return strcasecmp((*a)->d_name, (*b)->d_name);
}

int my_ls() {

	struct dirent **files;
	int n;
	int i=0;
	// need to change alphasort to own comparator
	n = scandir(".", &files,NULL,my_compar);
	if (n < 0) return 1;
	else {
		while (i<n) {
			if (files[i]->d_name[0] == '.') {
				free(files[i]);
				++i;
				continue;
			}
			printf("%s\n", files[i]->d_name);
			free(files[i]);
			++i;
		}
		free(files);
	}
	return 0;
}

int my_mkdir(char* dirname) {
	// need to check about names of directories and if spaces are allowed on only single alphanumeric tokens for vars
	char *checkedname;
	int i;
	char *ptr;
	if(dirname[0] == '$'){
		memmove(&dirname[0], &dirname[0 + 1], strlen(dirname));
		checkedname = mem_get_value(dirname);
		for (ptr = checkedname; *ptr != '\0'; ptr++){
			if (*ptr == ' ') {
				printf("Bad command: my_mkdir\n");
				return 1;
			}
		}
		if(strcmp(checkedname,"Variable does not exist") == 0){
			printf("Bad command: my_mkdir\n");
			return 0;
		} else {
			if (mkdir(checkedname, 0777) == -1) return 1;
			else return 0;
		}
	}

	if (mkdir(dirname, 0777) == -1) return 1;
	else return 0;

}

int touch(char* filename) {
	char *ptr;
	FILE *p = fopen(filename,"w"); 
	fclose(p);
	return 0;

}

int my_cd(char* dirname) {
	 if (chdir(dirname) != 0) {
		printf("Bad command: my_cd\n");
		return 1;
	 }
  
	return 0;
	
}
