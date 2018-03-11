/*   	Umix thread package
 *
 */

#include <setjmp.h>
#include <string.h>

#include "aux.h"
#include "umix.h"
#include "mykernel4.h"

static int MyInitThreadsCalled = 0;	// 1 if MyInitThreads called, else 0
static int curr = 0;
static int pre = -1;
static int next;
static int active = 0;
static int threadQueue[MAXTHREADS];

static struct thread {			// thread table
	int valid;			// 1 if entry is valid, else 0
	jmp_buf env;			// current context
    jmp_buf base_env;
    void (*func)();
    int param;

} thread[MAXTHREADS];

#define STACKSIZE	2*65536		// maximum size of thread stack

void PrintQueue(){
    Printf("Queue: #####\n");
    for (int i = 0; i < MAXTHREADS; i++)
        Printf("%d:  %d\n",i,threadQueue[i]);
}

int FoundThread(int t) {
    for( int i = 0; i < active; i++){
        if(threadQueue[i] == t)
            return i;
    }
    return -1;
}

void RemoveMyThread(int t) {
    if (active == 1) 
        threadQueue[0] = -1;

    else {
        for (int i = FoundThread(t); i < active - 1; i++) 
            threadQueue[i] = threadQueue[i+1];
        threadQueue[active - 1] = -1;
    }
    active--;
}

void MoveFrontThread(int t) {
    if (FoundThread(t) != -1)
        RemoveMyThread(t);

    for(int i = active; i > 0; i--)
        threadQueue[i] = threadQueue[i-1];
    threadQueue[0] = t;
    active++;
}

void MoveBackThread(int t) {
    if (FoundThread(t) != -1)
        RemoveMyThread(t);
    threadQueue[active++] = t;
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
        thread[i].func = NULL;
        thread[i].param = -1;
        threadQueue[i] = -1;
	}

	thread[0].valid = 1;			// initialize thread 0
    pre = -1;
    next = 1;
    MoveFrontThread(0);


        for (i = 0; i < MAXTHREADS; i++) {
            char stack[i*STACKSIZE];
            if (((int) &stack[STACKSIZE-1]) - ((int) &stack[0]) + 1 != STACKSIZE) {
                Printf ("Stack space reservation failed\n");
                Exit ();
            }

            if (setjmp(thread[i].base_env) != 0) {
                void (*f)() = thread[MyGetThread()].func;
                int p = thread[MyGetThread()].param;
                (*f)(p);
                MyExitThread();         
            }
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
            memcpy(thread[i].env, thread[i].base_env, sizeof(jmp_buf));
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
                memcpy(thread[i].env, thread[i].base_env, sizeof(jmp_buf));
                thread_id = i;
                thread[i].valid = 1;
                thread[i].func = f;
                thread[i].param = p;
                break;
            }
        }
    }
    next = thread_id + 1;
    if (next == MAXTHREADS)
        next = 0;

    MoveBackThread(thread_id);

	return thread_id;		// done, return new thread ID
}

/*   	MyYieldThread (t) causes the running thread, call it T, to yield to
 * 	thread t.  Returns the ID of the thread that yielded to the calling
 *  	thread T, or -1 if t is an invalid ID. Example: given two threads
 *  	with IDs 1 and 2, if thread 1 calls MyYieldThread (2), then thread 2
 * 	will resume, and if thread 2 then calls MyYieldThread (1), thread 1
 *  	will resume by returning from its call to MyYieldThread (2), which
 * 	will return the value 2.
 */

void PrintMyThread() {
    Printf("MyThread: #######\n");
    for (int i = 0; i < MAXTHREADS; i++) {
        Printf("Thread %d: %d\n", i, thread[i].valid);
    }

}
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
    
    int currentThread = threadQueue[0];
    MoveBackThread(currentThread);

    MoveFrontThread(t);
    pre = MyGetThread(); 
    if (pre == t) {
        return t;
    }
    if (setjmp (thread[pre].env) == 0) {
                curr = t;
                longjmp (thread[curr].env, 1);
        }
    return pre;
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
    int nextThread;
	if (! MyInitThreadsCalled) {
		Printf ("MySchedThread: Must call MyInitThreads first\n");
		Exit ();
	}

    if (active == 0)
        return;

    nextThread = threadQueue[1];

    if (active == 1)
        nextThread = threadQueue[0];

    MyYieldThread(nextThread);
        

}

/*  	MyExitThread () causes the currently running thread to exit.  
 */

void MyExitThread ()
{
	if (! MyInitThreadsCalled) {
		Printf ("MyExitThread: Must call MyInitThreads first\n");
		Exit ();
	}
    int currentThread = MyGetThread();

    thread[currentThread].valid = 0;
    RemoveMyThread(currentThread);
    //active--;
    //memcpy(thread[curr].env, thread[curr].base_env, sizeof(jmp_buf));
    if (active == 0){
        Exit();
    }
    MyYieldThread(threadQueue[0]);
}
