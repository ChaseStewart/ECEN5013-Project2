#include "common.h"
#include "main.h"

volatile int hbt_rsp[NUM_TASKS];
volatile sig_atomic_t alrm_flag = 0;

volatile int logger_state = STATE_RUNNING;
volatile int main_state   = STATE_STARTUP;
volatile int socket_state = STATE_RUNNING;

volatile int heartbeat_period = 1;

void my_print_help(void)
{
	printf("Usage: plant_server [-f] filename [-h]\n");
}

/* set all threads to SHUTDOWN state one by one then reap all resources */
void handleCtrlC(int sig)
{
	main_state   = STATE_SHUTDOWN;
	raise(HEARTBEAT_SIGNO);
}

int main(int argc, char **argv)
{
	int retval, curr_arg, sig, socket_id, conn_id, client_sockaddr_len;
	struct sockaddr_in bbg_server;
	char out_file_name[MAX_FILELEN];
	char change_file_name[MAX_FILELEN];
	logger_args *my_log_args;
	message_t msg;
	sigset_t set;
	pthread_t socket_thread, logger_thread;
	mqd_t main_queue, logger_queue, socket_queue;
	message_t *in_message;

	struct sigevent my_sigevent;

	socket_thread = 0;

	/* if no args provided, print help and exit */
	if (argc == 1)
	{
		my_print_help();
		return 0;
	}
	

	/* catch Necessary Signals */
	signal(SIGINT, handleCtrlC);
	signal(SIGTERM, handleCtrlC);
	
	retval = blockAllSigs();
	if (retval != 0)
	{
		printf("Failed to set sigmask.\n");
		return 1;
	}

	/* do argc argv */
	curr_arg = 0;
	while(curr_arg >= 0)
	{
		curr_arg = getopt_long(argc, argv, "f:h", options, NULL);
		if (curr_arg < 0)
		{
			continue;
		}
		switch( curr_arg)
		{
			/* get output filename from -f arg */
			case 'f':
				strcpy(out_file_name, optarg);
				break;

			/* print help for */
			case 'h':
			default:
				my_print_help();
				return 0;
		}
	}

	/* create initial queues */
	retval = initMainQueues(&main_queue, &logger_queue, &socket_queue);
	if (retval != 0)
	{
		printf("Failed to init main queues!\n");
		return 1;
	}

	/* create the logger thread */
	my_log_args = (logger_args *)malloc(sizeof(logger_args));
	my_log_args->filename = out_file_name;
	my_log_args->length = strlen(out_file_name);
	if(pthread_create(&logger_thread, NULL, mainLogger, my_log_args) != 0)
	{
		printf( "Failed to create logger thread!\n");
		return 1;
	}
	
	/* prepare main loop */
	for (int idx = 0; idx< NUM_TASKS; idx++)
	{
		hbt_rsp[idx] = 0;
	}

	/* register to receive temp_driver signals */
	my_sigevent.sigev_notify = SIGEV_SIGNAL;
	my_sigevent.sigev_signo  = HEARTBEAT_SIGNO;
	if (mq_notify(main_queue, &my_sigevent) == -1 )
	{
		return -1;
	}
	
	/* begin recurring heartbeat timer */		

	sigemptyset(&set);
	sigaddset(&set, HEARTBEAT_SIGNO);
	sigaddset(&set, SIGALRM);

	/* establish socket */
	socket_id = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_id == -1)
	{
		printf("[bbg_server] could not create socket\n");
		return 1;
	}

	/* setup bind params and then bind */
	bbg_server.sin_addr.s_addr = inet_addr(BBG_SERVER_HOST);
	bbg_server.sin_family = AF_INET;
	bbg_server.sin_port = htons(BBG_SERVER_PORT);

	/* listen for MAX_NUM_CONNS simultaneous connections */
	printf("[bbg_server] Connecting to %s:%d...\n", BBG_SERVER_HOST, BBG_SERVER_PORT);

	/* For each accepted connection, listen for messages on conn_id*/
	client_sockaddr_len = sizeof(struct sockaddr_in);
	if( connect(socket_id, (struct sockaddr *)&bbg_server, client_sockaddr_len ) < 0)
	{
		printf("[bbg_server] Failed to connect!");
		logFromMain(logger_queue, LOG_CRITICAL, "Failed to connect to server!\n");
		main_state = STATE_SHUTDOWN;
	}
	printf("[bbg_server] Connected!\n");

	/* create the socket thread */
	if(pthread_create(&socket_thread, NULL, mainSocket, &socket_id) != 0)
	{
		logFromMain(logger_queue, LOG_CRITICAL, "Failed to create socket thread!\n");
		return 1;
	}
	
	logFromMain(logger_queue, LOG_INFO, "Starting main loop\n");
	alarm(heartbeat_period);

	/* main loop- go until STATE_ERROR or STATE_SHUTDOWN */
	while (main_state > STATE_SHUTDOWN)
	{
		sigwait(&set, &sig);
		if (mq_notify(main_queue, &my_sigevent) == -1 )
		{
			;
		}
		if (sig == HEARTBEAT_SIGNO)
		{
			
			char in_buffer[1024];
			in_message = (message_t *) malloc(sizeof(message_t));

			/* process all messages and set corresponding hbt_rsp entry */
			errno = 0;
			while(errno != EAGAIN){
				retval = mq_receive(main_queue, in_buffer, SIZE_MAX, NULL);
				if (retval <= 0 && errno != EAGAIN)
				{
					printf("Error in recieve: %d\n", errno);
					break;
				}
				/* process heartbeat_response messages */
				in_message = (message_t *)in_buffer;
				if (in_message->id == HEARTBEAT_RSP)
				{
					hbt_rsp[in_message->source] = 1;
				}
			}
			errno=0;
		}

		else if (sig == SIGALRM)
		{
			/* for the first time, only request a heartbeat*/
			if (main_state == STATE_STARTUP)
			{
				main_state = STATE_REQ_RSP;
				reqHeartbeats(logger_queue, socket_queue);
				pthread_kill(logger_thread, LOGGER_SIGNO);
				pthread_kill(socket_thread, SOCKET_SIGNO);
			}

			/* for the second time, read current heartbeats then request more*/
			else if (main_state == STATE_REQ_RSP)
			{
				processHeartbeats(main_queue, logger_queue);
				reqHeartbeats(logger_queue, socket_queue);
				pthread_kill(logger_thread, LOGGER_SIGNO);
				pthread_kill(socket_thread, SOCKET_SIGNO);
			}
			alarm(heartbeat_period);
		}
		else
		{
			logFromMain(logger_queue, LOG_ERROR, "Something bad happened!\n");
		}
	}
	logFromMain(logger_queue, LOG_INFO, "exiting program\n");

	/* Kill the Socket thread */
	socket_state  = STATE_SHUTDOWN;
	pthread_kill(socket_thread, SOCKET_SIGNO);
	
	/* Kill the Logger thread */
	logger_state = STATE_SHUTDOWN;
	pthread_kill(logger_thread, LOGGER_SIGNO);
	
	/* close this instance of the queue */
	retval = mq_close(main_queue);
	if (retval == -1)
	{
		printf( "Failed to reap queue\n");
		return 1;
	}

	/* close this instance of the queue */
	retval = mq_close(logger_queue);
	if (retval == -1)
	{
		printf( "Failed to reap queue\n");
		return 1;
	}
	/* close this instance of the queue */
	retval = mq_close(socket_queue);
	if (retval == -1)
	{
		printf( "Failed to reap queue\n");
		return 1;
	}

	printf("All main queues closed!\n");

	/* on close, reap socket_thread */
	if (pthread_join(socket_thread, NULL) != 0)
	{
		printf("failed to reap socket_thread\n");
		return 1;
	}

	/* on close, reap logger_thread */
	if (pthread_join(logger_thread, NULL) != 0)
	{
		printf("failed to reap logger_thread\n");
		return 1;
	}
	/* now unlink the queue for all */
	retval = mq_unlink(MAIN_QUEUE_NAME);
	if (retval == -1)
	{
		printf("Failed to unlink queue\n");
		return 1;
	}
	retval = mq_unlink(SOCKET_QUEUE_NAME);
	if (retval == -1)
	{
		printf("Failed to unlink queue\n");
		return 1;
	}
	retval = mq_unlink(LOGGER_QUEUE_NAME);
	if (retval == -1)
	{
		printf("Failed to unlink queue\n");
		return 1;
	}


	if(main_state == STATE_ERROR)
	{
		printf("[bbg_server] Exited with error\n");
	}
	else
	{
		printf("[bbg_server] Exited without error\n");
	}
	return 0;
}

/* ensure all heartbeats are received */
int8_t processHeartbeats(mqd_t main_queue, mqd_t logger_queue)
{
	int hbt_source, retval;
	char in_buffer[4096];
	char heartbeat_msg[1024];
	message_t *in_message;
	in_message = (message_t *) malloc(sizeof(message_t));

	/* process all messages and set corresponding hbt_rsp entry */
	errno = 0;
	while(errno != EAGAIN){
		retval = mq_receive(main_queue, in_buffer, SIZE_MAX, NULL);
		if (retval <= 0 && errno != EAGAIN)
		{
			break;
		}

		/* process heartbeat_response messages */
		in_message = (message_t *)in_buffer;
		if (in_message->id == HEARTBEAT_RSP)
		{
			hbt_rsp[in_message->source] = 1;
		}
	}

	/* once all queue messages processed, ensure all heartbeats were there*/
	for (int idx = 0; idx< NUM_TASKS; idx++)
	{
		if (hbt_rsp[idx] == 0)
		{
			sprintf(heartbeat_msg, "Missing heartbeat response from %d\n", idx);
			logFromMain(logger_queue, LOG_ERROR, heartbeat_msg);
			main_state = STATE_ERROR;
		}
		hbt_rsp[idx] = 0;

	}
	if (main_state == STATE_ERROR)
	{
		printf("Missing heartbeats\n");
		raise(SIGINT);
		return 1;
	}
	else
	{
		sprintf(heartbeat_msg, "Received all heartbeats\n");
		logFromMain(logger_queue, LOG_INFO, heartbeat_msg);
		return 0;
	}
}


int8_t initMainQueues(mqd_t *main_queue, mqd_t *logger_queue, mqd_t *socket_queue)
{
	/* Create queue for main thread */
	(*main_queue) = mq_open(MAIN_QUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, 0755, NULL);
	if ((*main_queue) == (mqd_t) -1)
	{
		printf("Failed to initialize queue! Exiting...\n");
		return 1;
	}

	/* Create queue for logger thread */
	(*logger_queue) = mq_open(LOGGER_QUEUE_NAME, O_CREAT | O_WRONLY, 0755, NULL);
	if ((*logger_queue) == (mqd_t) -1)
	{
		printf("Failed to initialize queue! Exiting...\n");
		return 1;
	}
	
	/* Create queue for logger thread */
	(*socket_queue) = mq_open(SOCKET_QUEUE_NAME, O_CREAT | O_WRONLY, 0755, NULL);
	if ((*socket_queue) == (mqd_t) -1)
	{
		printf("Failed to initialize queue! Exiting...\n");
		return 1;
	}
	return 0;
}


/* send heartbeat to the heartbeat queue */
int8_t reqHeartbeats(mqd_t logger_queue, mqd_t socket_queue)
{
	int idx, retval;
	message_t *msg;

	/* craft heartbeat request */
	msg = (message_t *) malloc(sizeof(message_t));
	msg->id        = HEARTBEAT_REQ;
	msg->timestamp = time(NULL);
	msg->length    = 0;
	msg->source    = MAIN_ID;
	msg->message   = NULL;

	/* send logger heartbeat */
	retval = mq_send(logger_queue, (const char *) msg, sizeof(message_t), 0);
	if (retval < 0)
	{
		printf("Failed to send to logger with retval %d and errno %d \n", retval, errno);
		return 1;
	}
	
	/* send socket heartbeat */
	retval = mq_send(socket_queue, (const char *) msg, sizeof(message_t), 0);
	if (retval < 0)
	{
		printf("Failed to send to socket_thread with retval %d and errno %d \n", retval, errno);
		return 1;
	}

	return 0;
}

int8_t logFromMain(mqd_t queue, int prio, char *message)
{
	int retval;
	message_t msg;

	msg.id = LOGGER;
	msg.timestamp = time(NULL);
	msg.length = strlen(message);
	msg.priority = prio;
	msg.source = MAIN_ID;
	msg.message = message;
	retval = mq_send(queue, (const char *) &msg, sizeof(message_t), 0);
	if (retval == -1)
	{
		printf("Failed to send from main to queue! Exiting...\n");
		return 1;
	}
}

