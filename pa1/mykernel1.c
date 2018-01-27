/* mykernel1.c: your portion of the kernel
 *
 *  	Below are routines that are called by other parts of the kernel. 
 *  	Your ability to modify the kernel is via these routines.  You may
 *  	modify the bodies of these routines, and add code outside or them,
 *  	in any way you wish (however, you cannot change their interfaces).  
 */

#include <strings.h>
#include "aux.h"
#include "sys.h"
#include "mykernel1.h"

CONTEXT cMap[MAXPROCS];
int lastPid;

/*  	NewContext (p, c) will be called by the kernel whenever a new
 *  	process	is created. This is essentially a notification (which you
 *  	will make use of) that this newly created process has an ID of p,
 *  	and that its initial context is pointed to by c.  Make sure you
 *  	make a copy of the contents of this context (i.e., don't copy the
 *  	pointer, but the contents pointed to by the pinter), as the pointer
 *  	will become useless after this routine returns.
 */

void NewContext (p, c)
	int p;				// ID of new process just created
	CONTEXT *c;			// initial context for this process
{
	/* your code here 
	Printf("NewContext - p: %d\n", p);
	if (p == -1) 
		return;
	memcpy(&cMap[p-1],c,sizeof(CONTEXT));
	*/
}

/*  	MySwitchContext (p) should cause a context switch from the calling
 *  	process to process p. It should return the ID of the calling
 *  	process. The ID of the calling process can be determined by calling
 *  	GetCurProc (), which returns the currently running process's ID.  
 *  	The routine SwitchContext (p) is an internal working version of
 *  	context switching.  This is provided so that the kernel works
 *  	without modification, to allow the other exercises to execute and
 *  	to illustrate proper behavior. For Exercise F, the call to
 *  	SwitchContext (p) MUST be removed.  
 */
 
int MySwitchContext (p)
	int p;				// process to switch to
{
	/* your code here 
	Printf("MySwitchContext - p: %d\n", p);

	if (p == -1)
		return lastPid;
	int isRestore = 0;

	CONTEXT currentContext;
	int currentP = GetCurProc();
	lastPid = currentP;
	SaveContext(&currentContext);
	cMap[currentP-1] = currentContext;

	if (isRestore == 1)	return lastPid;
	else isRestore = 1;

	CONTEXT c = cMap[p-1];
	RestoreContext(&c);
	return(currentP);
*/
	return (SwitchContext(p));
}
