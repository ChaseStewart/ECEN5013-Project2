#include "common.h"
#include "logger.h"

FILE *out_file;

void * mainLogger(void *arg)
{
	sigset_t set;
	int retval, sig;
	struct sigevent my_sigevent;
	char in_buffer[MAX_MSGLEN];
	message_t *in_message;
	mqd_t main_queue, logger_queue;
	printf("[logger_thread] Created logger thread\n");

	/* initialize the queue. */
	retval = initLoggerQueues(&main_queue, &logger_queue);
	if (retval != 0)
	{
		printf("[logger_thread] Failed to init logger queues\n");
	}
	
	/* register to receive logger signals */
	my_sigevent.sigev_notify = SIGEV_SIGNAL;
	my_sigevent.sigev_signo  = LOGGER_SIGNO;
	if (mq_notify(logger_queue, &my_sigevent) == -1 )
	{
		printf("[logger_thread] Failed to notify!\n");
		return NULL;
	}
	
	retval = blockAllSigs();
	if (retval != 0)
	{
		printf("[logger_thread] Failed to set sigmask.\n");
		return (void *) 1;
	}
	
	out_file = initLogger(logger_queue, arg);
	if(out_file)
	{
		logFromLogger(logger_queue, LOG_INFO, "Log file opened\n");
	}

	sigemptyset(&set);
	sigaddset(&set, LOGGER_SIGNO);

	/* this is the main loop for the program */
	while(logger_state > STATE_SHUTDOWN)
	{
		sigwait(&set, &sig);

		/* NOTE: this call is allowed to fail */
		mq_notify(logger_queue, &my_sigevent);

		in_message = (message_t *) malloc(sizeof(message_t));
		errno = 0;
		while(errno != EAGAIN){
			retval = mq_receive(logger_queue, in_buffer, SIZE_MAX, NULL);
			if (retval <= 0 && errno != EAGAIN)
			{
				continue;
			}
			in_message = (message_t *)in_buffer;

			/* process Log*/
			if (in_message->id == LOGGER)
			{
				logMessage(in_message);
			} 
			else if (in_message->id == HEARTBEAT_REQ) 
			{
				sendHeartbeat(main_queue, LOGGER_ID);
			}
		}
	}
	logFromLogger(logger_queue, LOG_INFO, "Destroyed Logger\n");
	fclose(out_file);	
	printf("[logger_thread] Destroying Logger\n");
	pthread_exit(NULL);
}

int8_t initLoggerQueues(mqd_t *main_queue, mqd_t *logger_queue)
{
	/* Create main queue*/
	(*main_queue) = mq_open(MAIN_QUEUE_NAME, O_CREAT | O_WRONLY , 0755, NULL );
	if ((*main_queue) == (mqd_t) -1 )
	{
		return 1;
	}

	/* Create main queue*/
	(*logger_queue) = mq_open(LOGGER_QUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK , 0755, NULL );
	if ((*logger_queue) == (mqd_t) -1 )
	{
		return 1;
	}
	printf("[logger_thread] Created Logger\n");
	return 0;
}

int8_t logMessage(message_t *in_message)
{
	int retval;
	char dbg_lvl[16];
	char mySource[16];
	char outMessage[256];

	switch(in_message->priority)
	{
		case LOG_INFO:
			strcpy(dbg_lvl, "INFO");
			break;
		case LOG_DEBUG:
			strcpy(dbg_lvl, "DEBUG");
			break;
		case LOG_ERROR:
			strcpy(dbg_lvl, "ERROR");
			break;
		case LOG_CRITICAL:
			strcpy(dbg_lvl, "CRITICAL");
			break;
		default:
			return 1;
	}
	
	switch(in_message->source)
	{
		case SOCKET_ID:
			strcpy(mySource, "SOCKET");
			break;
		case LOGGER_ID:
			strcpy(mySource, "LOGGER");
			break;
		case MAIN_ID:
			strcpy(mySource, "MAIN");
			break;
		default:
			return 1;
	}

	retval = sprintf( outMessage ,"[%s][%s][%d] %s", dbg_lvl, mySource, in_message->timestamp, in_message->message);
	if (retval <= 0)
	{
		printf("[logger_thread] Error logging output!\n");
	}
	fputs(outMessage, out_file);
	return 0;
}

FILE *initLogger(mqd_t queue, void *arg)
{
	FILE *wipeFile;
	char *logname;
	logger_args *my_log_args;
	
	/* parse logger args */
	my_log_args = (logger_args *)malloc(sizeof(logger_args));
	my_log_args = (logger_args *) arg;
	logname = my_log_args->filename;
	wipeFile = fopen(logname, "w");
	fclose(wipeFile);
	return(fopen(logname, "a"));
}

int8_t logFromLogger(mqd_t queue, int prio, char *message)
{
	int retval;
	message_t msg;

	msg.id = LOGGER;
	msg.timestamp = time(NULL);
	msg.length = strlen(message);
	msg.priority = prio;
	msg.source = LOGGER_ID;
	msg.message = message;

	/* intentionally don't use queue- no need */
	retval = logMessage(&msg);
	return retval;
}

