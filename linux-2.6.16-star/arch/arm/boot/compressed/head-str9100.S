#include <asm/mach-types.h>

		.section	".start", "ax"

__str9100_start:
		mov	r7, #(MACH_TYPE_STR9100 & 0xFF00)
		orr	r7, r7, #(MACH_TYPE_STR9100 & 0x00FF)
