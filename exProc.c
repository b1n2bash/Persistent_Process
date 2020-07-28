#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/prctl.h>

void cycleChildren();
void * trackChildren(void *);

// C Programing Testing Background 
// Process creation

int main(){

    // Create a thread that will keep track of the childs status 
    // This will be controlled by the parent so the parent can restart the process
    pthread_t thread_id1; 
    pthread_t thread_id2; 


    // Renaming Children Processes
    const char * child1 = "child_1";
    const char * child2 = "child_2";

    // Create two children process'
    pid_t pid1, pid2;
    pid1 = fork();

    // Parent will be the controller for its children
    // Parent makes sure the kids aren't dead, if they are it restarts them.
    // Children dont do anything but spin for now.
    if (pid1 == 0){ 
        printf("In Child Process 1...\n"); 
	if(prctl(PR_SET_NAME, (unsigned long) child1) < 0)
		perror("Error setting process name for child1");
	cycleChildren();
    } else { 
	// Spawn Child 2
	pid2 = fork();
    	if(pid2 == 0){ 
        	printf("In Child Process 2...\n"); 
		if(prctl(PR_SET_NAME, (unsigned long) child2) < 0)
		    perror("Error setting process name for child2");
		cycleChildren();
	} else {
            printf("In Parent Process...\n");
	    pthread_create(&thread_id1, NULL, trackChildren, (void *) &pid1);
	    pthread_create(&thread_id2, NULL, trackChildren, (void *) &pid2);
	    pthread_join(thread_id1, NULL);
	    pthread_join(thread_id2, NULL);
	}
    }
    return 0;
} // End of Main


void * trackChildren(void * pastPid){
	int * childPid = (int *) pastPid;
	// printf("Child: %d\n", *childPid);
	
	// Process name to set restarted child to
        const char * rName = "revived";

	int stat;
	pid_t watchChild = waitpid(*childPid, &stat, 0); 
	if(WIFEXITED(stat)){
		printf("PARENT (%d): Child %d terminated with status: %d\n", getpid(), watchChild, WEXITSTATUS(stat));
	}else if(WIFSIGNALED(stat)){
		printf("PARENT (%d): Child %d terminated according to WIFSIGNALED\n", getpid(), *childPid);
		pid_t restart = fork();
		// In Child
		if(restart == 0){
		    if(prctl(PR_SET_NAME, (unsigned long) rName) < 0)
		        perror("Error setting process name for revived child process");
		    cycleChildren(); 
		// In Parent
		} else {
		    // Recall the tracking function with a new pid to track...
		    trackChildren((void *) &restart);
		}
	}
	return NULL;
}


// Cycle Child Process' Forever
void cycleChildren(){
	// Sleep for one second, then ping console with msg
	// int i = 0;
	do { // Cycle Forever, dont really do anything
	  // i++;
	  // printf("Child %d: %d\n", getpid(), i);
	  printf("I'm Alive! I am Child %d\n", getpid());
	  sleep(1);
	} while(1);

}
