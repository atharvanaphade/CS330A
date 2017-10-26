// stats.h
//	Routines for managing statistics about Nachos performance.
//
// DO NOT CHANGE -- these stats are maintained by the machine emulation.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "stats.h"
#include "system.h"

extern unsigned int thread_index;
//----------------------------------------------------------------------
// Statistics::Statistics
// 	Initialize performance metrics to zero, at system startup.
//----------------------------------------------------------------------

Statistics::Statistics()
{
    totalTicks = idleTicks = systemTicks = userTicks = 0;
    numDiskReads = numDiskWrites = 0;
    numConsoleCharsRead = numConsoleCharsWritten = 0;
    numPageFaults = numPacketsSent = numPacketsRecvd = 0;
    numCPUBursts=totalCPUBurstTime=avgCPUBurstLength=0;
    cpuUtilization=0;
    totalWaitingTime = avgWaitingTime = numberOfThreads = 0;
    totCompletionTime = avgCompletionTime = totCompletionTime2 = maxCompletionTime = 0;
    minCompletionTime = 100000;
    minBurstTime = 100000;
    maxBurstTime = 0;
    burstEstimationErr = 0;
}

//----------------------------------------------------------------------
// Statistics::Print
// 	Print performance metrics, when we've finished everything
//	at system shutdown.
//----------------------------------------------------------------------

void
Statistics::Print()
{
    printf("Ticks: total %d, idle %d, system %d, user %d\n", totalTicks,
	idleTicks, systemTicks, userTicks);
    printf("Disk I/O: reads %d, writes %d\n", numDiskReads, numDiskWrites);
    printf("Console I/O: reads %d, writes %d\n", numConsoleCharsRead,
	numConsoleCharsWritten);
    printf("Paging: faults %d\n", numPageFaults);
    printf("Network I/O: packets received %d, sent %d\n", numPacketsRecvd,
	numPacketsSent);

    // Our Statistics:

    // Computing Average CPU burst length
    avgCPUBurstLength=(totalCPUBurstTime)/numCPUBursts;
    // Computing CPU utilization
    cpuUtilization=(totalCPUBurstTime*100)/totalTicks;
    avgCompletionTime = totCompletionTime/(thread_index-1);
    printf("Total CPU busy time: %d\n",totalCPUBurstTime);
    printf("Total Execution Time: %d\n",totalTicks);
    printf("CPU Utilization: %d\n",cpuUtilization);
    printf("Minimum CPU burst length: %d\n",minBurstTime);
    printf("Maximum CPU burst length: %d\n",maxBurstTime);
    printf("Average CPU burst length: %d\n",avgCPUBurstLength);
    printf("Number of non-zero CPU bursts observed: %d\n",numCPUBursts);
    printf("Average Wait Time: %d\n",totalWaitingTime/thread_index);
    printf("Maximum thread completion time: %d\n",maxCompletionTime);
    printf("Minimum thread completion time: %d\n",minCompletionTime);
    printf("Average thread completion time: %d\n",avgCompletionTime);
    printf("Variance of thread completion time: %d\n",(totCompletionTime2/(thread_index-1)-avgCompletionTime*avgCompletionTime));
    printf("Burst estimation error ratio: %f\n",(float)burstEstimationErr/totalCPUBurstTime);
}
