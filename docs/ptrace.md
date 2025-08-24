# Ptrace

## 1. strace's perspective

What is strace?

- Diagnostic, debugging and instructional utility.
- Monitors and tampers with syscalls and signals.
- Unprivileged, user space.
- uses ptrace API.
- the only general purpose tracer of its kind.

The best syscall in decoder in town.

- Can access tracee memory, filesystem, etc.
- Can perform complex manipulates with the data.
- Can obtain a lot of associated information.
- Can tamper with syscall numbers and their return values.
- Can inject data both on entering and on exiting syscalls.

## 2. Architecture of a ptrace based tracer: overview

What is the ptrace API for?
A means by which one process may:

- Observe and control the execution of another process.
- Examine and change the tracee's registers and memory.

Tracer architecture summary:

- A tracee needs to be attached to the tracer.
- The Tracer enters event loop.
- When the tracer is finished tracing, it can cause the tracee to continue executing in a regular, non-traced mode.

## 3. Architecture of a ptrace based tracer: event loop

Tracer event loop:

- The tracee stops each time a traceable event happens.
- The tracer is notified using a `wait` family syscall. E.g. `waitpid(-1, &status, __WALL)`.
- That call returns a status value containing information that indicates the cause of the stop.
- While the tracee is stopped, the tracer can use various `ptrace` commands to inspect and modify the tracee.
- The Tracer then causes the tracee to continue.

## 4. Attaching tracees

Tradition approach:

- Attaching a process that already exists: `ptrace(PTRACE_ATTACH, pid, 0, 0)`.
- Successful `PTRACE_ATTACH` sends `SIGSTOP` to the tracee.
- Attaching a process that has to be created first:

```c
pid = fork();
if (pid == 0) {
    ptrace(PTRACE_TRACEME, 0, 0, 0);
    raise(SIGSTOP);
    // ...
}
```

- Set options for the new tracee during the first stop: `ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_flags)`.
- The first SIGSTOP is suppressed by the tracer.

In modern approach (since Linux 3.4 released = 12 years ago):

- Attaching a process that already exists: `ptrace(PTRACE_SEIZE, pid, 0, PTRACE_O_flags)`.
- `PTRACE_SEIZE` does not stop the new tracee, there is a separate `ptrace(PTRACE_INTERRUPT, pid, 0, 0)` command to stop it without interfering with signals.
- Attaching a process that has to be created first:

```c
pid = fork();
if (pid == 0) {
    raise(SIGSTOP);
    // ...
}

waitpid(pid, &status, WSTOPPED);
ptrace(PTRACE_SEIZE, pid, 0, PTRACE_O_flags);
```

## 5. Types of ptrace stops

Any stopped state in which the tracee is ready to accept ptrace commands. Every time the tracee enters a ptrace stop state, the tracer is notified.

- Signal delivery stops: Just prior to signal delivery.
- Syscall stops: Just prior to entering and exiting a system call.
- Group stops: When a tracee receives a stopping signal.
- `PTRACE_EVENT_*` stops: Depending on PTRACE_O_* options set.

### 5.1. Signal delivery stops

Conditions:

- Each time a signal is about to be delivered.
- Even if the signal is being ignored.
- Except `SIGKILL` which causes no signal delivery stop.
- The signal is not yet delivered to the tracee.
- Reported by `wait` family syscalls with `WIFSTOPPED(status) && WSTOPSIG(status) == sig`.

Actions:

- The corresponding siginfo structure can be retrieved using `ptrace(PTRACE_GETSIGINFO, pid, 0, &siginfo)` and modified using `ptrace(PTRACE)SETSIGINFO, pid, 0, &siginfo)`.
- The tracer restarts the tracee with `ptrace(PTRACE_retart, pid, 0, sig)` where `PTRACE_restart` is one of ptrace restart operations.
- If `sig == 0` then the signal is not delivered.
- Otherwise, signal `sig` is injected.
- Injected signal `sig` can differ from `WSTOPSIG(status)`.

### 5.2. Syscall stops

Conditions:

- When the tracee is restarted from a ptrace stop using `ptrace(PTRACE_SYSCALL, pid, 0, sig)`.
- It will stop just prior to entering the next system call.
- And just prior to exiting the current system call.
- Reported by `wait()` family syscalls with either `WSTOPSIG(status) == SIGTRAP`, or, if `PTRACE_O_TRACESYSGOOD` option was set, `WSTOPSIG(status) == (SIGTRAP|0x80)`.
- Makes it easy to distinguish signal and syscall stops.

Syscall and signal stops:

- Signal stop never happens between syscall enter and exit stops.
- When syscall is interrupted by signal, the signal delivery stop happens after syscall exit stop.

What can happen with a tracee between syscall enter and exit stops?

- Stop in a `PTRACE_EVENT_*` stop.
- Exit, if it entered `_exit` or `exit_group` syscall.
- Be killed by `SIGKILL`.
- Die silently, if it is a thread group leader, the `execve()` happened in another thread, and that thread is not traced by the same tracer.

How to distinguish syscall enter and exit stops?

- Traditional approach: keep track of the sequence of ptrace stops.
- Modern approach: Use `PTRACE_GET_SYSCALL_INFO`:

```c
struct ptrace_syscall_info psi;
ptrace(PTRACE_GET_SYSCALL_INFO, pid, sizeof(psi), &psi);
```

`PTRACE_GET_SYSCALL_INFO` API:

```c
struct ptrace_syscall_info {
    __u8 op;    /* Type of system call stop. */
    /*...*/
}
```

- `op == PTRACE_SYSCALL_INFO_ENTRY` means it is a syscall enter stop.
- `op == PTRACE_SYSCALL_INFO_EXIT` means it is a syscall exit stop.

### 5.3. Syscall stops: obtaining syscall information

How the tracer can inspect syscall number, arguments and exit status?

The ancient method:

- To inspect registers: `ptrace(PTRACE_PEEKUSER, pid, offset, 0)`.
- Painfully slow: one word at a time.

The very old method:

- To fetch all registers in one go: `ptrace(PTRACE_GETREGS, pid, 0, &user_regs_struct)`.
- Not available on modern architecture.

The old method:

- To fetch all registers in one go: `ptrace(PTRACE_GETREGSET, pid, NT_PRSTATUS, &iovec)`.

The problems with the register fetching approach.

- Very architecture-dependent.
- Unreliable at least on one popular 64-bit architecture.

What's wrong there?

- Process bitness does not have to match syscall bitness.
- 64-bit processes can invoke both 64-bit and 32-bit system calls.
- No reliable way to tell them apart by inspecting registers.

#### 5.3.1. Get system call info with PTRACE_GET_SYSCALL_INFO

```c
struct ptrace_syscall_info {
    __u8 op;        /* Type of system call stop */
    __u32 arch;     /* AUDIT_ARCH_* value; see seccomp(2) */
    __u64 instruction_pointer; /* CPU instruction pointer */
    __u64 stack_pointer;    /* CPU stack pointer */
    union {
        struct {    /* op == PTRACE_SYSCALL_INFO_ENTRY */
            __u64 nr;       /* System call number */
            __u64 args[6];  /* System call arguments */
        } entry;
        struct {    /* op == PTRACE_SYSCALL_INFO_EXIT */
            __s64 rval;     /* System call return value */
            __u8 is_error;  /* System call error flag;
                               Boolean: does rval contain
                               an error value (-ERRCODE) or
                               a nonerror return value? */
        } exit;
        struct {    /* op == PTRACE_SYSCALL_INFO_SECCOMP */
            __u64 nr;       /* System call number */
            __u64 args[6];  /* System call arguments */
            __u32 ret_data; /* SECCOMP_RET_DATA portion
                               of SECCOMP_RET_TRACE
                               return value */
        } seccomp;
    };
};
```

## 6. Accessing tracee memory

Inspect and modifying the tracee memory.

The old method:

- To inspect memory: `ptrace(PTRACE_PEEKDATA, pid, addr, 0)`.
- To modify memory: `ptrace(PTRACE_POKEDATA, pid, addr, word)`.
- Painfully slow: one word at a time.

The modern method:

- To inspect memory: `process_vm_readv()`.
- To modify memory: `process_vm_writev()`.
- Allows tracers to read/write pages from/to tracee's memory.

## 7. Seccomp-assisted tracing and syscall filtering

`strace --seccomp-bpf`:

- Automatically generates and attachs a BPF program to filter syscalls.
- Make execution of untraced syscalls two orders of magnitude faster.

- Using seccomp-bpf faster than normal ptrace ~ 40times.

## 8. PTRACE_EVENT_SECCOMP

Conditions:

- `PTRACE_O_TRACESECCOMP` option is set.
- `PTRACE_EVENT_SECCOMP` stop occurs whenever a `SECCOMP_RET_TRACE` rule of a seccomp filter is triggered.
- Independent of the method was used to restart the tracee.

Actions:

- Similar to syscall enter stops.
- `PTRACE_CONT` is used instead of `PTRACE_SYSCALL` on exiting syscall.

## 9. Following forks

The stone age (before 2.5.46):

- Quite a bit of work to follow clones, forks, and vforks.
- new process had to be created using `clone()` syscall with `CLONE_PTRACE` flag set.
- on entering `clone(), fork()` and `vfork()` syscalls, the tracer had to change them to `clone()` with appropriate flags.
- `strace` had to turn `vfork()` into `fork()` by stripping `CLONE_VFORK` flag.

The modern era:

- Introduced new options: `PTRACE_O_TRACECLONE`, `PTRACE_O_TRACEFORK` and `PTRACE_O_TRACEVFORK`.
- Newly created threads are traced automatically.
- The tracee will enter the corresponding `PTRACE_EVENT_*` stops.
- The tracer can find out the new pid in the tracer namespace using `ptrace(PTRACE_GETEVENTMSG, pid, 0, &msg)`.

Related `PTRACE_EVENT_*` stops:

- `PTRACE_EVENT_CLONE` - Stop before return from clone.
- `PTRACE_EVENT_FORK` - stop before return from fork.
- `PTRACE_EVENT_VFORK` - stop before return from vfork.

## 10. Following execs

What happens when a thread in a thread group calls `execve()`.

- All other threads in the process are destroyed.
- The thread ID of the thread that invoked `execve()` is set to the thread group ID.
- At completion of the system call, it appears as though the `execve()` occurred in the thread group leader, regardless of which thread did the `execve()`.
- This resetting of the thread ID looks very confusing to tracers.

Solution:

- Set `PTRACE_O_TRACEEEXEC` option.
- It enables `PTRACE_EVENT_EXEC` stop, which occurs before `execve()` returns.
- In this stop, the trace can use `ptrace(PTRACE_GETEVENTMGR, pid, 0, &msg)` to retrieve the tracee's former thread ID.
- When the tracer receives `PTRACE_EVENT_EXEC` stop notification, it is guaranteed that except this tracee and the thread group leader no other threads from the process are alive.