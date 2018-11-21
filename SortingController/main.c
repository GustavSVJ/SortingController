#include "regulator.h"
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <sched.h>
#include <rtai_shm.h>
#include <rtai_lxrt.h>
#include <pthread.h>
#include <comedilib.h>
#include <math.h>

char stopflag = 0;

static double shute_b0 = 85.88;
static double shute_b1 = -73.42;
static double shute_a1 = -0.4378;

static double belt_b0 = 3.996;
static double belt_b1 = -3.738;
static double belt_a1 = -1;

unsigned int shute_ref = 3200;
unsigned int belt_ref = 169;
double belt_ref_ms = 0.2;


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

	FILE *fp;
	FILE *fp2;

	fp = fopen("LogOut.txt", "w+");
	fp2 = fopen("DetectorLog.txt", "w+");

	fprintf(fp, "Time; A0in; A0out; A1in; A1out;\n");

	mlockall(MCL_CURRENT | MCL_FUTURE);
	const struct sched_param params = { .sched_priority = sched_get_priority_max(SCHED_FIFO) };
	sched_setscheduler(0, SCHED_FIFO, &params);

	RT_TASK *task = rt_task_init(nam2num("h1901"), 0, 0, 0);


	rt_task_make_periodic_relative_ns(task, 0, 2000000);

	statetype shute_reg, belt_reg;
	regul_init(&shute_reg);
	regul_init(&belt_reg);

	comedi_t * hw = comedi_open("/dev/comedi2");

	comedi_data_write(hw, 1, 1, 0, AREF_GROUND, 2048);
	unsigned int beltOffset;
	comedi_data_read_delayed(hw, 0, 2, 0, AREF_DIFF, &beltOffset, 50000);
	printf("The belt value was %d when the belt was stopped\n", beltOffset);

	int j = 0;

	/*Length detection variables*/
#define beltSpeedAverage 200
	unsigned int detectorCounter = 0;
	unsigned int beltSpeedCounter = 0;
	unsigned int beltSpeed[beltSpeedAverage];

	/*End of length detection variables*/

	long long int timeOld = rt_get_time_ns();


	while (stopflag == 0) {

		long long int timeRelative = rt_get_time_ns() - timeOld;
		double time = timeRelative;
		fprintf(fp2, "%lf\n", time);

		/*Length detection*/

		unsigned int data;
		comedi_data_read_delayed(hw, 0, 2, 0, AREF_DIFF, &data, 50000); //read sensor into data if there is a stick (value above 3000) or not (0)
	
		if (data > 3000) { //count upwards while sensing a stick

			detectorCounter++;
		}
		else if (detectorCounter != 0) { // calculate length in objectLength as soon as the stick has passed the sensor
			printf("The counter was %d\n", detectorCounter);
			
			/*
			double avgBeltSpeed = 0;
			
			int i = 0;
			for (i = 0; i < beltSpeedAverage; i++) {
				avgBeltSpeed += beltSpeed[i];
			}

			avgBeltSpeed = (((avgBeltSpeed / beltSpeedAverage) / 4096) * 20) - 10;

			avgBeltSpeed = avgBeltSpeed * 384.6153846 * 0.1047197551 * 0.1666666667 * 0.036;

			*/

			double objectLength = detectorCounter * 0.002 * belt_ref_ms * 100;

			printf("The average belt speed was %f and the object was %f cm and the counter was %d\n", belt_ref_ms, objectLength, detectorCounter);

			detectorCounter = 0;

			if (3.75 < objectLength && objectLength < 6.25) {
				printf("The stick was 5 centimeters long!\n");
				shute_ref = 3600;
			}
			else if(6.25 < objectLength && objectLength < 8.75) {
				printf("The stick was 7.5 centimeters long!\n");
				shute_ref = 3250;
			}
			else if (8.75 < objectLength && objectLength < 11.25) {
				printf("The stick was 10 centimeters long!\n");
				shute_ref = 3000;
			}
			else {
				printf("Unknown length!\n");
			}
		}

		/*End of length detection*/

		if (j) {
			j = 0;

			//Shute regulator
			unsigned int data;


			comedi_data_read_delayed(hw, 0, 0, 0, AREF_DIFF, &data, 50000);
			fprintf(fp, "%f;%d;", time, data);
			int e = (shute_ref - data);

			regul_out(&shute_reg, e, shute_b0);

			regul_update(&shute_reg, e, shute_b1, shute_a1);

			if (shute_reg.u > 2045) {
				data = 4095;
			}
			else if (shute_reg.u < -2045) {
				data = 0;
			}
			else {
				data = (unsigned int)shute_reg.u + 2048;
			}

			comedi_data_write(hw, 1, 0, 0, AREF_GROUND, data);
			fprintf(fp, "%d;", data);
			//Belt Regulator
			comedi_data_read_delayed(hw, 0, 1, 0, AREF_DIFF, &data, 50000);

			int inputData = data - beltOffset;
			e = (belt_ref - inputData);
			fprintf(fp, "%d;", data);
			beltSpeed[beltSpeedCounter] = data;
			if (beltSpeedCounter < beltSpeedAverage - 1) {
				beltSpeedCounter++;
			}
			else {
				beltSpeedCounter = 0;
			}

			regul_out(&belt_reg, e, belt_b0);

			regul_update(&belt_reg, e, belt_b1, belt_a1);

			if (belt_reg.u > 2045) {
				data = 4095;
			}
			else if (belt_reg.u < 0) {
				data = 2048;
			}
			else {
				data = (unsigned int)belt_reg.u + 2048;
			}

			comedi_data_write(hw, 1, 1, 0, AREF_GROUND, data);

			fprintf(fp, "%d;\n", data);
		}
		else {
			j = 1;
		}

		rt_task_wait_period();
	}

	comedi_data_write(hw, 1, 1, 0, AREF_GROUND, 2048);
	comedi_data_write(hw, 1, 0, 0, AREF_GROUND, 2048);
	comedi_close(hw);
	fclose(fp);
	fclose(fp2);
	rt_task_delete(task);

	return NULL;

}

int main()
{

	char *input;
	pthread_t thread;
	char controllerStatus = 0;

	rt_allow_nonroot_hrt();

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

		else if (strcmp(input, "read") == 0) {
			comedi_t * hw = comedi_open("/dev/comedi2");

			unsigned int data;
			comedi_data_read_delayed(hw, 0, 1, 0, AREF_DIFF, &data, 50000);
			printf("The reading was %d\n", data);

			comedi_close(hw);
		}

		else if (strcmp(input, "beltRef") == 0) {
			printf("Type a new reference speed for the belt in [m/s]\n");
			scanf("%lf", &belt_ref_ms);
			printf("The typed reference was %lf\n", belt_ref_ms);

			belt_ref = round(204.8 * 4.138028520 * belt_ref_ms);

			printf("Resulting in a reference of %d\n", belt_ref);

		}

		else if (strcmp(input, "shuteRef") == 0) {
			printf("Type a new reference for the shute\n");
			scanf("%d", &shute_ref);
			printf("The typed ref was %d\n", shute_ref);
		}

	}

	return 0;
}