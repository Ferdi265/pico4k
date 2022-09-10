#pragma once
#ifdef __ASSEMBLER__

.syntax unified

.macro static_assert value, msg
.if !(\value)
.err \msg
.endif
.endm

#endif
