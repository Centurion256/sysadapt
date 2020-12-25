# Sysadapt

Sysadapt is a tool that translates legacy `x86-32` ABI syscalls into `AMD64` fast syscalls using `ptrace`.

## Usage
Sysadapt comes with a shared library for invoking legacy syscalls and a sample victim program. 

**Build with CMake**
```bash
cmake [-DVERBOSE]
```
`VERBOSE`: compile with verbosity. Refer to source code for more information

**Run**
```bash
./build/sysadapt build/call < in.txt
```
## Runner

The victim program provided with sysadapt simply invokes two syscalls: `read` and `write` in order to echo `Hello, World!` from stdin into stdout. However, it uses `32-bit legacy ABI` to write from the buffer. This is generally considered to be **undefined behaviour**, because `32-bit legacy ABI` uses `EIP`, which is a 32-bit register, to access the 64-bit address space. When run under `sysadapt`, as shown in usage section, the syscall will be dynamically  translated into the `64-bit ABI` at runtime, and therefore the program will succeed.

## Syscall32 library

The shared library provided with sysadapt contains a variadic `syscall32` function which invokes a `legacy 32-bit ABI` syscall from given arguments. It also includes `write32` and `read32`, which are simple wrappers over `syscall32`. In general, `syscall32` can be used for any `32-bit` syscall.

## References

If you wish to learn more about how `sysadapt` operates or x86 syscalls on Linux in general, refer to these documents

-  [Ptrace man page](https://man7.org/linux/man-pages/man2/ptrace.2.html)
-  [x86-32 and 64 syscall tables](https://chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md#x86_64-64_bit)
-  [Using C to inspect syscalls](https://ops.tips/gists/using-c-to-inspect-linux-syscalls/)
-  [Intercepting Linux syscalls with ptrace](https://nullprogram.com/blog/2018/06/23/)
-  [Anatomy of a system call](https://lwn.net/Articles/604515/)
-  [Linux-Insides: Syscalls](https://0xax.gitbooks.io/linux-insides/content/SysCall/linux-syscall-2.html)
-  [Difinitive guide to syscalls](https://blog.packagecloud.io/eng/2016/04/05/the-definitive-guide-to-linux-system-calls/#legacy-system-calls)
-  [Linus Torvalds on this issue](https://lore.kernel.org/lkml/CA+55aFzcSVmdDj9Lh_gdbz1OzHyEm6ZrGPBDAJnywm2LF_eVyg@mail.gmail.com/)