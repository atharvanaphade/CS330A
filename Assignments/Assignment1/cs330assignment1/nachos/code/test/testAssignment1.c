#include "syscall.h"

int main(){
    syscall_wrapper_PrintString("Test Code for Assignment 1\n");
    syscall_wrapper_PrintString("X Parent PID: ");
    syscall_wrapper_PrintInt(syscall_wrapper_GetPID());
    syscall_wrapper_PrintChar('\n');
    syscall_wrapper_PrintString("X parent Sleep at time: ");
    syscall_wrapper_PrintInt(syscall_wrapper_GetTime());
    syscall_wrapper_PrintChar('\n');
    syscall_wrapper_Sleep(1); // increasing this sleep time is giving memory error
    syscall_wrapper_PrintString("X parent returned from sleep at time: ");
    syscall_wrapper_PrintInt(syscall_wrapper_GetTime());
    syscall_wrapper_PrintChar('\n');

    int x = syscall_wrapper_Fork();
    if(x == 0){
        syscall_wrapper_Sleep(10000);
        syscall_wrapper_PrintString("X Child PID: ");
        syscall_wrapper_PrintInt(syscall_wrapper_GetPID());
        syscall_wrapper_PrintChar('\n');
        syscall_wrapper_PrintString("X Child's parent PID: ");
        syscall_wrapper_PrintInt(syscall_wrapper_GetPPID());
        syscall_wrapper_PrintChar('\n');
        syscall_wrapper_PrintChar('\n');

        int y = syscall_wrapper_Fork();
            if(y == 0){
                syscall_wrapper_Sleep(10000);
                syscall_wrapper_PrintString("Y Child PID: ");
                syscall_wrapper_PrintInt(syscall_wrapper_GetPID());
                syscall_wrapper_PrintChar('\n');
                syscall_wrapper_PrintString("Y Child's parent PID: ");
                syscall_wrapper_PrintInt(syscall_wrapper_GetPPID());
                syscall_wrapper_PrintChar('\n');
                syscall_wrapper_PrintChar('\n');
                syscall_wrapper_Exec("../test/vectorsum");
            }
            else{
                syscall_wrapper_PrintString("Y Parent is X child.\n");
                syscall_wrapper_PrintString("Y Parent after fork waiting for child: ");
                syscall_wrapper_PrintInt(y);
                syscall_wrapper_PrintChar('\n');
                syscall_wrapper_Join(y);
                syscall_wrapper_PrintChar('\n');
                syscall_wrapper_PrintString("Y Parent executed ");
                syscall_wrapper_PrintInt(syscall_wrapper_GetNumInstr());
                syscall_wrapper_PrintString(" instructions.\n");
                syscall_wrapper_PrintChar('\n');
            }
        syscall_wrapper_Exec("../test/forkjoin");
    }
    else{
        syscall_wrapper_PrintString("X Parent after fork waiting for child: ");
        syscall_wrapper_PrintInt(x);
        syscall_wrapper_PrintChar('\n');
        syscall_wrapper_Join(x);
        syscall_wrapper_PrintChar('\n');
        syscall_wrapper_PrintString("X Parent executed ");
        syscall_wrapper_PrintInt(syscall_wrapper_GetNumInstr());
        syscall_wrapper_PrintString(" instructions.\n");
    }

    syscall_wrapper_PrintString("\nPhysical address of X ");
    syscall_wrapper_PrintInt(syscall_wrapper_GetPA(&x));
    syscall_wrapper_PrintChar('\n');
    syscall_wrapper_PrintString("Current physical address of stack top: ");
    syscall_wrapper_PrintInt(syscall_wrapper_GetPA(syscall_wrapper_GetReg(29)));
    syscall_wrapper_PrintChar('\n');
    syscall_wrapper_PrintString("We are currently at PC: ");
    syscall_wrapper_PrintIntHex(syscall_wrapper_GetReg(34));
    syscall_wrapper_PrintChar('\n');
    return 0;
}