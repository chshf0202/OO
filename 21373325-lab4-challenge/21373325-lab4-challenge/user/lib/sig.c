//lab 4 challenge

#include <lib.h>

int sig_entry_set = 0;

static void __attribute__((noreturn)) sig_entry(struct Trapframe *tf, int signum);

static void __attribute__((noreturn)) sig_entry(struct Trapframe *tf, int signum) {
	//debugf("in sig entry with signum = %d\n", signum);
	void (*sa_handler)(int) = env->env_sigaction[signum - 1].sa_handler;
	if (sa_handler) {
		//debugf("has sa_handler: %x\n", sa_handler);
		sa_handler(signum);
		//debugf("out sa_handler\n");
	} else if (signum == SIG_KILL || signum == SIG_SEGV || signum == SIG_TERM) {
		syscall_env_destroy(0);
	}
	syscall_sig_finish();
	//debugf("do set tf\n");
	int r = syscall_set_trapframe(0, tf);
	//debugf("done set tf\n");
	user_panic("syscall_set_trapframe returned %d", r);
}

void sigemptyset(sigset_t *set) {
	memset(set, 0, sizeof(sigset_t));
}

void sigfillset(sigset_t *set) {
	memset(set, -1, sizeof(sigset_t));
}

void sigaddset(sigset_t *set, int signum) {
	set->sig[(signum - 1) / 32] |= 1 << ((signum - 1) % 32);
}

void sigdelset(sigset_t *set, int signum) {
	set->sig[(signum - 1) / 32] &= ~(1 << ((signum - 1) % 32));
}

int sigismember(const sigset_t *set, int signum) {
	return (set->sig[(signum - 1) / 32] & (1 << ((signum - 1) % 32))) == 0 ? 0 : 1;
}

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
        if (!sig_entry_set) {
                syscall_set_sig_entry(sig_entry);
                sig_entry_set = 1;
	}
	if (oldact != NULL) {
		memset(oldact, 0, sizeof(oldact));
	}
	return syscall_sigaction(signum, act, oldact);
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
	if (oldset != NULL) {
		memset(oldset, 0, sizeof(oldset));
	}
	return syscall_sigprocmask(how, set, oldset);
}

int kill(u_int envid, int sig) {
	/*if (!sig_entry_set) {
		syscall_set_sig_entry(sig_entry);
		sig_entry_set = 1;
	}*/
	return syscall_kill(envid, sig);
}





