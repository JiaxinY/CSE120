// Some test cases for proportional, let me know if there are any issues.

#include <stdio.h>
#include "aux.h"
#include "umix.h"

// Unused CPU time should be EQUALLY distributed to any remaining
// non-requesting processes that have not requested CPU time.
// output should be something like: 12344444444312444444441234444321132132132123123312132132312312312321312312312312
void TestUnusedCPUtime()
{
    if (Fork () == 0) {

        if (Fork () == 0) {

            RequestCPUrate (0);         // process 4 does not request time, so it has all unused CPU time, which is 70%
            SlowPrintf (6, "44444444444444444444");
            Exit ();
        }

        RequestCPUrate (10);                // process 2
        SlowPrintf (6, "22222222222222222222");
        Exit ();
    }

    if (Fork () == 0) {

        RequestCPUrate (10);                // process 3
        SlowPrintf (6, "33333333333333333333");
        Exit ();
    }

    RequestCPUrate (10);                    // process 1
    SlowPrintf (6, "11111111111111111111");
    Exit ();
}


// output should be something like: 12344242423142424242314242424231424242423142424242313131313131313131313131313131
void TestUnusedCPUtime2()
{
    if (Fork () == 0) {

        if (Fork () == 0) {

            RequestCPUrate (0);         // process 4 does not request time, so it can have 1/2 unused CPU time, which is 40%
            SlowPrintf (6, "44444444444444444444");
            Exit ();
        }

        RequestCPUrate (0);             // process 2 does not request time, so it can also have 1/2 unused CPU time, which is 40%
        SlowPrintf (6, "22222222222222222222");
        Exit ();
    }

    if (Fork () == 0) {

        RequestCPUrate (10);                // process 3
        SlowPrintf (6, "33333333333333333333");
        Exit ();
    }

    RequestCPUrate (10);                    // process 1
    SlowPrintf (6, "11111111111111111111");
    Exit ();
}


// No need to deal with leftover extra as long as a process gets (at least) what is.
// should be getting something like this: 1511151111111111151111111111111111111111151111111111115555555555555555555555555555555555555555555555
void testHaveLeftOverCPUtime() {

    if (Fork () == 0) {

        RequestCPUrate (5);             // process 3
        SlowPrintf (1, "55555555555555555555555555555555555555555555555555");
        Exit ();
    }

    RequestCPUrate (80);                    // process 1
    SlowPrintf (1, "11111111111111111111111111111111111111111111111111");
    Exit ();
}



// Not calling RequestCPUrate (0) makes the current process a nonrequesting process as well
// output should be something like this: 123313131311313131313131313131313311342424424242424242242424242424242424
void testNonRequestingProcess1()
{
    if (Fork () == 0) {

        if (Fork () == 0) {

            // process 4 - Not calling RequestCPUrate (0) makes the current process a nonrequesting process as well
            SlowPrintf (7, "444444444444444444");
            Exit ();
        }
        // process 2 - Not calling RequestCPUrate (0) makes the current process a nonrequesting process as well
        SlowPrintf (7, "222222222222222222");
        Exit ();
    }

    if (Fork () == 0) {

        RequestCPUrate (50);                // process 3
        SlowPrintf (7, "333333333333333333");
        Exit ();
    }

    RequestCPUrate (50);                    // process 1
    SlowPrintf (7, "111111111111111111");
    Exit ();
}

// Process 1 and 3 each get 50% of process so they run first then nonrequesting processes 4 and 2 split the rest afterwards.
// output should be something like this: 123313131311313131313131313131313311342424424242424242242424242424242424
void testNonRequestingProcess2()
{
    if (Fork () == 0) {

        if (Fork () == 0) {

            // process 4
            RequestCPUrate (0); // This makes the current process a nonrequesting process
            SlowPrintf (7, "444444444444444444");
            Exit ();
        }
        // process 2
        RequestCPUrate (0); // This makes the current process a nonrequesting process
        SlowPrintf (7, "222222222222222222");
        Exit ();
    }

    if (Fork () == 0) {

        RequestCPUrate (50);                // process 3
        SlowPrintf (7, "333333333333333333");
        Exit ();
    }

    RequestCPUrate (50);                    // process 1
    SlowPrintf (7, "111111111111111111");
    Exit ();
}

void Main()
{
    //TestUnusedCPUtime();
    //TestUnusedCPUtime2();
    //testHaveLeftOverCPUtime();
    //testNonRequestingProcess1();
    testNonRequestingProcess2();
}

