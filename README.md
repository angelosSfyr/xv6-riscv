# XV6-COW-LAB
Implemented the copy-on-write mechanism for the xv6-riscv os.

**lab** : https://pdos.csail.mit.edu/6.S081/2020/labs/cow.html

# XV6-RISCV
xv6 is a re-implementation of Dennis Ritchie's and Ken Thompson's UnixVersion 6 (v6).
xv6 loosely follows the structure and style of v6,
but is implemented for a modern RISC-V multiprocessor using ANSI C.

# Copy On Write Explanation
The fork() system call in xv6 copies all of the parent process's user-space memory into the child. If the parent is large, copying can take a long time. Worse, the work is often largely wasted; for example, a fork() followed by exec() in the child will cause the child to discard the copied memory, probably without ever using most of it. On the other hand, if both parent and child use a page, and one or both writes it, a copy is truly needed.The goal of copy-on-write (COW) fork() is to defer allocating and copying physical memory pages for the child until the copies are actually needed, if ever.

# Goal
The task is to implement copy-on-write fork in the xv6 kernel. If the modified kernel executes both the cowtest and usertests programs successfully.

# Builing and running XV6

You will need a RISC-V "newlib" tool chain from
https://github.com/riscv/riscv-gnu-toolchain, and qemu compiled for
riscv64-softmmu. Once they are installed, and in your shell
search path, you can run "make qemu".
