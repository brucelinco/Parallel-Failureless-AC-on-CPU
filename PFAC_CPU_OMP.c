#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <omp.h>

//#define OUTPUT_PFAC_TABLE

#define MAX_STATE 131000
#define CHAR_SET 256
#define PATTERN_LEN 10000
char *input_string;
int input_size;
int PFAC_table[MAX_STATE][CHAR_SET];
int output_table[MAX_STATE];
int *match_result;
int pattern_len[11000];
int initial_state;
    
int create_table(char *patternfilename)
{
    int i, j;
    int state_num = 1;
    int pattern_num = 0;
    int ch;
    int state = 0;
    FILE *fptr;
    FILE *fpout;
    int pre_state;
    char pre_char;
    int final_state=0;
    int count;
    char string[PATTERN_LEN];
 
    // initialize PFAC table
    for (i = 0; i < MAX_STATE; i++) {
        for (j = 0; j < CHAR_SET; j++) {
            PFAC_table[i][j] = -1;
        }
        output_table[i] = 0;
    }
    
    // open input file
    fptr = fopen(patternfilename, "rb");
    if (fptr == NULL) {
        perror("Open input file failed.");
        exit(1);
    }
    
   //count pattern number
   pattern_num=1;
   count=0;
   fgets(string, PATTERN_LEN,fptr);
   while(!feof(fptr)){
   	int len=strlen(string);
		 if ( '\n' == string[len-1] ){
 	          len-- ;
 	   }
	   pattern_len[pattern_num]=len;
     count++;
     pattern_num++;
     fgets(string, PATTERN_LEN,fptr);
     
   }
   printf("The number of pattern:%d\n ",count);
   rewind(fptr);
      
   //set initial state
   initial_state=count; 
   printf("initial state : %d\n",initial_state);
   
   
  //end of count pattern number
	pattern_num=0;
	state=initial_state;
  state_num=initial_state+1;
  
    while (1) {
        int ch = fgetc(fptr);
        if (ch == EOF) {    // read file end
            break;
        }
        else if (ch == '\n') {    // finish reading a pattern
        	  PFAC_table[pre_state][pre_char] = final_state;
            pattern_num = pattern_num + 1;
            output_table[final_state] = pattern_num;
            
            for(i=0;i<256;i++){
               PFAC_table[final_state][i]=PFAC_table[state][i];
               PFAC_table[state][i]=-1;
             }
             
            state_num--;
            state = initial_state;
            final_state++;
        }
        else {    // create table
            if (PFAC_table[state][ch] == -1) {
                PFAC_table[state][ch] = state_num;
                pre_state=state;
                pre_char=ch;
                state = state_num;
                state_num = state_num + 1;
            }
            else {
            	  pre_state=state;
                pre_char=ch;
                state = PFAC_table[state][ch];

            }
        }
        
        if (state_num > MAX_STATE) {
            perror("State number overflow\n");
            exit(1);
        }
    }
    printf("The number of state is %d\n",state_num);
    
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
    
    return state_num;
}


void PFAC_CPU(char* d_input_string, int* d_match_result, int input_size){
		int start;
		int pos;    // position to read input for the thread
    int state;
    int inputChar;
    int match_pattern = 0;	
    struct timeval t_start, t_end;
    float elapsedTime;



    // initialize match result on CPU

    for (pos = 0; pos < input_size; pos++) {
        d_match_result[pos] = 0;
    }

    // start time
    gettimeofday(&t_start, NULL);
    #pragma omp parallel private (start, state, pos, inputChar) shared (d_match_result, input_size, d_input_string, PFAC_table, initial_state) num_threads(8)
    {
    #pragma omp for schedule(dynamic,32768)
    for (start=0; start < input_size; start++) {
        state = initial_state;
        pos = start;

        while ( (state != -1) && (pos < input_size) ) {
 
            if(state <initial_state){
            	 d_match_result[start]=state+1;
            }
            	
            // read input character

            inputChar =(unsigned char)d_input_string[pos];

            state = PFAC_table[state][inputChar];

			pos = pos + 1;
        }
    }
    }
    
    // stop time
    gettimeofday(&t_end, NULL);
    // compute and print the elapsed time in millisec
    elapsedTime = (t_end.tv_sec - t_start.tv_sec) * 1000.0;
    elapsedTime += (t_end.tv_usec - t_start.tv_usec) / 1000.0;
    printf("The elapsed time is %f ms\n", elapsedTime);
    printf("The input size is %d bytes\n", input_size );
    printf("The throughput is %f Gbps\n",(float)input_size/(elapsedTime*1000000)*8 );
	
}


int main(int argc, char **argv)
{
    int i;
    int state_num;
    int deviceID = 0;
    FILE *fpin;
    FILE *fpout;
    int count=0;
 
    if(argc!=3){
       printf("using command %s pattern_file input_file\n",argv[0]);
       exit(0);
    }
    state_num = create_table(argv[1]);

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
    PFAC_CPU(input_string, match_result, input_size);
    //printf("[2]\n");

    // Output results
    fpout = fopen("PFAC_CPU_OMP_match_result.txt", "w");
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
