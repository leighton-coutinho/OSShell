//#ifndef SHELLMEMORY_H
//#define SHELLMEMORY_H
void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
int mem_get_location(char *var);
char *mem_get_valuei(int i);
void mem_clean(int start, int end);
char *mem_get_var(int loc);
void mem_set_instruction(char *var_in, char *value_in);
void dataStoreinit();
int findfreeframe();
void setframe(int frametoset, int location);
int getframelocation(int myframe);
void mem_clean_paging(int pageT[], int num);
void mem_clean_frame(int framenum);
void setfreeframe(int frameno);
void display_frame_contents(int framenum);
int findFrameToEivct();
void incLRU(int indx);

