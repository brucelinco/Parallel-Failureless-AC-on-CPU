#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
//#include <cuda_runtime.h>

//#define OUTPUT_PFAC_TABLE

#define MAX_STATE 131000
#define CHAR_SET 256
#define PATTERN_LEN 10000
#define TRAP_STATE  0xFFFFFFFF

char *input_string;
int input_size;
int PFAC_table[MAX_STATE][CHAR_SET];
int output_table[MAX_STATE];
int *match_result;
int pattern_len[11000];
void PFAC_CPU(char* d_input_string, int* d_match_result, int input_size, int initial_state);  
int create_table_reorder(char *patternfilename, 
  int *state_num_ptr, int *num_finalState_ptr, int *initial_state_ptr ) ;


int main(int argc, char **argv)
{
    int i;
    int state_num;
    int deviceID = 0;
    FILE *fpin;
    FILE *fpout;
    int count=0;
    int num_finalState;
    int initial_state;
    

    create_table_reorder(argv[1], &state_num, &num_finalState , &initial_state);    


    // read input data
    fpin = fopen(argv[2], "rb");
    if (fpin == NULL) {
        perror("Open input file failed.");
        exit(1);
    }
    // obtain file size:
    fseek (fpin , 0 , SEEK_END);
    input_size = ftell (fpin);
    rewind (fpin);
    // allocate memory to contain the whole file:
    input_string = (char *) malloc (sizeof(char)*input_size);
    if (input_string == NULL){
        perror("Allocate input memory error");
        exit (1);
    }//else printf("The input size is %d\n",input_size);
    	
    // copy the file into the buffer:
    input_size = fread (input_string, 1, input_size, fpin);
    fclose(fpin);
    
    // allocate memory for output
    match_result = (int *) malloc (sizeof(int)*input_size);
    if (match_result == NULL){
        perror("Allocate output memory error");
        exit (1);
    }

    
    //printf("[1]\n");
    PFAC_CPU(input_string, match_result, input_size, initial_state);
    //printf("[2]\n");

    // Output results
    fpout = fopen("PFAC_CPU_match_result.txt", "w");
    if (fpout == NULL) {
        perror("Open output file failed.\n");
        exit(1);
    }
    // Output match result to file
    for (i = 0; i < input_size; i++) {
        if (match_result[i] != 0) {
            fprintf(fpout, "At position %4d, match pattern %d\n", i, match_result[i]);
            count++;
        }
    }
    printf("The matched number is %d\n", count);
    fclose(fpout);
    
    free( input_string ) ;
    free( match_result ) ;

    return 0;
}



void PFAC_CPU(char* d_input_string, int* d_match_result, int input_size, int initial_state){
	int start;
	int pos;    // position to read input for the thread
    int state;
    int inputChar;
    struct timeval t_start, t_end;
    float elapsedTime;
    int match;

    // initialize match result on CPU
    for (pos = 0; pos < input_size; pos++) {
        d_match_result[pos] = 0;
    }

    // start time
    gettimeofday(&t_start, NULL);
    
    for (start=0; start < input_size; start++) {
        state = initial_state;
        pos = start;
        match=0;
 
        while ( pos < input_size ) {
            inputChar =(unsigned char)d_input_string[pos];
            state = PFAC_table[state][inputChar];
            
            if ( -1 == state ){ break ; }
            	
            
            if (state < initial_state) {
            	 match = state;
            }
           	pos = pos + 1;
        }
        d_match_result[start]=match;
   
   
    }
    
    // stop time
    gettimeofday(&t_end, NULL);
    // compute and print the elapsed time in millisec
    elapsedTime = (t_end.tv_sec - t_start.tv_sec) * 1000.0;
    elapsedTime += (t_end.tv_usec - t_start.tv_usec) / 1000.0;
    printf("The elapsed time is %f ms\n", elapsedTime);
    printf("The input size is %d bytes\n", input_size );
    printf("The throughput is %f Gbps\n",(float)(input_size)/(elapsedTime*1000000/8) );
	
}

int create_table_reorder(char *patternfilename, 
    int *state_num_ptr, int *num_finalState_ptr, int *initial_state_ptr )
{
    FILE *fpin;
	  char string[PATTERN_LEN];
    int pattern_num ;
    int count ;
    int pre_state;
    int pre_char;
    int final_state=1;//old is 0
    int i, j ;
    int state ;
    int state_num ;
    int initial_state ;
        
    // initialize PFAC table
    for (i = 0; i < MAX_STATE; i++) {
        for (j = 0; j < CHAR_SET; j++) {
            PFAC_table[i][j] = TRAP_STATE ;
        }
        output_table[i] = 0;
    }
    
    // open input file
    fpin = fopen(patternfilename, "rb");
    if (fpin == NULL) {
        perror("Open input file failed.");
        exit(1);
    }
        
    //count pattern number
    pattern_num=0;
    fgets(string, PATTERN_LEN,fpin);
    while(!feof(fpin)){
   	    int len = strlen(string);
		    if ( '\n' == string[len-1] ){
 	          len-- ;
 	      }
 	      pattern_num++;
	      pattern_len[pattern_num]=len;
        fgets(string, PATTERN_LEN,fpin);
    }
    printf("The number of pattern:%d\n ", pattern_num );
    rewind(fpin);
   
    //set initial state
    initial_state = pattern_num+1 ; //old is "initial_state = pattern_num;"
    printf("initial state : %d\n", initial_state);

    //end of count pattern number
	  pattern_num=0;//old is 0
	  state = initial_state; // state is current state
    state_num = initial_state+1; // state_num usable state
    
    pre_state = -1 ; // wrong state
    pre_char = -1 ; // wrong char
    final_state=1;
    while (1) {
        int ch = fgetc(fpin);
        if (ch == EOF) {    // read file end
            break;
        }
        else if (ch == '\n') {    // finish reading a pattern
        	  PFAC_table[pre_state][pre_char] = final_state;
            pattern_num = pattern_num + 1;
            output_table[final_state] = pattern_num;
            
            for(i=0;i<CHAR_SET;i++){
               PFAC_table[final_state][i]= PFAC_table[state][i];
               PFAC_table[state][i]=TRAP_STATE;
             }
             
            state_num--;
            state = initial_state;
            final_state++;
        }
        else {    // create table
            if (TRAP_STATE == PFAC_table[state][ch]) {
                PFAC_table[state][ch] = state_num;
                pre_state = state;
                pre_char = ch;
                state = state_num; // go to next state
                state_num = state_num + 1;
            }
            else {
            	  pre_state = state;
                pre_char = ch;
                state = PFAC_table[state][ch]; // go to next state
            }
        }
        
        if (state_num > MAX_STATE) {
            perror("State number overflow\n");
            exit(1);
        }
    }
    printf("The number of state is %d\n", state_num);

#ifdef OUTPUT_PFAC_TABLE
    // open output file
    fpout = fopen("PFAC_table.txt", "w");
    if (fpout == NULL) {
        perror("Open output file failed.\n");
        exit(1);
    }
    // output PFAC table
    for (i = 0; i < state_num; i++) {
        for (j = 0; j < CHAR_SET; j++) {
            fprintf(fpout, "%d ", PFAC_table[i][j]);
        }
        fprintf(fpout, "%d\n", output_table[i]);
    }
#endif
    
    *state_num_ptr = state_num ;
    *num_finalState_ptr = pattern_num ;
    *initial_state_ptr = initial_state ;
    return state_num;

}
