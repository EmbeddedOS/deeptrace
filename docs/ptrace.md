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
