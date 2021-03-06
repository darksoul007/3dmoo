#ifndef __ARM_REGFORMAT_H__
#define __ARM_REGFORMAT_H__

enum arm_regno{
	R0 = 0,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
	R8,
	R9,
	R10,
	R11,
	R12,
	R13,
	LR,
	R15, //PC,
	CPSR_REG,
	SPSR_REG,
#if 1
	PHYS_PC,
	R13_USR,
	R14_USR,
	R13_SVC,
	R14_SVC,
	R13_ABORT,
	R14_ABORT,
	R13_UNDEF,
	R14_UNDEF,
	R13_IRQ,
	R14_IRQ,
	R8_FIRQ,
	R9_FIRQ,
	R10_FIRQ,
	R11_FIRQ,
	R12_FIRQ,
	R13_FIRQ,
	R14_FIRQ,
	SPSR_INVALID1,
	SPSR_INVALID2,
	SPSR_SVC,
	SPSR_ABORT,
	SPSR_UNDEF,
	SPSR_IRQ,
	SPSR_FIRQ,
	MODE_REG, /* That is the cpsr[4 : 0], just for calculation easily */
	BANK_REG,
	EXCLUSIVE_TAG,
	EXCLUSIVE_STATE,
	EXCLUSIVE_RESULT,
	CP15_BASE,
	CP15_C0 = CP15_BASE,
	CP15_C0_C0 = CP15_C0,
	CP15_MAIN_ID = CP15_C0_C0,
	CP15_CACHE_TYPE,
	CP15_TCM_STATUS,
	CP15_TLB_TYPE,
	CP15_C0_C1,
	CP15_PROCESSOR_FEATURE_0 = CP15_C0_C1,
	CP15_PROCESSOR_FEATURE_1,
	CP15_DEBUG_FEATURE_0,
	CP15_AUXILIARY_FEATURE_0,
	CP15_C1_C0,
	CP15_CONTROL = CP15_C1_C0,
	CP15_AUXILIARY_CONTROL,
	CP15_COPROCESSOR_ACCESS_CONTROL,
	CP15_C2,
	CP15_C2_C0 = CP15_C2,
	CP15_TRANSLATION_BASE = CP15_C2_C0,
	CP15_TRANSLATION_BASE_TABLE_0 = CP15_TRANSLATION_BASE,
	CP15_TRANSLATION_BASE_TABLE_1,
	CP15_TRANSLATION_BASE_CONTROL,
	CP15_DOMAIN_ACCESS_CONTROL,
	CP15_RESERVED,
	/* Fault status */
	CP15_FAULT_STATUS,
	CP15_INSTR_FAULT_STATUS,
	CP15_COMBINED_DATA_FSR = CP15_FAULT_STATUS,
	CP15_INST_FSR,
	/* Fault Address register */
	CP15_FAULT_ADDRESS,
	CP15_COMBINED_DATA_FAR = CP15_FAULT_ADDRESS,
	CP15_WFAR,
	CP15_IFAR,
	CP15_PID,
	CP15_CONTEXT_ID,
	CP15_THREAD_URO,
	CP15_TLB_FAULT_ADDR, /* defined by SkyEye */
	CP15_TLB_FAULT_STATUS, /* defined by SkyEye */
	/* VFP registers */
	VFP_BASE,
	VFP_FPSID = VFP_BASE,
	VFP_FPSCR,
	VFP_FPEXC,
#endif	
	MAX_REG_NUM,
};

#define VFP_OFFSET(x) (x - VFP_BASE)
#endif
