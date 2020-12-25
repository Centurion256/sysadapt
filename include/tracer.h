#ifndef TRACER_INCLUDE_GUARD
#define TRACER_INCLUDE_GUARD

#include <stdint.h>
#include <sys/user.h>
#include <byteswap.h>

#define SYSCALL_INSTR 0x0f05
#define INT80_INSTR 0xcd80

//A mapping from 32-bit ABI into 64-bit ABI
//Refer to chromium.googlesource.com/chromiumos/docs/+/master/constants/syscalls.md for in-depth x86 and x64 syscall reference
static int32_t const SYSCALL_MAP[431] = {
    219, 60, 57, 0, 1, 2, 3, -1, 85, 86, 87, 520, 80, 201, 133, 90, 94, -1, -1, 8, 39, 165, -1, 105, 102, -1, 521, 37, -1, 34, 132, -1, -1, 21, -1, -1, 162, 62, 82, 83, 84, 32, 22, 100, -1, 12, 106, 104, -1, 107, 108, 163, 166, -1, 514, 72, -1, 109, -1, -1, 95, 161, 136, 33, 110, 111, 112, -1, -1, -1, 113, 114, -1, -1, 170, 160, 97, 98, 96, 164, 115, 116, 23, 88, -1, 89, 134, 167, 169, -1, 9, 11, 76, 77, 91, 93, 140, 141, -1, 137, 138, 173, -1, 103, 38, 36, 4, 6, 5, -1, 172, 153, -1, -1, 61, 168, 99, -1, 74, -1, 56, 171, 63, 154, 159, 10, -1, 174, 175, 176, 177, 179, 121, 81, -1, 139, 135, 183, 122, 123, -1, 78, -1, 73, 26, 515, 516, 124, 75, 156, 149, 150, 151, 152, 142, 143, 144, 145, 24, 146, 147, 148, 35, 25, 117, 118, -1, 178, 7, 180, 119, 120, 157, 513, 512, 14, 522, 523, 524, 130, 17, 18, 92, 79, 125, 126, 525, 40, 181, 182, 58, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 155, 27, 28, 217, -1, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, -1, 202, 203, 204, 205, 211, 543, 207, 208, 544, 210, 221, 231, 212, 213, 233, 232, 216, 218, 526, 223, 224, 225, 226, 227, 228, 229, 230, -1, -1, 234, 235, -1, 236, 237, 239, 238, 240, 241, 242, 243, 527, 245, 528, 529, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, -1, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 530, 531, 275, 277, 276, 532, 533, 309, 281, 280, 282, 283, 284, 285, 286, 287, 289, 290, 291, 292, 293, 294, 534, 535, 536, 298, 537, 300, 301, 302, 303, 304, 305, 306, 538, 308, 539, 540, 312, 313, 314, 315, 316, 317, 318, 319, 321, 545, 41, 53, 49, 42, 50, 288, 542, 541, 51, 52, 44, 518, 517, 519, 48, 323, 324, 325, 326, 546, 547, 329, 330, 331, 332, 158, 333, 334, 64, 66, 29, 31, 30, 67, 68, 69, 70, 71, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441
};
typedef struct user_regs_struct* user_regs_struct_t;
typedef struct syscall_info {
    uint16_t number; /* syscall nunber */
    uint64_t instr;  /* syscall instruction and two previous bytes of memory */
    uint64_t ip;     /* syscall instruction pointer */
    struct user_regs_struct regs; /* pointer to CPU state snapshot before syscall */
} *syscall_info_t;

//get only the relevant syscall opcode
#define SYS_GET_OPCODE(instr) (bswap_16(((instr) & 0xffff)))
//set the opcode to a new value
#define SYS_SET_OPCODE(instr, opcode) (((instr) & (0xffff << 16)) | bswap_16((opcode)))

int8_t tracee_goto_syscall(pid_t child);

int16_t tracee_get_syscall_info(pid_t child, syscall_info_t info);

int8_t tracee_convert_syscall(pid_t child, syscall_info_t info);

ssize_t tracee_syscall_restart_prepare(pid_t child, syscall_info_t info);

ssize_t tracee_syscall_restart(pid_t child, syscall_info_t info);



#endif