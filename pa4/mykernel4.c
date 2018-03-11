/*   	Umix thread package
 *
 */

#include <setjmp.h>
#include <string.h>

#include "aux.h"
#include "umix.h"
#include "mykernel4.h"

static int MyInitThreadsCalled = 0;	// 1 if MyInitThreads called, else 0
static int curr;
static int next;
static int active;
static int head;
static int tail;

static struct thread {			// thread table
	int valid;			// 1 if entry is valid, else 0
	jmp_buf env;			// current context
    jmp_buf base_env;
    int next;
    void (*func)();
    int param;

} thread[MAXTHREADS];

#define STACKSIZE	65536		// maximum size of thread stack


void printMyQueue(){
    int j = head;
    while (thread[j].next != -1){
        DPrintf("Thread: %d, next: %d\n", j, thread[j].next);
        j = thread[j].next;
    }
}

void EnqueueThread(int t) {
    thread[tail].next = t;
    tail = t;
    //Printf("####Add: %d\n", t);
    //printMyQueue();
}

int DequeueThread(int t) {
    int p = -1;
    int c = head;
    int found = -1;
    
    //Printf("****Remove: %d\n ",t);

    if (c == t) {
        head = thread[head].next;
        thread[t].next = -1;
        return t;
    }

    while(c!=tail) {
        if (c == t) {
            found = 1;
            break;
        }
        p = c;
        c = thread[c].next;
    }
    if(found == -1 && tail == t) {
        thread[tail].next = -1;
        tail = p;
        return t;
    }

    if(found) {
        thread[p].next = thread[c].next;
        thread[c].next = -1;
        return c;
    }   // printMyQueue();
    return -1; 
}

int PopThread() {
    return DequeueThread(head);
}

/* 	MyInitThreads () initializes the thread package. Must be the first
 *  	function called by any user program that uses the thread package.  
 */

void MyInitThreads ()
{
	int i;

	if (MyInitThreadsCalled) {		// run only once
		Printf ("MyInitThreads: should be called only once\n");
		Exit ();
	}

	for (i = 0; i < MAXTHREADS; i++) {	// initialize thread table
		thread[i].valid = 0;
        thread[i].next = -1;
        thread[i].func = NULL;
        thread[i].param = -1;
	}

	thread[0].valid = 1;			// initialize thread 0
    curr = 0;
    active = 1;
    next = 1;
    head = 0;
    tail = 0;

    if (setjmp(thread[0].env) == 0) {
        for (i = 0; i < MAXTHREADS; i++) {
            char stack[i*STACKSIZE];
            if (((int) &stack[STACKSIZE-1]) - ((int) &stack[0]) + 1 != STACKSIZE) {
                Printf ("Stack space reservation failed\n");
                Exit ();
            }

            if (setjmp(thread[i].env) != 0) {
                void (*f)() = thread[MyGetThread()].func;
                int p = thread[MyGetThread()].param;
                (*f)(p);
                MyExitThread();         
            }
            memcpy(&thread[i].base_env, &thread[i].env, sizeof(thread[i].env));
        }
    }
    else {
        (*thread[0].func)(thread[0].param);
        MyExitThread();
    }

	MyInitThreadsCalled = 1;
}

/*  	MyCreateThread (f, p) creates a new thread to execute
 * 	f (p), where f is a function with no return value and
 *  	p is an integer parameter.  The new thread does not begin
 * 	executing until another thread yields to it. 
 */

int MyCreateThread (f, p)
	void (*f)();			// function to be executed
	int p;				// integer parameter
{
	if (! MyInitThreadsCalled) {
		Printf ("MyCreateThread: Must call MyInitThreads first\n");
		Exit ();
	}

    if (active >= MAXTHREADS) {
        return -1;
    }
    
    int thread_id = -1;
    for (int i = next; i < MAXTHREADS; i++) {	
        if (thread[i].valid == 0) {
            thread_id = i;
            thread[i].valid = 1;
            thread[i].func = f;
            thread[i].param = p;
            break;
        }
    }

    if (thread_id == -1) {
        for(int i = 0; i < next; i++) {
            if (thread[i].valid == 0) {
                thread_id = i;
                thread[i].valid = 1;
                thread[i].func = f;
                thread[i].param = p;
                break;
            }
        }
    }
    //Printf("Assign Thread: %d\n", thread_id);
    next = thread_id + 1;
    if (next == MAXTHREADS)
        next = 0;
    active++;
    //Printf("Active Thread: %d\n", active);
    //if (setjmp (thread[thread_id].env) == 0) {	// save context of created thread
    //    longjmp (thread[curr].env, 1);	// back to thread 0
    //}

    /* here when thread 1 is scheduled for the first time */

    //(thread[thread_id].func)(thread[thread_id].param);		// execute func (param)

    //MyExitThread ();		// thread 1 is done - exit

	return (thread_id);		// done, return new thread ID
}

/*   	MyYieldThread (t) causes the running thread, call it T, to yield to
 * 	thread t.  Returns the ID of the thread that yielded to the calling
 *  	thread T, or -1 if t is an invalid ID. Example: given two threads
 *  	with IDs 1 and 2, if thread 1 calls MyYieldThread (2), then thread 2
 * 	will resume, and if thread 2 then calls MyYieldThread (1), thread 1
 *  	will resume by returning from its call to MyYieldThread (2), which
 * 	will return the value 2.
 */

int MyYieldThread (t)
	int t;				// thread being yielded to
{
	if (! MyInitThreadsCalled) {
		Printf ("MyYieldThread: Must call MyInitThreads first\n");
		Exit ();
	}

	if (t < 0 || t >= MAXTHREADS) {
		Printf ("MyYieldThread: %d is not a valid thread ID\n", t);
		return (-1);
	}
	if (! thread[t].valid) {
		Printf ("MyYieldThread: Thread %d does not exist\n", t);
		return (-1);
	}
    
    int currentThread = MyGetThread();
    //Printf("Current: %d t: %d\n", currentThread,t);
    if (currentThread == t)
        return t;
    if (t != head)
    {
        EnqueueThread(currentThread);
        DequeueThread(t);
    
    }
    //Printf("currentThread: %d  t: %d\n",currentThread, t);
        if (setjmp (thread[currentThread].env) == 0) {
                curr = t;
                longjmp (thread[t].env, 1);
        }
    return currentThread;
}

/*   	MyGetThread () returns ID of currently running thread. 
 */

int MyGetThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("MyGetThread: Must call MyInitThreads first\n");
		Exit ();
	}
    return curr;
}

/* 	MySchedThread () causes the running thread to simply give up the
 *  	CPU and allow another thread to be scheduled.  Selecting which
 *  	thread to run is determined here.  Note that the same thread may
 * 	be chosen (as will be the case if there are no other threads). 
 */

void MySchedThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("MySchedThread: Must call MyInitThreads first\n");
		Exit ();
	}

    if (thread[curr].valid)
        EnqueueThread(curr);
    
    int t = PopThread();

    if (t != -1){
        if (setjmp (thread[curr].env) == 0) {
            curr = t;
            longjmp (thread[curr].env, 1);
        }
    }


}

/*  	MyExitThread () causes the currently running thread to exit.  
 */

void MyExitThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("MyExitThread: Must call MyInitThreads first\n");
		Exit ();
	}
    thread[curr].valid = 0;
    active--;
    memcpy(thread[curr].env, thread[curr].base_env, sizeof(jmp_buf));
    if (active == 0){
        Exit();
    }
    MySchedThread();
}
