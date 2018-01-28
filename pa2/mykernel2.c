/* mykernel2.c: your portion of the kernel
 *
 *   	Below are procedures that are called by other parts of the kernel. 
 * 	Your ability to modify the kernel is via these procedures.  You may
 *  	modify the bodies of these procedures any way you wish (however,
 *  	you cannot change the interfaces).  
 */

#include "aux.h"
#include "sys.h"
#include "mykernel2.h"

#define TIMERINTERVAL 1	// in ticks (tick = 10 msec)

/* 	A sample process table. You may change this any way you wish.  
 */

static int unRequested;
static struct {
	int valid;		// is this entry valid: 1 = yes, 0 = no
	int pid;		// process ID (as provided by kernel)
    int next;
    int request;
    int stride;
    int pass;
} proctab[MAXPROCS];

static int head;
static int tail;
static int current;

static struct {
    int arr[MAXPROCS];
    int head;
    int tail;
    int count;
} proQueue;

static struct {
    int arr[MAXPROCS];
    int top;
} proStack;


int peek() {
    return proQueue.arr[proQueue.head];
}
void enQueue(int proIndex) {
    if (proQueue.count != MAXPROCS){
        if(proQueue.tail == MAXPROCS-1) {
            proQueue.tail = -1;
        }
            proQueue.arr[++proQueue.tail] = proIndex;
            proQueue.count++;
        
    }
}
int deQueue() {
    int index = proQueue.arr[proQueue.head++];

    if(proQueue.head == MAXPROCS) {
        proQueue.head = 0;
    }

    proQueue.count--;
    return index;
}

void push(int proIndex) {
    if(proStack.top < MAXPROCS) {
        proStack.arr[++proStack.top] = proIndex;
    }
}

int pop() {
    int i = proStack.arr[proStack.top];
    if(proStack.top >= 0) {
        proStack.top--;
    }
    return i;
}

int top() {
    return proStack.arr[proStack.top];
}

void noRequestPro(int restCPU) {
    int restStride;
    if (restCPU == 0) {
        restStride = 0;
    }
    else {
        restStride = 100000/restCPU;
    }
    for (int i = 0; i < MAXPROCS; i++) {
        if (proctab[i].request == 0) {
            proctab[i].stride = restStride;
        }
    }
}

void resetPassValue(int min) {
    for (int i = 0; i < MAXPROCS; i++) {
        if (proctab[i].valid) {
            proctab[i].pass -= min;
        }
    }
}

/*  	InitSched () is called when kernel starts up. First, set the
 * 	scheduling policy (see sys.h). Make sure you follow the rules
 *   	below on where and how to set it.  Next, initialize all your data
 * 	structures (such as the process table).  Finally, set the timer
 *  	to interrupt after a specified number of ticks. 
 */

void InitSched ()
{
	int i;
    head = 0;
    tail = 0;
    current = 0;
    unRequested = 100;
	/* First, set the scheduling policy.  You should only set it
	 * from within this conditional statement. While you are working
	 * on this assignment, GetSchedPolicy () will return NOSCHEDPOLICY. 
	 * Thus, the condition will be true and you may set the scheduling
	 * policy to whatever you choose (i.e., you may replace ARBITRARY).  
	 * After the assignment is over, during the testing phase, we will
	 * have GetSchedPolicy () return the policy we wish to test.  Thus
	 * the condition will be false and SetSchedPolicy (p) will not be
	 * called, thus leaving the policy to whatever we chose to test. 
	 */
	if (GetSchedPolicy () == NOSCHEDPOLICY) {	// leave as is
		//SetSchedPolicy (ROUNDROBIN);		// set policy here
        //SetSchedPolicy (ARBITRARY);       // set policy here
        //SetSchedPolicy (FIFO);            // set policy here
        //SetSchedPolicy (LIFO);            // set policy here
        SetSchedPolicy(PROPORTIONAL);
	}
		
	/* Initialize all your data structures here */
	for (i = 0; i < MAXPROCS; i++) {
		proctab[i].valid = 0;
        proctab[i].next = -1;
        proctab[i].request = 0;
        proctab[i].stride = 0;
        proctab[i].pass = 0;
	}
    
    proQueue.head = 0;
    proQueue.tail = -1;
    proQueue.count = 0;

    proStack.top = -1;
	/* Set the timer last */
	SetTimer (TIMERINTERVAL);
}


/*  	StartingProc (p) is called by the kernel when the process
 * 	identified by PID p is starting.  This allows you to record the
 *  	arrival of a new process in the process table, and allocate
 * 	any resources (if necessary). Returns 1 if successful, 0 otherwise. 
 */

int StartingProc (p)
	int p;				// process that is starting
{
	int i;
    int minPass = 1000000;
    for (int j = 0; j < MAXPROCS; j++) {
        if (proctab[j].valid) {
            if (proctab[j].pass < minPass) {
                minPass = proctab[j].pass;
            }
        }
    }

	for (i = 0; i < MAXPROCS; i++) {
		if (! proctab[i].valid) {
			proctab[i].valid = 1;
			proctab[i].pid = p;
            if(GetSchedPolicy() == FIFO) {
                enQueue(i);
            }
            
            else if(GetSchedPolicy() == LIFO) {
                push(i);
                DoSched();
            }
            else if(GetSchedPolicy() == ROUNDROBIN) {
                proctab[tail].next = i;
                proctab[i].next = head;
                tail = i;
            }
            else if(GetSchedPolicy() == PROPORTIONAL) {
                if (minPass != 1000000) {
                    proctab[i].pass = minPass;
                }
                else {
                    proctab[i].pass = 0;
                }
                MyRequestCPUrate(p,0);
            }
            return (1);
		}
	}

	DPrintf ("Error in StartingProc: no free table entries\n");
	return (0);
}
			

/*   	EndingProc (p) is called by the kernel when the process
 * 	identified by PID p is ending.  This allows you to update the
 *  	process table accordingly, and deallocate any resources (if
 *  	necessary).  Returns 1 if successful, 0 otherwise. 
 */


int EndingProc (p)
	int p;				// process that is ending
{
	int i;
    int pre;
    
   // DPrintf("\nEnding Process[%d]...", p);
   // DPrintf ("\nHead: %d, Tail: %d, current: %d, next: %d\n",head, tail, current);

   // DPrintf ("proctab: ");
   // for(int j = 0; j < 10; j++)
     //   DPrintf ("%d  ",proctab[j].valid);
   // DPrintf("\n");
    
    if (GetSchedPolicy() == ROUNDROBIN) {
        i = head;
        pre = -1;
        while((i != -1) && !(proctab[i].valid && proctab[i].pid == p)){
            pre = i;
            i = proctab[i].next;
           // DPrintf("pre: %d, i: %d\n", pre, i);
        }
        if(i!=-1) {
          //  DPrintf("=========  i != -1, i = %d\n", i);
            proctab[i].valid = 0;
            if(i == head){
            //    DPrintf("=========  i == head\n");
                head = proctab[i].next;
                proctab[i].next = -1;
                proctab[tail].next = head;
              //  DPrintf("new head = %d\n", head);
            }
            else if(i == tail) {
             //   DPrintf("=========  i == tail\n");
                proctab[pre].next = proctab[tail].next;
                proctab[tail].next = -1;
                tail = pre;
              //  DPrintf("new tail = %d\n", tail);
            }
            else {
               // DPrintf("=========  i is neither head or tail\n");
                proctab[pre].next = proctab[i].next;
                proctab[i].next = -1;
            }
            return(1);
        }
    }
    
    else if (GetSchedPolicy() == PROPORTIONAL) {
        int target;
        int count = 0;
        for (i = 0; i < MAXPROCS; i++) {
            if (proctab[i].valid) {
                if (proctab[i].pid == p) {
                    target = i;
                }
                else if (proctab[i].request == 0) {
                    count++;
                }
            }
        }
        if(count != 0) {
            unRequested += proctab[target].request;
            int rest = unRequested / count;
            noRequestPro(rest);
        }
        proctab[target].valid = 0;
        //DPrintf("Ending Process: %d\n", p);
        return(1);
    }
    
    else {
	    for (i = 0; i < MAXPROCS; i++) {
		    if (proctab[i].valid && proctab[i].pid == p) {
			    proctab[i].valid = 0;
                pop();
                deQueue();
         /*       DPrintf ("QueueHead: %d\n",proQueue.head);
                DPrintf ("new proctab: ");
                for(int j = 0; j < 10; j++)
                    DPrintf ("%d ",proQueue.arr[j]);
                DPrintf("\n");
*/
                return (1);	 
            }
        }    
    }
    
	DPrintf ("Error in EndingProc: can't find process %d\n", p);
	return (0);
}


/* 	SchedProc () is called by kernel when it needs a decision for
 *  	which process to run next.  It calls the kernel function
 * 	GetSchedPolicy () which will return the current scheduling policy
 *   	which was previously set via SetSchedPolicy (policy). SchedProc ()
 * 	should return a process PID, or 0 if there are no processes to run. 
 */

int SchedProc ()
{
	int i;

	switch (GetSchedPolicy ()) {
        case ARBITRARY: {
                            for (i = 0; i < MAXPROCS; i++) {
                                if (proctab[i].valid) {
                                    return (proctab[i].pid);
			                    }
		                    }
                            break;
                        }
		    
	case FIFO: {
                i = peek();
               // DPrintf("Schedule: head: %d tail %d count %d arr: %d i: %d", proQueue.head, proQueue.tail, proQueue.count, proQueue.arr[1], i);
                if (proctab[i].valid) {
                    return (proctab[i].pid);
                }

                break;
                   }
	case LIFO: {

                i = top();
                //DPrintf("Schedule: i: %d, proStack.arr[i]: %d, top: %d\n", i, proStack.arr[i], proStack.arr[proStack.top]);

                if (proctab[i].valid) {
                    return (proctab[i].pid);
                }

                break;
                }
	case ROUNDROBIN: {
                    i = current;
                    current = proctab[current].next;
                    //DPrintf("\nAttempting to schedule another process: i = %d, current = %d...\n", i, current);
                    //DPrintf("Schedule process %d", i);
                    if (proctab[i].valid) {
                        //DPrintf("\nScheduled PID: %d\n",proctab[i].pid);
                        return (proctab[i].pid);
                    }
                    break;
                    }
	
	case PROPORTIONAL: {
                int minIndex = 11;
                int minPass = 1000000;
                for (int i = 0; i < MAXPROCS; i++) {
                    if (proctab[i].valid) {
                        if (proctab[i].request == 0 && unRequested == 0) {
                            continue;
                        }
                        if (proctab[i].pass <= minPass) {
                            minPass = proctab[i].pass;
                            minIndex = i;
                        }
                    }
                }
               // DPrintf("MinIndex: %d\n", minIndex);
                if (minIndex != 11) {
                    if (minPass + proctab[minIndex].stride > 1000000) {
                        resetPassValue(minPass);
                    }
                    proctab[minIndex].pass += proctab[minIndex].stride;
                    return (proctab[minIndex].pid);
                }
                break;
                       }
    }
	return (0);
}


/*  	HandleTimerIntr () is called by the kernel whenever a timer
 *  	interrupt occurs.  Timer interrupts should occur on a fixed
 * 	periodic basis.
 */

void HandleTimerIntr ()
{
	SetTimer (TIMERINTERVAL);

	switch (GetSchedPolicy ()) {	// is policy preemptive?

	case ROUNDROBIN:		// ROUNDROBIN is preemptive
	case PROPORTIONAL:		// PROPORTIONAL is preemptive

		DoSched ();		// make scheduling decision
		break;

	default:			// if non-preemptive, do nothing
		break;
	}
}

/*  	MyRequestCPUrate (p, n) is called by the kernel whenever a process
 * 	identified by PID p calls RequestCPUrate (n).  This is a request for
 *   	n% of CPU time, i.e., requesting a CPU whose speed is effectively
 * 	n% of the actual CPU speed. Roughly n out of every 100 quantums
 *  	should be allocated to the calling process.  n must be at least
 *  	0 and must be less than or equal to 100. MyRequestCPUrate (p, n)
 * 	should return 0 if successful, i.e., if such a request can be
 *  	satisfied, otherwise it should return -1, i.e., error (including if
 * 	n < 1 or n > 100). If MyRequestCPUrate (p, n) fails, it should
 *   	have no effect on scheduling of this or any other process, i.e., AS
 * 	IF IT WERE NEVER CALLED.
 */

int MyRequestCPUrate (p, n)
	int p;				// process whose rate to change
	int n;				// percent of CPU time
{
	/* your code here */
    if (n>=0 && n <= 100) {
        int targetIndex;
        int noReqPro = 0;
        int currentMinPass = 1000000;
        for (int i = 0; i < MAXPROCS; i++) {
            if (proctab[i].valid){
                if (proctab[i].pid == p) {
                    targetIndex = i;
                }
                if (proctab[i].pass < currentMinPass) {
                    currentMinPass = proctab[i].pass;
                }
                if (proctab[i].request == 0) {
                    noReqPro++;
                }
            }
        }
        int total = proctab[targetIndex].request + unRequested;
        if (n > total) {
            return (0);
        }
        unRequested = total - n;
        proctab[targetIndex].request = n;
        if (n == 0) {
            proctab[targetIndex].stride = 0;
            noReqPro++;
        }        
        else { 
            proctab[targetIndex].stride = 100000/n;
        }
        if (noReqPro != 0) {
            int rest = (total - n) / noReqPro;
            noRequestPro(rest);
        }
        resetPassValue(currentMinPass);
    }
	return (0);
}


