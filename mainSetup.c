#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
 

struct command{
   char aliasName[50];
   char commandName[50];
   char command[50];
   struct command *nextNode;
};

struct backgroundCommand{
    char name[50];
    pid_t pid;
    struct backgroundCommand *nextNode;
};



struct command* createNewNode(char *aliasName,char *commandName, char *command)
{
	struct command *newNode = (struct command *) malloc(sizeof(struct command));
	strcpy(newNode->commandName,commandName);
	strcpy(newNode->aliasName,aliasName);
	strcpy(newNode->command,command);
	newNode->nextNode = NULL;
	return newNode;
}

struct backgroundCommand* createNewBG(char *commandName, pid_t pid)
{
    struct backgroundCommand *newNode = (struct backgroundCommand *) malloc(sizeof(struct backgroundCommand));
    strcpy(newNode->name,commandName);
    newNode->pid = pid;
    newNode->nextNode = NULL;
    return newNode;
}






/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

int numberOfCommand;
pid_t fgID;

void setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */
    	
    ct = 0;
        
    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);  

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

/* the signal interrupted the read system call */
/* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
	exit(-1);           /* terminate with error code of -1 */
    }

	printf(">>%s<<",inputBuffer);
    for (i=0;i<length;i++){ /* examine every character in the inputBuffer */

        switch (inputBuffer[i]){
	    case ' ':
	    case '\t' :               /* argument separators */
		if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
		    ct++;
		}
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
		start = -1;
		break;

            case '\n':                 /* should be the final char examined */
		if (start != -1) {
           // if (inputBuffer[i-1] != '&') {
                args[ct] = &inputBuffer[start];
                ct++;
            //}
		}
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
		break;

	    default :             /* some other character */
		    if (start == -1)
		        start = i;
                if (inputBuffer[i] == '&'){
		            *background  = 1;
                    inputBuffer[i-1] = '\0';
		    }
	    } /* end of switch */
     }    /* end of for */
     args[ct] = NULL; /* just in case the input line was > 80 */

	for (i = 0; i <= ct; i++)
        printf("args %d = %s\n", i, args[i]);




    numberOfCommand=ct;

} /* end of setup routine */


int findingCommandPath(char *pathArray[128], char *temp[111], char *args ) //find path and decide to command exist or not
{

    int z,k;
    int boolCommand=0;
    FILE *file;
	char arr[200];
	strcpy(arr,getenv("PATH"));
	const char *env = arr;
	char *exist;
	pathArray[0] = "/bin";
	
	for(k=1;(exist = strsep(&env,":")) != NULL;k++){
		pathArray[k]= exist;
	}
		      
	  for (z = 0; z<10 ;  z++)
        {
            strcpy(temp, pathArray[z]);
            strcat(temp, "/");
            strcat(temp, args);
            file = fopen(temp, "r"); // If the file exist, use this path to exec
            if (file)
            {
                fclose(file);
                boolCommand = 1;
                break;
            }


        }
        if (!boolCommand) {
            perror("\nError: Command not found.\n");
			return 0;
        }
		return 1;
}

struct command* insert(struct command *header,char *aliasName,char *commandName, char *command) //insert linked list op for alias list
{
	struct command *iterate;
	char *pathArray[128];
 	char *temp[111];
		
		
	
		//if LL empty
	if(header == NULL) 
	{
		header = createNewNode(aliasName,commandName,command);
		return header;
	} 
	
	
	//if LL not empty
	else
	{
		iterate = header;
		while(iterate != NULL)
		{
			if(iterate->nextNode == NULL){
				iterate->nextNode = createNewNode(aliasName,commandName,command);
				return header;
			}			
			iterate = iterate->nextNode ;
		}
	}
	return header;	
}

struct backgroundCommand* insertBg(struct backgroundCommand *header,char *commandName, pid_t pid){

    struct backgroundCommand *iterate;
    //if LL empty
    if(header == NULL)
    {
        header = createNewBG(commandName,pid);
        return header;
    }


        //if LL not empty
    else
    {
        iterate = header;
        while(iterate != NULL)
        {
            if(iterate->nextNode == NULL){
                iterate->nextNode = createNewBG(commandName,pid);
                return header;
            }
            iterate = iterate->nextNode ;
        }
    }
    return header;
} //insert linked list op for bg process


struct command* deleteAlias(struct command *header,char *aliasName){

    struct command *iterate,*prev;
	
	//if LL empty 
	if(header == NULL) {
		printf("There is no command in alias\n");
		return header;
	}
	
	
	//if LL not empty
	else
	{
		/*linked list deleted first elemet*/
		iterate = header;
		prev = iterate->nextNode;
		if(prev == NULL) {
            if (!strcmp(aliasName, header->aliasName)) {
                header = NULL;
                free(iterate);
                iterate = NULL;
                return header;
            }
        }
		else {
            if (!strcmp(aliasName, header->aliasName)) {
                header = header->nextNode;
                free(iterate);
                iterate = NULL;
                return header;
            }
        }

		while(iterate != NULL)
		{	
			if(!strcmp(aliasName,iterate->aliasName)){
				if(iterate->nextNode == NULL){
					prev->nextNode = NULL;
					free(iterate);
					iterate = NULL;
					printf("There is no command in alias\n");
					return header;
				}	
				prev->nextNode = iterate->nextNode;
				free(iterate);
				iterate = NULL;
				return header;
			}			
			prev = iterate;			
			iterate = iterate->nextNode ;
			
		}
	}
	return header;	
	
	
} //delete given alias name in alias list

struct backgroundCommand* deleteBg(struct backgroundCommand *header,pid_t pid){

    struct backgroundCommand *iterate,*prev;

    //if LL empty
    if(header == NULL) {
        return header;
    }


        //if LL not empty
    else {
        /*linked list deleted first elemet*/
        iterate = header;
        prev = iterate->nextNode;
        if (prev == NULL) {
            if (pid == header->pid) {
                header = NULL;
                free(iterate);
                iterate = NULL;
                return header;
            }
         }
        else {
            if (pid == header->pid) {
                header = header->nextNode;
                free(iterate);
                iterate = NULL;
                return header;
            }
        }

        while(iterate != NULL)
        {
            if(pid == header->pid){
                if(iterate->nextNode == NULL){
                    prev->nextNode = NULL;
                    free(iterate);
                    iterate = NULL;
                    return header;
                }
                prev->nextNode = iterate->nextNode;
                free(iterate);
                iterate = NULL;
                return header;
            }
            prev = iterate;
            iterate = iterate->nextNode ;

        }
    }
    return header;


} //delete given pid in bg list

struct command* search(struct command *header,char *aliasName) //search given alias name in list
{
	
	struct command *iterate;
	iterate = header;
	
	if(iterate == NULL){
		return NULL;
	}
	else{
		while(iterate != NULL)
		{			
			if(!strcmp(iterate->aliasName,aliasName)) 
				return iterate;// search successful
			iterate = iterate->nextNode;
		}
	}
	return NULL;
		 
}


void printAliasList(struct command *header) //print all list for alias list
{
	
	struct command *iterate;
	iterate = header;
	
	if(iterate == NULL){
		printf("Alias list is empty\n");
		return;
	}
	else{
		while(iterate != NULL)
		{			
			printf("%s %s\n",iterate->aliasName,iterate->commandName);
			iterate = iterate->nextNode;
		}
	}
		
}




struct backgroundCommand* checkBg(struct backgroundCommand *header){

    struct backgroundCommand *iterate;

    if (header == NULL) return header;

    iterate = header;
    while (iterate != NULL){
        if(waitpid(iterate->pid,NULL,WNOHANG))
            header = deleteBg(header,iterate->pid);
        iterate = iterate->nextNode;
    }
    return header;
}  //check background process running or bot

struct backgroundCommand* fgCommand(struct backgroundCommand *header) {

    int i = 0;
    struct backgroundCommand *iterate;

    if (header == NULL) {
        return header;
    }
    iterate = header;
    printf("\nBackgrounds process [%d]%d   [%d]%s   move to foreground\n\n", i, iterate->pid, i, iterate->name);
    while (iterate != NULL) {

        if (waitpid(iterate->pid,NULL,0))
            header = deleteBg(header, iterate->pid);
        iterate = iterate->nextNode;
        if (iterate == NULL) break;
        i++;
        printf("Backgrounds process [%d]%d  [%d]%s   move to foreground\n\n", i, iterate->pid,i, iterate->name);
    }
    return header;
} //fg command operations check linked list and wait you to close

struct command* createAlias( char *args[],struct command *header)
{
	int i;
	char *pathArray[128];
	char *temp[111];
	char *command,*command1,*command2 ;
	char temp_commandName[111];
	char temp_command[111];
	
	command1 = strdup(args[1]);
	strcpy(temp_command,command1);
	
	strsep(&command1,"\"");
	
	if(numberOfCommand == 3)
		command2 = strsep(&command1,"\"");

	if(numberOfCommand > 3)
		command2 = strsep(&command1,"\0");
	
	strcpy(temp_commandName,strdup(args[1]));
	strcpy(temp_command,command2);

	
	if(numberOfCommand == 3){
		if(!findingCommandPath(pathArray, temp, command2)) {
			return header;
		} 
		header = insert(header,args[2],args[1],temp_command);
	}
	
	if(numberOfCommand > 3)
	{
		if(!findingCommandPath(pathArray, temp, command2)) 
			return header;
		strcat(temp_commandName," ");
		strcat(temp_command," ");
		for(i=2; i<numberOfCommand-1; i++){
			if(i==(numberOfCommand-2)){
				strcat(temp_commandName,args[i]);
				strcat(temp_command,strtok(args[i],"\""));
				break;
			}
			strcat(temp_commandName,args[i]);
			strcat(temp_commandName," ");
			strcat(temp_command,args[i]);
			strcat(temp_command," ");
		}	
		header = insert(header,args[numberOfCommand-1],temp_commandName,temp_command);
	}
		
	return header;	
}




void redirection(char *args[]){ // redirection operations with dup2
    int d,argNumber=-1;
    int fd,fd_new;
    char *pathArray[128];
    char *temp[111];

    for(d=0;args[d]!='\0';d++) {

        if((strcmp(args[d],"<")==0))
        {
            argNumber = d - 1;
            fd = open(args[d+1], O_RDONLY , 0600);
            dup2(fd,STDIN_FILENO); //fd to STDIN
            close(fd);
        }

        else if ((strcmp(args[d], ">") == 0)) {
            argNumber = d - 1;

            fd = open(args[d + 1],  O_CREAT | O_WRONLY, 0600);

            dup2(fd, STDOUT_FILENO); //fd to STDIN

            close(fd);
        }
        else if((strcmp(args[d],">>")==0)) {
            argNumber = d - 1;
            fd = open(args[d+1],  O_CREAT | O_WRONLY | O_APPEND , 0600);
            dup2(fd,STDOUT_FILENO); //fd to STDOUT
            close(fd);

        }
        else if((strcmp(args[d],"2>")==0)) {
            argNumber = d - 1;
            fd = open(args[d+1],   O_WRONLY | O_CREAT | O_APPEND , 0600);
            dup2(fd,STDERR_FILENO); //fd to STDERR
            close(fd);

        }
        else if((strcmp(args[d],"<")==0) && (strcmp(args[d+2],">") ==0)){

            argNumber = d - 1;
            fd = open(args[d+1], O_RDONLY , 0600);
            dup2(fd,STDIN_FILENO);
            close(fd);
            fd_new = open(args[d+3], O_CREAT | O_WRONLY, 0600);
            dup2(fd_new,STDOUT_FILENO);
            close(fd_new);

        }

    }


     if(findingCommandPath(pathArray, temp, args[0])) {
         if(argNumber==-1)return;
         //how many argumans there are in line
         switch (argNumber) {
             case 0:
                 execl(temp, args[0], NULL);
                 break;
             case 1:
                 execl(temp, args[0], args[1], NULL);
                 break;
             case 2:
                 execl(temp, args[0], args[1], args[2], NULL);
                 break;
             case 3:
                 execl(temp, args[0], args[1], args[2], args[3], NULL);
                 break;
             case 4:
                 execl(temp, args[0], args[1], args[2], args[3], args[4], NULL);
                 break;
             case 5:
                 execl(temp, args[0], args[1], args[2], args[3], args[4], args[5], NULL);
                 break;
         }
     }
}




int main(void)
{


    char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
    int background; /* equals 1 if a command is followed by '&' */
    char *args[32]; /*command line arguments */
	char *pathArray[128];
	int status;
	pid_t childpid;
	char *temp[111];
	struct command *pt=NULL,*searchTemp=NULL; //alias linked list
	struct backgroundCommand *header=NULL; //background process linked list
	int i;


	system("clear");


	while (1){

	    background = 0;

         printf("myshell: ");
			
			
		fflush(stdout);
	
		setup(inputBuffer, args, &background);

		header = checkBg(header);

        for(i=0; args[i]!= NULL; i++);

        if(i!=0)if(!strcmp(args[i-1],"&")) args[i-1]=NULL; // this remove & sign

        if(!args[0]) continue; // if args null return back myshell:

        //above part for B section (alias operations , fg , exit , clr)
        else if(!strcmp("clr",args[0]))
		{
			system("clear");
		}
		
		else if(!strcmp("alias",args[0])){
			if(!strcmp("-l",args[1])){
				printAliasList(pt);
			}
			else
                pt = createAlias(args,pt);
		}
		
		else if(!strcmp("unalias",args[0]))
		{
			pt = deleteAlias(pt,args[1]);
		}
		
		else if((searchTemp=search(pt,args[0])))
		{
			system(searchTemp->command);
		}
		else if(!strcmp("fg",args[0])){
			header = fgCommand(header);
		}
		else if(!strcmp("exit",args[0])){
		    if(header)continue;
		    exit(0);
		}


        else {//this block for A and C section (with fork())
            if (!findingCommandPath(pathArray, temp, args[0])) continue; //control of command exist or not


            childpid = fork();

            if (background == 1 && childpid != 0)       //parent code (background process track - insert in linked list)
                header = insertBg(header, args[0],childpid);

            if (childpid == -1) { //fork error detection
                perror("Failed to fork");
                return 1;
            }
            if (childpid == 0) {    // child code
                redirection(args);//if redirection child process run execl func in this function
                execl(temp, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], //our runing command
                      args[10], args[11],args[12], args[13], args[14], args[15], args[16], args[17], args[18],
                      args[19], args[20],
                      args[21],args[22], args[23], args[24], args[25], args[26], args[27], args[28], args[29], args[30],
                      args[31], NULL);
                perror("Child failed to exec command");
                return 1;
            }
            if (background == 0) { // if process foreground process this "if" exeute
                if (childpid != waitpid(childpid,NULL, 0 )) {   //parent code / wait currently working process from its pid
                    perror("Parent failed to wait due to signal or error");
                    return 1;
                }
            }

        }

                       
   }

}
