/*****************************************************************************
modified: 2010/09/01
pattern file: ASCII format
產生的table與振雄相同
modified: 2010/09/15
加入AC matching 功能
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <omp.h>
#define STATE_NUM 131000
#define CHAR_NUM 256
#define PATTERN_LEN 10000

#define OUTPUT_MATCHED_FILE   "AC_CPU_OMP_match_result.txt"


int AC_table[STATE_NUM][CHAR_NUM];	//for AC algorithm
int DFA_table[STATE_NUM][CHAR_NUM];			//for DFA
int failure_table[STATE_NUM];
int output_table[STATE_NUM];
int pattern_len[11000];

void enterpattern(char []);
void constructfailure(void);
void constructDFA(void);
void convert2pat(char*, char*);



int *match_result;
//int *match_len;

int newstate=0;
int pattern_num=0;

int main(int argc, char **argv)
{
	
    FILE* fptr;
    FILE* fptr_w_ac;
    FILE* fptr_w_trie;
    FILE* fptr_w_dfa;
    FILE* inputPtr;
    FILE* resultPtr;
    char * buffer;
    size_t result;
    int lSize;
    struct timeval t_start, t_end;
    float elapsedTime;

    char string[PATTERN_LEN];
    char pat[PATTERN_LEN];
	
    int i=0;
    int j;
    int transition=0;
    int transition_dfa=0;
    int count=0;
    int state_num=0;
    int state;
    int pos;
 
    if ( 3 > argc ){
            printf("useage: [bin] [pattern] [input file] [dump to file] \n");
	    printf("default of [dump to file] is 1 (write match result to file) \n");
            printf("for benchmarking, set [dump to file] = 0 to save calibration time\n ");
	    return 0 ;
    }


    if(argc!=3)
        printf("./AC_CPU pattern_file input_stream\n");

	if((fptr=fopen(argv[1],"r"))==NULL){ //reading ASCII text
		printf("Pattern file cannot be open.\n");
		exit(1);
	} 

	if((fptr_w_trie=fopen("AC_Trie_table.txt","w"))==NULL){
		printf("File cannot be open");
		exit(1);
	} 
		
    if((fptr_w_ac=fopen("AC_table.txt","w"))==NULL){
		printf("File cannot be open");
		exit(1);
	} 

	if((fptr_w_dfa=fopen("DFA_table.txt","w"))==NULL){
		printf("File cannot be open");
		exit(1);
	} 
	
	if((inputPtr=fopen(argv[2],"rb"))==NULL){
        printf("Input stream file cannot be opened.\n");
        exit(1);
    }
  
    if((resultPtr=fopen( OUTPUT_MATCHED_FILE ,"w"))==NULL){
        printf("output file cannot be opened.\n");
        exit(1);
    }

    //initial ac array
	for (i=0;i<STATE_NUM;i++){
		for(j=0;j<CHAR_NUM;j++){
			AC_table[i][j]=-1;
		}
	}
    //initial dfa array
	for (i=0;i<STATE_NUM;i++){
		for(j=0;j<CHAR_NUM;j++){
			DFA_table[i][j]=-1;
		}
	}

	//initial failure and output array
	for (i=0;i<STATE_NUM;i++){
		failure_table[i]=0;
		output_table[i]=0;
	}
	
   // obtain file size:
	 fseek (inputPtr , 0 , SEEK_END);
     lSize = ftell (inputPtr);
     rewind (inputPtr);
     
	 // allocate memory to contain the whole file:
	 buffer = (char*) malloc (sizeof(char)*lSize);
     if (buffer == NULL){
        printf("Memory error"); 
        exit (2);
     }
     
     // copy the file into the buffer:
     result = fread (buffer,1,lSize,inputPtr);
     if (result != lSize) {
        fputs ("Reading error",stderr); 
        exit (3);
     }

	 // allocate memory for output
    match_result = (int *) malloc (sizeof(int)*lSize);
    if (match_result == NULL){
        perror("Allocate output memory error");
        exit (1);
    }
    /*
    match_len = (int *) malloc (sizeof(int)*lSize);
    if (match_len == NULL){
        perror("Allocate output memory error");
        exit (1);
    }
    */
    // initialize output memory
    for (i = 0; i < lSize; i++) {
        match_result[i] = 0;
        //match_len[i] = 0;
    }
	
//	creating AHO trie
	pattern_num=1;
	
	fgets(string,PATTERN_LEN,fptr);
	while(!feof(fptr)){
	int len=strlen(string);

		 
		 if ( '\n' == string[len-1] ){
 	          len-- ;
 	   }
		 
	   pattern_len[pattern_num]=len;
    enterpattern(string);	
		fgets(string,PATTERN_LEN,fptr);
		pattern_num++;
	}


                                    
	for(i=0;i<=newstate;i++){
		for(j=0;j<CHAR_NUM;j++){
		   fprintf(fptr_w_trie,"%d ",AC_table[i][j]);
		}
		fprintf(fptr_w_trie,"%d\n",output_table[i]);
	}
	
  //for all character a such that g(0,a)=fail do g(0,a) <-0
  for(i=0;i<CHAR_NUM;i++){				
		if(AC_table[0][i]==-1)
			AC_table[0][i]=0;
	}
	

  constructfailure();
	
	
  for(i=0;i<=newstate;i++){
		for(j=0;j<CHAR_NUM;j++){
		   fprintf(fptr_w_ac,"%d ",AC_table[i][j]);
		}
       fprintf(fptr_w_ac,"%d %d\n",output_table[i], failure_table[i]);// The last two value are output and failure, respectively
	}



	constructDFA();

	
 for(i=0;i<=newstate;i++){
		for(j=0;j<CHAR_NUM;j++){
		   fprintf(fptr_w_dfa,"%d ",DFA_table[i][j]);
		}
        fprintf(fptr_w_dfa,"%d \n",output_table[i]);
	}
       


/**************AC matching process********************/
      


    //matching by AC 
    int NUM_THREADS = 8 ;
    int segmentSize = (lSize + NUM_THREADS-1) / NUM_THREADS ;
    int remainder = 256 ;
    int end ;


    printf("lSize=%d, segmentSize=%d, NUM_THREADS=%d, remainder=%d\n",
        lSize, segmentSize, NUM_THREADS, remainder );

#define iMIN(x,y) ((x)>(y))? (y):(x) 
int fstate;	
int matchVector;
omp_set_num_threads(NUM_THREADS); 

    // start time
    gettimeofday(&t_start, NULL);
    
#pragma omp parallel for private (j, pos, end, state, fstate, matchVector) \
shared (segmentSize, remainder, pattern_len, NUM_THREADS, lSize, output_table, match_result, buffer, AC_table, failure_table) num_threads(8) schedule(static,8)
    //#pragma omp for schedule(static,1)
    for(j = 0 ; j < NUM_THREADS ; j++ ){    
        pos = segmentSize * j ;
        end = pos + segmentSize + remainder ;
        end = iMIN(end, lSize) ;
        state = 0 ;
        for( ; pos < end ; pos++){
           while(AC_table[state][buffer[pos]]==-1){
              state=failure_table[state];
              if(output_table[state]!=0){
        	       match_result[pos-pattern_len[output_table[state]]] = output_table[state];    
              }                                            
           }        
             
           state=AC_table[state][buffer[pos]];
           if(output_table[state]!=0){
               match_result[pos+1-pattern_len[output_table[state]]] = output_table[state];    
           } 
           
           fstate=state;
             while((fstate=failure_table[fstate])!=0){
                if((matchVector=output_table[fstate])!=0){
               	    match_result[pos+1-pattern_len[matchVector]] = matchVector;    
                }
             }

        }
    }     
     
      
    // stop time
    gettimeofday(&t_end, NULL);
    // compute and print the elapsed time in millisec
    elapsedTime = (t_end.tv_sec - t_start.tv_sec) * 1000.0;
    elapsedTime += (t_end.tv_usec - t_start.tv_usec) / 1000.0;
    printf("The input size is %d\n", lSize);
    printf("The elapsed time is %f ms\n", elapsedTime);
    printf("The throughput is %f Gbps\n",(float)lSize/(elapsedTime*1000000)*8 );

    
    int dump2file = 1 ;
    if (4 == argc ) {
       if ( isdigit(argv[3][0]) ){
           dump2file = argv[3][0] - '0' ;
       }
    }

    if ( dump2file ){

    // Output match result to file
        for (i = 0; i < lSize; i++) {
            if (match_result[i] != 0) {
                fprintf(resultPtr, "At position %4d, match pattern %d\n", i, match_result[i]);
                count++;
            }
        }
    } else {
        printf("don't dump file \n");
    }
    printf("The number of matched pattern is %d\n",count );
/********************************************/

    
    fclose(fptr);
    fclose(fptr_w_trie);
    fclose(fptr_w_ac);
    fclose(fptr_w_dfa);
    fclose(inputPtr);
    fclose(resultPtr);
	free(buffer);
    free(match_result);
	return 0;

}


void convert2pat(char* string, char* pattern){
	char * tokenptr;
	int i=0;
	tokenptr=strtok(string," ");
	while(tokenptr!=NULL){
		pattern[i]=atoi(tokenptr);
		i++;
		tokenptr=strtok(NULL, " ");

	}
}

//Function enterpattern

void enterpattern(char pattern[]){
	int i=0;
	int state=0;

	while(AC_table[state][pattern[i]]!=-1){
		state=AC_table[state][pattern[i]];			//state<-g(state,aj);
		i++; 
	}

	while(pattern[i]!='\n'){
		newstate++;								//newstate=newstate+1;
		AC_table[state][pattern[i]]=newstate;			//g(state,ap)=newstate;
		state=newstate;
		i++;
	}
	output_table[state]=pattern_num;

}



//Function constructfailure

void constructfailure(void){
	int i;
	int index=-1;
	int queue[STATE_NUM*2];
	int state;
	int temp;
	int start=0;


	
	for(i=0;i<STATE_NUM;i++)
		queue[i]=0;

	for(i=0;i<CHAR_NUM;i++){				
		if(AC_table[0][i]!=0){
			index++;
			queue[index]=AC_table[0][i];
			failure_table[AC_table[0][i]]=0;

		}
	}

	while(start<=index){
	
		temp=queue[start];
	
		for(i=0;i<CHAR_NUM;i++){
			if(AC_table[temp][i]!=-1){
				index++;
				queue[index]=AC_table[temp][i]; //enqueue
				state=failure_table[temp];
				while(AC_table[state][i]==-1){	//bug is here
					state=failure_table[state];
				}
				failure_table[AC_table[temp][i]]=AC_table[state][i];	//f(4)=1
				
			
			}
		}
		start++;
	}
}


void constructDFA(void){
	int queue_dfa[STATE_NUM];
	int i;
	int index=-1;
	int temp;
	int start=0;

	for(i=0;i<STATE_NUM;i++)
		queue_dfa[i]=0;

	for (i=0;i<CHAR_NUM;i++){	
		DFA_table[0][i]=AC_table[0][i]; 
		
		if(AC_table[0][i]!=0){
			index++;
			queue_dfa[index]=AC_table[0][i];
		}
	}

	while(start<=index){
		temp=queue_dfa[start];
		
		for(i=0;i<CHAR_NUM;i++){ 
			if(AC_table[temp][i]!=-1){
				index++;
				queue_dfa[index]=AC_table[temp][i];//push s
				DFA_table[temp][i]=AC_table[temp][i]; 
 
			}
			else{
				DFA_table[temp][i]=DFA_table[failure_table[temp]][i];
				
			}
		}
		start++;
	}
}



