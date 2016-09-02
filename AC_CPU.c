#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#define STATE_NUM 131000
#define CHAR_NUM 256
#define PATTERN_LEN 10000

//#define DEBUG

int AC_table[STATE_NUM][CHAR_NUM];	//for AC algorithm
int DFA_table[STATE_NUM][CHAR_NUM];			//for DFA
int failure_table[STATE_NUM];
int output_table[STATE_NUM];
int pattern_len[11000];

void enterpattern(char []);
void constructfailure(void);
void constructDFA(void);

int *match_result;


int newstate=0;
int pattern_num=0;

int main(int argc, char **argv){
	
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

	int state_num=0;
	int state;
	int pos;
	int nState;
	int count=0;
 
   
	if(argc!=3)
		printf("using command: %s pattern_file input_stream\n",argv[0] );
	
	if((fptr=fopen(argv[1],"r"))==NULL){ //reading ASCII text
		printf("Pattern file cannot be open.\n");
		exit(1);
	} 
	
	if((inputPtr=fopen(argv[2],"rb"))==NULL){
       printf("Input stream file cannot be opened.\n");
       exit(1);
	}
  
 
	if((resultPtr=fopen("AC_CPU_match_result.txt","w"))==NULL){
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
  
    // initialize output memory
    for (i = 0; i < lSize; i++) {
        match_result[i] = 0;
        //match_len[i] = 0;
    }
	

	pattern_num=1;
	
	fgets(string,PATTERN_LEN,fptr);
	while(!feof(fptr)){
		 
		 int len=strlen(string);

		 
		 if ( '\n' == string[len-1] ){
 	          len-- ;
 	   }
		 
	   pattern_len[pattern_num]=len;
     //printf("pattern %s of length %d\n", string, len);
 	   enterpattern(string);	
	   fgets(string,PATTERN_LEN,fptr);
	   pattern_num++;
	}

	printf("The total state is %d\n",newstate);
  
	
	//for all character a such that g(0,a)=fail do g(0,a) <-0
	for(i=0;i<CHAR_NUM;i++){				
		if(AC_table[0][i]==-1)
			AC_table[0][i]=0;
	}
	

  constructfailure();
	
#ifdef DEBUG 
	printf("\nState\tChar\tGoto\n");
	for(i=0;i<=newstate;i++){
		for(j=0;j<CHAR_NUM;j++){
			if(AC_table[i][j]!=-1 && AC_table[i][j]!=0){
				printf("%d\t%c\t%d\n",i,j,AC_table[i][j]);
				transition++;
			}
		}
	}

	printf("\nState\tOutput\n");
  for(i=0;i<=newstate;i++){
	   if(output_table[i]!=0){
	      printf("%d\t%d\n",i,output_table[i]);
 	   }
 	}
	
	printf("\nState\tFailure\n");
  for(i=0;i<=newstate;i++){
	   //if(failure_table[i]!=0){
	      printf("%d\t%d\n",i,failure_table[i]);
	   //}
	}	
#endif


/**************AC matching process********************/
      
    // start time
    gettimeofday(&t_start, NULL);

     //matching by AC 
     
     state=0;
     int matchVector;
     for(pos=0;pos<lSize;pos++){
     	nState=AC_table[state][buffer[pos]]; 	  
        while(nState==-1){        	
           nState=failure_table[state];
           if((matchVector=output_table[nState])!=0){
        	   match_result[pos-pattern_len[matchVector]] = matchVector;                     
           }
           state=nState;   
           nState=AC_table[state][buffer[pos]];                                        
        }        
             
        //state=AC_table[state][buffer[pos]];
        state=nState;

        if((matchVector=output_table[state])!=0){                	
            match_result[pos+1-pattern_len[matchVector]] = matchVector;    
        }
                
        int fstate=state;
        while((fstate=failure_table[fstate])!=0){
           if((matchVector=output_table[fstate])!=0){
        	    match_result[pos+1-pattern_len[matchVector]] = matchVector;    
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
    printf("The throughput is %f Gbps\n",(float)lSize/(elapsedTime/8*1000000) );


       // Output match result to file
        for (i = 0; i < lSize; i++) {
           if (match_result[i] != 0) {
              fprintf(resultPtr, "At position %4d, match pattern %d\n", i, match_result[i]);
			  count++;
           }
        }
     
	    printf("The number of matched pattern is %d\n",count );
    
/********************************************/
    free(buffer);
    free(match_result);
    
    fclose(fptr);
    fclose(inputPtr);
    fclose(resultPtr);

	return 0;

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
	int queue[STATE_NUM];
	int state;
	int temp;
	int start=0;
	int nState;

	
	for(i=0;i<STATE_NUM;i++)
		queue[i]=0;

	for(i=0;i<CHAR_NUM;i++){				
		if((nState=AC_table[0][i])!=0){
		   index++;
		   queue[index]=nState;
		   failure_table[nState]=0;
		}
	}

	while(start<=index){
	
		temp=queue[start];
	
		for(i=0;i<CHAR_NUM;i++){
		   if((nState=AC_table[temp][i])!=-1){
		    index++;
        queue[index]=nState; 
			  state=failure_table[temp];
				while((AC_table[state][i])==-1){	
					state=failure_table[state];
				}
				failure_table[nState]=AC_table[state][i];
			
			}
		}
		start++;
	}
}


