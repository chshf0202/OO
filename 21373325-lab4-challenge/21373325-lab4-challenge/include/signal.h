#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <env.h>
#include <queue.h>
#include <trap.h>

//sigprocmask operands
#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

//signal actions num
#define SIG_KILL 9
#define SIG_SEGV 11
#define SIG_TERM 15

//signal mask set
typedef struct Sigset_t {
   	 int sig[2];	//at most 64 kinds of signal
} sigset_t;

//signal action
struct sigaction {
	void (*sa_handler)(int);
    	sigset_t sa_mask;
};

//signal
struct Signal {
	LIST_ENTRY(Signal) sig_link;
	int signum;
	//int sigstate;
};

LIST_HEAD(sig_list, Signal);

//signal trapframe
/*struct SigTf {
	LIST_ENTRY(SigTf) sig_tf_link;
	struct Trapframe sig_tf;
}

LIST_HEAD(sig_tf_list, SigTf);
*/

//signal structure allocate
int sig_alloc(struct Signal **new_sig);

//handle signal
void do_signal(struct Trapframe *tf);

//signal init and free
void sig_init(u_int envid, u_int parent_id);
void sig_free(u_int envid);

//default sa_handler
//void sig_kill(int signum);
//void sig_segv(int signum);
//void sig_term(int signum);

#endif






