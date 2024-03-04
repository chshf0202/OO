//lab 4 challenge

#include <env.h>
#include <pmap.h>

extern struct Env *env;

struct sig_list free_sig_list;
//struct sig_tf_list free_tf_list;

int sig_alloc(struct Signal **new_sig) {
	if (LIST_EMPTY(&free_sig_list)) {
        	struct Page *p;
        	try(page_alloc(&p));
	    	p->pp_ref++;
		struct Signal *s = (struct Signal *)page2kva(p);
		int n = BY2PG / sizeof(struct Signal);
		int i;
        	for (i = 0; i < n; i++) {
           		LIST_INSERT_HEAD(&free_sig_list, s + i, sig_link);
        	}
    	}
    	struct Signal *s = LIST_FIRST(&free_sig_list);
    	LIST_REMOVE(s, sig_link);
    	*new_sig = s;
    	return 0;
}

extern void do_signal(struct Trapframe *tf);

void do_signal(struct Trapframe *tf) {
	//printk("in do signal\n");
	//check return
	if (LIST_EMPTY(&curenv->env_sig_pending)) {
		//printk("no sig pending, return, curenv:%x, epc:%x\n", curenv->env_id, tf->cp0_epc);
		return;
	}
	sigset_t mask = curenv->env_sig_mask;
	//LIST_FOREACH(s, &curenv->sig_waiting_list, sig_link) {
	//	if (s->sigstate == SIG_RUNNING) {
	//		mask.sig[0] |= curenv->env_sigaction[s->signum - 1].sa_mask.sig[0];
	//		mask.sig[1] |= curenv->env_sigaction[s->signum - 1].sa_mask.sig[1];
	//	}
	//}
	struct Signal *s = LIST_FIRST(&curenv->env_sig_pending);
	//printk("sig now: %d, mask: %08x%08x\n", curenv->env_sig_now, mask.sig[1], mask.sig[0]);
	if (curenv->env_sig_now) {
		mask = curenv->env_sigaction[curenv->env_sig_now - 1].sa_mask;
		//printk("sig now: %d, mask: %08x%08x\n", curenv->env_sig_now, mask.sig[1], mask.sig[0]);
	}
	if ((mask.sig[(s->signum - 1) / 32] & (1 << ((s->signum - 1) % 32))) != 0 && s->signum != SIG_KILL) {
		//printk("sig %d masked %d%d, return, curenv:%x\n", s->signum, mask.sig[1], mask.sig[0], curenv->env_id);
		return;
	}
	if ((s->signum == SIG_KILL || s->signum == SIG_SEGV || s->signum == SIG_TERM) && !curenv->env_sig_entry) {
		env_destroy(curenv);
	}
	LIST_REMOVE(s, sig_link);
        LIST_INSERT_HEAD(&free_sig_list, s, sig_link);
	//save context
	struct Trapframe tmp_tf = *tf;
	if (tf->regs[29] < USTACKTOP || tf->regs[29] >= UXSTACKTOP) {
		tf->regs[29] = UXSTACKTOP;
	}
	tf->regs[29] -= sizeof(struct Trapframe);
	*(struct Trapframe *)tf->regs[29] = tmp_tf;
	tf->regs[4] = tf->regs[29];
	tf->regs[5] = s->signum;
	tf->regs[29] -= sizeof(tf->regs[4]) + sizeof(tf->regs[5]);
	tf->cp0_epc = curenv->env_sig_entry;
	//do signal
	curenv->env_sig_now = s->signum;
	//printk("finish do signal, go to sig entry\n");
}

extern struct Env envs[NENV] __attribute__((aligned(BY2PG)));

void sig_init(u_int envid, u_int parent_id) {
    	struct Env *e;
    	if (envid == 0) {
		e = curenv;
    	} else {
	    	e = envs + ENVX(envid);
    	}
    	if (parent_id == 0) {
        	memset(e->env_sigaction, 0, 64 * sizeof(struct sigaction));
		memset(&e->env_sig_mask, 0, sizeof(sigset_t));
		memset(&e->env_sig_entry, 0, sizeof(u_int));
		memset(&e->env_sig_pending, 0, sizeof(struct sig_list));
		memset(&e->env_sig_now, 0, sizeof(int));
    	} else {
        	struct Env *p;
        	panic_on(envid2env(parent_id, &p, 0));
        	memcpy(e->env_sigaction, p->env_sigaction, 64 * sizeof(struct sigaction));
		memcpy(&e->env_sig_entry, &p->env_sig_entry, sizeof(u_int));
		memcpy(&e->env_sig_mask, &p->env_sig_mask, sizeof(sigset_t));
    	}
}

void sig_free(u_int envid) {
    	struct Env *e;
    	if (envid == 0) {
	    	e = curenv;
    	} else {
		e = envs + ENVX(envid);
    	}
    	struct Signal *s;
    	while(!LIST_EMPTY(&e->env_sig_pending)) {
		s = LIST_FIRST(&e->env_sig_pending);
        	LIST_REMOVE(s, sig_link);
        	LIST_INSERT_HEAD(&free_sig_list, s, sig_link);
    	}
}







