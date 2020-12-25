#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>
#include <syscall.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <err.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <byteswap.h>
#include "tracer.h"

int8_t tracee_goto_syscall(pid_t child)
{
    int status;
    for (;;)
    {
        errno = 0; //reset errno before ptrace
        if (ptrace(PTRACE_SYSCALL, child, 0, 0) != 0) //ptrace failed. Usually signifies that the child process exited unexpectedly with a non-zero value
        {
            return -1; //-1 == ptrace failed, probably child exited unsuccessfully. Check errno for more details
        }

        waitpid(child, &status, 0); 
        if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80) //0x80 & SIGSTOP == syscall, refer to manpage
        {
            return 0; //0 == stopped at syscall-enter or exit.
        }
        else if (WIFEXITED(status)) //child exited normally. Stop.
        {
            return 1; // 1 == program exited successfully
        }
    }
    return 0;
}

int16_t tracee_get_syscall_info(pid_t child, syscall_info_t info)
{
    typedef uint16_t WORD; //refers to asm WORD(16 bits). Not strictly necessary, but contextually more appropriate than using sizeof(uint16_t)
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, child, 0, &regs) != 0) 
    {
        return -1;
    }
    /*Read the instruction used to trigger a syscall from tracee's memory
    * Keep in mind that RIP holds the pointer to the NEXT instruction, therefore sizeof(WORD) is subtracted
    * Both `int 0x80` and `syscall` are 2 byte instructions
    * Note: on Von Neumann architectures, PEEKTEXT == PEEKDATA
    */
    int64_t syscall_instr; // must be signed, since ptrace may fail with -1.
    if ((syscall_instr = ptrace(PTRACE_PEEKTEXT, child, regs.rip - sizeof(WORD), 0)) == -1) //can't access memory
    {
        return -1;
    }
    info->number = (uint16_t) regs.orig_rax; //syscall number is stored in RAX
    info->instr = (uint64_t) syscall_instr; //save the syscall instruction
    info->ip = regs.rip - sizeof(WORD); //save the pointer to the instruction
    info->regs = regs; //save CPU state. Note: compiler shall copy the regs structure, no need for memcpy

    return 0;
}

int8_t tracee_convert_syscall(pid_t child, syscall_info_t info)
{   
    switch (SYS_GET_OPCODE(info->instr))
    {
        case SYSCALL_INSTR: //for x64 ABI conforming syscall, do nothing
            break;
        case INT80_INSTR: //for non-conforming (x32 ABI) syscall, change the rax value
            
            if (SYSCALL_MAP[info->number] == -1) //-1 == no appropriate x64 counterpart, can't do anything
                break;
            
            //intentionally set an invalid syscall number, this should fail with ENOSYS
            ptrace(PTRACE_POKEUSER, child, sizeof(long) * ORIG_RAX, -1); 
            return 1; //1 == tracee should restart

            

            // ptrace(PTRACE_POKEUSER, child, sizeof(long) * ORIG_RAX, ~__X32_SYSCALL_BIT & (info->number & 0xffffffff));
            // uint64_t num = ptrace(PTRACE_PEEKUSER, child, sizeof(long) * RIP);
            // printf("new number: %d\n", num);
            break;
        default:
            return -1;
    }
    return 0;
}

ssize_t tracee_syscall_restart_prepare(pid_t child, syscall_info_t info)
{
    //clear errno, the previous syscall has been cancelled
    ptrace(PTRACE_POKEUSER, child, sizeof(long) * RAX, 0);
    //change the instruction. 
    ptrace(PTRACE_POKETEXT, child, info->ip, SYS_SET_OPCODE(info->instr, SYSCALL_INSTR));
    //rewind the address back to the syscall instruction
    ptrace(PTRACE_POKEUSER, child, sizeof(long) * RIP, info->ip);
    return 0;
}

ssize_t tracee_syscall_restart(pid_t child, syscall_info_t info)
{
    //Look up the corresponding syscall number in the mapping table
    /*Quote from manpage for syscall:
    * ...In order to indicate that a system call is called under the x32 ABI, an additional
    * bit, __X32_SYSCALL_BIT, is bitwise ORed with the system call number.
    */ 
    //X32 ABI compatibility mode helps deal with register extending and other subtle differences between x32 and x64 that can't be addressed in user-space
    info->regs.orig_rax = __X32_SYSCALL_BIT + SYSCALL_MAP[info->number];
    info->regs.r8 = info->regs.rdi;
    info->regs.r9 = info->regs.rbp;
    info->regs.rsi ^= info->regs.rcx;
    info->regs.rcx ^= info->regs.rsi;
    info->regs.rsi ^= info->regs.rcx;
    info->regs.rdi ^= info->regs.rbx;
    info->regs.rbx ^= info->regs.rdi;
    info->regs.rdi ^= info->regs.rbx;
    //swap registers. Refer to x32 and x64 ABI for more information

    ptrace(PTRACE_SETREGS, child, 0, &(info->regs));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "sysadapt failed: not enough arguments\n");
        exit(EXIT_FAILURE);
    }
    pid_t child;
    int8_t retval = 0;
    int status = 0;
    int8_t restart = 0;
    if ((child = fork()) == 0) //in child
    {
        ptrace(PTRACE_TRACEME, 0, 0, 0); //become a tracee
        #ifdef TRACEEXEC //if you prefer to start tracing before the exec starts. Not recommended
            kill(getpid(), SIGSTOP); //commit suicide, resume parent. Will continue later
        #endif
        execvp(argv[1], argv + 1); //execute image of a child process. Note that the child will stop before actually running the program
        // TODO: error handling
        error(EXIT_FAILURE, errno, "in child process"); //print error + errno message and exit with code 1
    }
    if (child == -1) //in error
    {
        //TODO: handle this -- done
        error(EXIT_FAILURE, errno, "fork failed");
    }
    //in parent
    waitpid(child, &status, 0);
    if(WIFSIGNALED(status) || WIFEXITED(status))
    {
        error(EXIT_FAILURE, errno, "exec failed");
    }
    //TODO: terminate parent if execvp in child was unsuccessful -- done

    ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL); //distinguish between syscalls and other signals and exit if child exits.
    // Note: in order to distinguish the end of EXEC, PTRACE_O_TRACEEXEC could be added, although it is unnecessary here.  
    
    static struct syscall_info info; // a structure that contains syscall number and instruction. Will be set in call to tracee_get_syscall_info. Allocated statically
    for(;;)
    {
        //let tracee go to and stop before the syscall-start
        retval = tracee_goto_syscall(child);
        if (retval != 0) //program has exited (1 or -1). Check the reason after the loop.
            break;
        if (restart) //reset syscall number and clear restart flag
        {
            restart = 0;
            tracee_syscall_restart(child, &info);
            #ifdef VERBOSE
            printf("syscall restarted successfully\n");
            printf("regs: rdi: %x, rsi: %x, rdx: %x\n", info.regs.rdi, info.regs.rsi, info.regs.rdx);
            #endif
        }

        //determine and set the parameters in syscall_info_t structure
        retval = tracee_get_syscall_info(child, &info);
        if (retval != 0) //definitely failed.
            break;
        #ifdef VERBOSE
        printf("instruction: %s, number: %d\n", SYS_GET_OPCODE(info.instr) == INT80_INSTR ? "int 0x80" : "syscall", info.number);
        #endif
        //based on information gathered, either leave everything unchanged or, in case of x32 abi, mark syscall as invalid
        restart = tracee_convert_syscall(child, &info);
        if (restart < 0)
            break;

        //let tracee execute the syscall and stop before syscall-exit
        retval = tracee_goto_syscall(child);
        if (retval != 0)
            break;
        if (restart) //cancel the dummy syscall and prepare to restart the previous syscall
        {
            #ifdef VERBOSE
            printf("legacy syscall detected, converting and restarting with x64 ABI...\n");
            #endif
            tracee_syscall_restart_prepare(child, &info);
        }
        //rince and repeat...
    }

    if (retval == -1) //ptrace failed somehow. Print errno and exit with code 1
    {
        error(-EXIT_FAILURE, errno, "program exited unexpectedly");
    }
    return 0; //exit successfully
    
}