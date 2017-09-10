Syscall Implementations:

SysCall_GetReg:
Returns the contents of the processor register, number of which is passed as argument. Argument is read using machine::ReadRegister on register $4 and returned by writing value to register $2 using machine::WriteRegister.

SysCall_GetPA:
Returns the physical address corresponding to the virtual address passed as argument. Argument is read from register $4 and translated to physical memory based on machine/translate.cc. Various failing conditions are checked in which case -1 is returned by writing -1 to $2, else the physical memory is returned.

SysCall_GetPID:
Returns the id of the calling thread. An extern counter is maintained in threads.h which is incremented each time a new thread's constructor is called. Public functions for getting and setting the PID value for each thread were created and used to return the PID.

SysCall_GetPPID:
Returns the id of the parent of the calling thread. Similar to above, the value of parent's PID is stored in thread's variable when its constructor is called as the PID of the currentThread at the time of constructor call occuring. Special case of 1st thread is handled seperately by assigning parent's PID as 0.

SysCall_Time:
Returns the total ticks at present (roughly represents the current simulated time). Value stored in variable totalTicks in the stats class is written to register $2 for this purpose.

SysCall_Sleep:
Puts the calling thread to sleep for the number of ticks passed as argument. An extern queue Wailist is created using list.h for storing current sleeping threads and waketimes in a sorted manner based on waketimes. The current thread is put to sleep using NachOSThread::PutThreadToSleep() after disabling interrupts which are enabled after this function. Special case of argument 0 is handled seperately by calling NachOSThread::YieldCPU() directly.

SysCall_Yield:
Gives CPU to scheduler so that other threads can be scheduled. A call to NachOSThread::YieldCPU() is made for this purpose after which program counters are advanced.

SysCall_Fork:
A new thread (childThread) is created and a pointer to parent thread is added in thread class. An array in parent for maintaining PID values of child threads has been created which is updated here. ProcessAddressSpace for childthread is assigned using parent's ProcessAddressSpace, and contents are copied using NachOSThread::ThreadFork(). The user state for child is saved using NachOSThread::SaveUserState, and child is given a 0 return value by assigning 0 to $2 register. child's PID value is returned by writing to register $2.

SysCall_Join:
Waits for the child to complete whose PID value is passed as argument. First a check is done on PID value to confirm that PID belongs to a child of the thread by searching through childPID array, failing which returns a -1. Next, it is checked if the child has exited by checking the value corresponding to child stored in an array of exitcodes which are written when a child exits and this value, if exists, is returned by writing to register $2. Otherwise, the parent is put to sleep after storing the PID value of child it is supposed to wait for in a variable joinpid.

SysCall_NumInstr:
Returns the number of instructions executed so far by the calling process. For this, a variable is created in Thread class which is incremented for the current thread every time we read an instruction in oneInstruction function in mipsim.c.

SysCall_Exec:
Stores the file_path in a fixed size string buffer. Opens the executable file and makes a address space for process then initializes registers and runs the program.

SysCall_Exit:
if there is only one thread running then it halts the machine else it sets the exit status in parent thread's child array and moves current thread to FinishThread(). If parent thread is already waiting for this process then it moves parent to ready queue also.
