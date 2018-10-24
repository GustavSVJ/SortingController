#include "regulator.h"
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <sched.h>
#include <rtai_shm.h>
#include <rtai_lxrt.h>
#include <pthread.h>


char stopflag = 0;

/* Read a string, and return a pointer to it.  Returns NULL on EOF.
 * Taken from the readline manual (info readline).
 */
char *rl_gets(char *prompt)
{
	/* A static variable for holding the line. */
	static char *line_read = (char *)NULL;

	/* If the buffer has already been allocated, return the memory
	   to the free pool. */
	if (line_read) {
		free(line_read);
		line_read = (char *)NULL;
	}

	/* Get a line from the user. */
	line_read = readline(prompt);

	/* If the line has any text in it, save it on the history. */
	if (line_read && *line_read)  add_history(line_read);

	return line_read;
}

void *RegulatorThread(void* stuff) {
	mlockall(MCL_CURRENT | MCL_FUTURE);
	const struct sched_param params = { .sched_priority = sched_get_priority_max(SCHED_FIFO) };
	sched_setscheduler(0, SCHED_FIFO, &params);

	RT_TASK *task2 = rt_task_init(nam2num("h1901"), 0, 0, 0);

	rt_task_make_periodic_relative_ns(task2, 0, 1000000);

	while (stopflag == 0) {
	
		rt_task_wait_period();
	}

	rt_task_delete(task2);

	return NULL;

}

int main()
{
	static double b0 = 85.88;
	static double b1 = -73.42;
	static double a1 = -0.4378;

	char *input;
	pthread_t thread;
	char controllerStatus = 0;

	while ((input = rl_gets("ShuteController> "))) {

		if (strcmp(input, "help") == 0) {
			puts("Who needs that?\n");
		}

		else if (strcmp(input, "start") == 0) {
			puts("Starting the controller!\n");
			stopflag = 0;
			if (pthread_create(&thread, NULL, RegulatorThread, NULL)) {
				printf("Error: unable to create thread");
			}
			else {
				controllerStatus = 1;
				puts("Write stop to exit!\n");
			}
		}

		else if (strcmp(input, "stop") == 0 && controllerStatus == 1) {
			stopflag = 1;
			pthread_join(thread, NULL);
			controllerStatus = 0;
			puts("Controller stopped!\n");
		}

		else if ((strcmp(input, "quit") == 0) || (strcmp(input, "exit") == 0)) {
			break;
		}

	}

	/*

	statetype reg;

	regul_init(&reg);

	int i;
	for (i = 0; i < 100; i++) {
		double e = 1;
		regul_out(&reg, e, 85.88);
		printf("Value = %f\n", reg.u);
		regul_update(&reg, e, -73.42, -0.4378);
	}

	*/

	return 0;
}