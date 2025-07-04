#include <env.h>
#include <pmap.h>
#include <printk.h>
#include <trap.h>

extern void handle_int(void);
extern void handle_tlb(void);
extern void handle_sys(void);
extern void handle_mod(void);
extern void handle_reserved(void);
extern void handle_adel(void);
extern void handle_ades(void);

void (*exception_handlers[32])(void) = {
	[0 ... 31] = handle_reserved,
	[0] = handle_int,
	[2 ... 3] = handle_tlb,
	[4] = handle_adel,
	[5] = handle_ades,
#if !defined(LAB) || LAB >= 4
	[1] = handle_mod,
	[8] = handle_sys,
#endif
};

/* Overview:
 *   The fallback handler when an unknown exception code is encountered.
 *   'genex.S' wraps this function in 'handle_reserved'.
 */
void do_reserved(struct Trapframe *tf)
{
	print_tf(tf);
	panic("Unknown ExcCode %2d", (tf->cp0_cause >> 2) & 0x1f);
}

void do_adel(struct Trapframe *tf)
{
	// 在此实现相应操作以使修改后指令符合要求
	Pte *ppte;
	int *pte = page2kva(page_lookup(curenv->env_pgdir, tf->cp0_epc, ppte));
	int num = (*pte) >> 22 & 31;
	int addr = tf->regs[num] + (*pte) & 0xffff;
	addr = addr & ~0x3;
	int imm = addr - tf->regs[num];
	*pte = ((*pte) & 0xffff0000) | imm;
	printk("AdEL handled, new imm is : %04x\n", (*pte) & 0xffff); // 这里的 new_inst 替换为修改后的指令
}

void do_ades(struct Trapframe *tf)
{
	// 在此实现相应操作以使修改后指令符合要求
	Pte *ppte;
	int *pte = page2kva(page_lookup(curenv->env_pgdir, tf->cp0_epc, ppte));
	int num = (*pte) >> 22 & 31;
	int addr = tf->regs[num] + (*pte) & 0xffff;
	addr = addr & ~0x3;
	int imm = addr - tf->regs[num];
	*pte = ((*pte) & 0xffff0000) | imm;
	printk("AdES handled, new imm is : %04x\n", (*pte) & 0xffff);
}
