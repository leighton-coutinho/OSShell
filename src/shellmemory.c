#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<math.h>
#include "interpreter.h"

#define x fSize
#define y vSize

struct memory_struct{
	char *var;
	char *value;
};

struct memory_struct shellmemory[1000];
//struct memory_struct shellmemory[SHELL_MEM_LENGTH];

struct DataStore {
	int myframes[(x/3)]; // list of indices that point to where a frame in memory
	int LRUFrameCount[(x/3)]; // LRU count for each frame
	int varStore; // index of where variable storing starts
} thedatastore;

// should just be x/3 only changed to avoid segmentation fault for testing 
// segmentation fault because frame store was full 
void dataStoreinit(){
	int i;
	thedatastore.varStore = x+1;
	for (i=0; i< ((x/3)+ 1); i++){		
		thedatastore.myframes[i] = -1; // -1 means nothing in the frame
		thedatastore.LRUFrameCount[i] = 0; // LRU starts at 0
	}
}

void incLRU(int indx){ //called with curr->currFrame
	thedatastore.LRUFrameCount[indx] += 1;
}

int findFrameToEivct(){ // find minimum LRU that isnt 0 and return it
	int min = INFINITY;
	int leastframe = -1;
	for(int i = 0; i < (x/3); i++){
		//printf("curr frame: %d, LRUcount: %d");
		if(thedatastore.LRUFrameCount[i] > 0 && thedatastore.LRUFrameCount[i] < min){
			min = thedatastore.LRUFrameCount[i];
			leastframe = i;
		}
	}

	if(min != INFINITY) {
		return leastframe;
	} else {
		return -1;
	}
}

int findfreeframe() {
	int i;
	for (i=0; i<(x/3); i++){
		// equal to -1 means free
		if (thedatastore.myframes[i] == -1){
			return i;
		} 
	}

	// means no free frame so need to evict
	//printf("retuned -1 so no free frame\n");
	return -1;

}

void setfreeframe(int frameno) {
	int i;
	for (i=0; i<(x/3); i++){
		// equal to -1 means free
		if (i == frameno){
			thedatastore.myframes[i] = -1;
			return;
		} 
	}

	// means no free frame so need to evict
	//printf("retuned -1 so no free frame\n");
	return;
}

void setframe(int frametoset, int location) {
	int i;
	for (i=0; i<(x/3); i++){
		if (i == frametoset){
			thedatastore.myframes[i] = location;
		} 
	}
	return;

}

int getframelocation(int myframe) {
	int i;
	for (i=0; i<(x/3); i++){
		if (i == myframe){
			return thedatastore.myframes[i];
		} 
	}
	//printf("couldnt find frame\n");
	return -1;

}

// Helper functions
int match(char *model, char *var) {
	int i, len=strlen(var), matchCount=0;
	for(i=0;i<len;i++)
		if (*(model+i) == *(var+i)) matchCount++;
	if (matchCount == len)
		return 1;
	else
		return 0;
}

char *extract(char *model) {
	char token='=';    // look for this to find value
	char value[1000];  // stores the extract value
	int i,j, len=strlen(model);
	for(i=0;i<len && *(model+i)!=token;i++); // loop till we get there
	// extract the value
	for(i=i+1,j=0;i<len;i++,j++) value[j]=*(model+i);
	value[j]='\0';
	return strdup(value);
}


// Shell memory functions

void mem_init(){
	int i;
	for (i=0; i<1000; i++){		
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
	}
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
	int i;
	for (i=0; i<1000; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}

	//Value does not exist, need to find a free spot.
	for (i=0; i<1000; i++){
		if (strcmp(shellmemory[i].var, "none") == 0){
			shellmemory[i].var = strdup(var_in);
			shellmemory[i].value = strdup(value_in);
			//shellmemory[i].value is essentially a pointer to the instruction
			return;
		} 
	}

	return;

}

void mem_set_instruction(char *var_in, char *value_in) {
	int i;
	for (i=0; i<1000; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}

	//Value does not exist, need to find a free spot.
	for (i=0; i<1000; i++){
		if (strcmp(shellmemory[i].var, "null") == 0){
			shellmemory[i].var = strdup(var_in);
			shellmemory[i].value = strdup(value_in);
			//shellmemory[i].value is essentially a pointer to the instruction
			return;
		} 
	}

	return;

}

void mem_clean(int start, int end){
	while(start < end){
        //free(shellmemory[start].var);
        shellmemory[start].var = "none";
		
       // free(shellmemory[start].value);
        shellmemory[start].value = "none";
    	
		start++;
	}
}

void mem_clean_paging(int pageT[], int num){
	for(int i = 0; i < num; i++){
		int startOfPage = thedatastore.myframes[pageT[i]];

		for(int j = startOfPage; j <= startOfPage + 2; j++){
        	shellmemory[j].var = "none";
        	shellmemory[j].value = "none";	
		}
		
	}
}

void mem_clean_frame(int framenum){ 
	int startOfPage = thedatastore.myframes[framenum];
	for(int j = startOfPage; j <= startOfPage + 2; j++){
		shellmemory[j].var = "none";
		shellmemory[j].value = "none";	
	}
}

void display_frame_contents(int framenum){ 
	int startOfPage = thedatastore.myframes[framenum];
	for(int j = startOfPage; j <= startOfPage + 2; j++){
		printf("%s", shellmemory[j].value);	
	}
}

//get value based on input key
char *mem_get_value(char *var_in) {
	int i;
	for (i=0; i<1000; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
			return strdup(shellmemory[i].value);
		} 
	}
	return NULL;

}

char *mem_get_var(int loc) {
	return shellmemory[loc].var;
}


//get value based on location
char *mem_get_valuei(int i) {
	return shellmemory[i].value;
}


// create get mem location function here
int mem_get_location(char *var_in) {
	int i;

	for (i=0; i<1000; i++){
		if (strcmp(shellmemory[i].var, var_in) == 0){
		//	printf("%p\n", &shellmemory[i]); // struct location
		//	printf("%p\n", shellmemory[i].value); // location start of instruction on each line  //differs by 40 per line
		//	printf("%s\n", shellmemory[i].value);  // line itself
		// i is the location withinn shellmemory
			return i;
		} 
	}
	return 1;

}