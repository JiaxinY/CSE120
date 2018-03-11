#include <stdio.h>
#include "aux.h"
#include "umix.h"

void InitRoad (void);
void driveRoad (int from, int mph);

struct {
    int direction;
    int westLine;
    int eastLine;
    int westWaitCount;
    int eastWaitCount;
    int eastGate;
    int westGate;
    int Gate;
    int turns;
    int carsOnRoad;
    int road[NUMPOS];
} shm;

void Main ()
{
  InitRoad ();

  if (Fork () == 0) {
    Delay (0);
    driveRoad (WEST, 50);
    Exit ();
  }

  if (Fork () == 0) {
    Delay (50);
    driveRoad (EAST, 20);
    Exit ();
  }

  if (Fork () == 0) {
    Delay (100);
    driveRoad (WEST, 20);
    Exit ();
  }

  driveRoad (WEST, 5);

  Exit ();
}

void InitRoad ()
{
	/* do any initializations here */
    Regshm ((char *) &shm, sizeof (shm));
    shm.westLine = Seminit(1);
    shm.eastLine = Seminit(1);
    shm.turns = Seminit(0);
    shm.westGate = Seminit(1);
    shm.eastGate = Seminit(1);
    for(int i = 0; i < NUMPOS; i++) {
        shm.road[i] = Seminit(1);
        //Printf("shm.road[%d]: %d\n", i, shm.road[i]);
    }

 /*   for(int i = 0; i < NUMPOS; i++)
        Printf("shm.road[%d]: %d\n", i, shm.road[i]);
    Printf("shm.westLine: %d\n", shm.westLine);
    Printf("shm.eastLine: %d\n", shm.eastLine);
    Printf("shm.Gate: %d\n", shm.Gate);
*/
    shm.westWaitCount = 0;
    shm.eastWaitCount = 0;
    shm.carsOnRoad = 0;
    //shm.turns = WEST;
    shm.direction = -1;
}

#define IPOS(FROM)	(((FROM) == WEST) ? 1 : NUMPOS)

void driveRoad (from, mph)
	int from;			// coming from which direction
	int mph;			// speed of car
{
	int c;				// car ID c = process ID
	int p, np, i;			// positions

	c = Getpid ();			// learn this car's ID
    
    if (from == WEST) {
        // Printf("WEST:\n");
        shm.westWaitCount++;
        Wait(shm.westLine);
    }
    
    else {
        //  Printf("EAST:\n");
        shm.eastWaitCount++;
        Wait(shm.eastLine);
    }

    if ((((shm.direction == WEST && shm.eastWaitCount!=0) || (shm.direction == EAST && shm.westWaitCount!=0)) && shm.carsOnRoad!=0)) {
        Printf("Take TURNS %d!! %d\n",c,shm.carsOnRoad);
        Wait(from == WEST? shm.westGate : shm.eastGate);
        Wait(shm.turns);
        
    }

    if (from == WEST) {
        Wait(shm.westGate);
    }
    else {
       // shm.eastWaitCount++;
        Wait(shm.eastGate);

    }

    //shm.direction = from;
    Wait(shm.road[IPOS(from)-1]);

    shm.direction = from;
    EnterRoad (from);
    PrintRoad ();
    Printf ("Car %d enters at %d at %d mph\n", c, IPOS(from), mph);
   
    if(!shm.carsOnRoad){
        if (from == WEST) {
            Wait(shm.eastGate);
            Wait(shm.eastLine);
        }
        else {
            Wait(shm.westLine);
            Wait(shm.westGate);
        }
    }

    shm.carsOnRoad++;
  
    if (from == WEST) {
        Signal(shm.westGate);
        Signal(shm.westLine);
    }
    else {
        Signal(shm.eastGate);
        Signal(shm.eastLine);
    }
    
  //  if (shm.westWaitCount!=0 && shm.eastWaitCount!=0) {
    //    Signal(shm.turns); 
    //}
 

    for (i = 1; i < NUMPOS; i++) {
		if (from == WEST) {
			p = i;
			np = i + 1;
           // Signal(shm.westGate);
		} else {
			p = NUMPOS + 1 - i;
			np = p - 1;
            // Signal(shm.eastGate);

		}

		Delay (3600/mph);
        Wait(shm.road[np-1]);
		ProceedRoad ();
        PrintRoad ();
		Printf ("Car %d moves from %d to %d\n", c, p, np);    
        Signal(shm.road[p-1]);

        if (p-1 == 0 || p-1 == 9) {
            if (from == WEST) {// && shm.westWaitCount!=0) {
            shm.westWaitCount--;
            Signal(shm.westLine);
            }
        
            else {//if(from == EAST && shm.eastWaitCount!=0) {
                shm.eastWaitCount--;
                Signal(shm.eastLine);
            }
        }

	}

	Delay (3600/mph);
	ProceedRoad ();
	PrintRoad ();
	Printf ("Car %d exits road\n", c);
    shm.carsOnRoad--;

    if(from == WEST)
        Signal(shm.road[NUMPOS-1]);
    else
        Signal(shm.road[0]);

   // Printf("shm.carsOnRoad: %d\n", shm.carsOnRoad);
    if (shm.carsOnRoad == 0) {
        Signal(shm.turns);
        if (from == WEST) {
            Signal(shm.eastLine);
            Signal(shm.eastGate);
        }
        else {
            Signal(shm.westGate);
            Signal(shm.westGate);
        }
    }
    /*if(shm.carsOnRoad == 0) {
        shm.turns = (from == WEST? EAST : WEST);
        //Signal(shm.Gate);
        if(shm.turns == WEST && shm.westWaitCount!=0) {
            Printf (" Open West Gate\n");
            //shm.westGate = 1;
            shm.westWaitCount--;
            Signal(shm.westLine);
            Signal(shm.westGate);
        }
        else if (shm.eastWaitCount!=0) {
            Printf (" Open East Gate\n");
          //  shm.eastGate = 1;
            shm.eastWaitCount--;
            Signal(shm.eastLine);
            Signal(shm.eastGate);
        }
    }
    */
}


