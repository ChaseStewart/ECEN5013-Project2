#include "common.h"
#include "my_socket.h"

void * mainSocket(void *arg)
{
	sigset_t set;
	int retval, sig, sock_handle;
	struct sigevent my_sigevent;
	mqd_t socket_queue, main_queue, logger_queue;
	message_t *in_message;
	char in_buffer[MAX_MSGLEN];
	int *type_enum;
	int *temp_data;
	int *light_data;
	int *humid_data;

	/* initialize the queue. */
	retval = initSocketQueues(&main_queue, &logger_queue, &socket_queue);
	if (retval != 0)
	{
		printf("[socket_thread] Failed to init socket queues\n");
	}
	
	/* register to receive socket signals */
	my_sigevent.sigev_notify = SIGEV_SIGNAL;
	my_sigevent.sigev_signo  = SOCKET_SIGNO;
	if (mq_notify(socket_queue, &my_sigevent) == -1 )
	{
		printf("[socket_thread] Failed to notify!\n");
		return NULL;
	}
	
	retval = blockAllSigs();
	if (retval != 0)
	{
		printf("[socket_thread] failed to set sigmask.\n");
		return (void *) 1;
	}

	sock_handle = *(int *) arg;
	printf("[socket_thread] created socket\n");
	printf("[socket_thread] socket ID is %d\n", sock_handle);
	
	if(retval < 0)
	{
		logFromSocket(logger_queue, LOG_CRITICAL, "Failed to init socket\n");
	}

	sigemptyset(&set);
	sigaddset(&set, SOCKET_SIGNO);
	/* this is the main loop for the program */
	while(socket_state > STATE_SHUTDOWN)
	{
		retval = recv(sock_handle, &in_buffer, MAX_MSGLEN, 0);
		if (retval > 0)
		{
			sscanf(in_buffer, "%d:%d:%d:%d", &type_enum, &temp_data, &light_data, &humid_data);
			printf("[socket_thread] type is %d, temp is %d, light is %d, humidity is %d\n", type_enum, temp_data, light_data, humid_data);
		
		}
		else if ((retval == 0))
		{
			pthread_exit(NULL);
		}
		else if ((retval < 0) && (errno != EINTR)) 
		{
			printf("[socket_thread] Failed to receive with retval %d and errno %d\n", retval, errno);
			socket_state = STATE_SHUTDOWN;
		}

		/* NOTE: this call is allowed to fail */
		mq_notify(socket_queue, &my_sigevent);
		
		in_message = (message_t *) malloc(sizeof(message_t));
		errno = 0;
		while(errno != EAGAIN){
			retval = mq_receive(socket_queue, in_buffer, SIZE_MAX, NULL);
			if (retval <= 0 && errno != EAGAIN)
			{
				continue;
			}
			in_message = (message_t *)in_buffer;

			/* process Log*/
			if (in_message->id == SOCKET)
			{
				retval = sendMessage(in_message);
			} 
			else if (in_message->id == HEARTBEAT_REQ) 
			{
				sendHeartbeat(main_queue, SOCKET_ID);
			}
		}
	}
	printf("[socket_thread] Destroyed Socket\n");
	logFromSocket(logger_queue, LOG_INFO, "Destroyed Socket\n");
	pthread_exit(NULL);
}

int8_t initSocketQueues(mqd_t *main_queue, mqd_t *logger_queue, mqd_t *socket_queue)
{

	/* Create queue for main thread */
	(*main_queue) = mq_open(MAIN_QUEUE_NAME, O_CREAT | O_WRONLY, 0755, NULL);
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

	/* Create queue for socket thread */
	(*socket_queue) = mq_open(SOCKET_QUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, 0755, NULL);
	if ((*socket_queue) == (mqd_t) -1)
	{
		printf("Failed to initialize queue! Exiting...\n");
		return 1;
	}

	return 0;
}

int8_t logFromSocket(mqd_t queue, int prio, char *message)
{
	int retval;
	message_t msg;

	msg.id = LOGGER;
	msg.timestamp = time(NULL);
	msg.length = strlen(message);
	msg.priority = prio;
	msg.source = SOCKET_ID;
	msg.message = message;

	/* intentionally don't use queue- no need */
	retval = mq_send(queue, (const char *) &msg, sizeof(message_t), 0);
	if (retval == -1)
	{
		printf("Failed to send from main to queue! Exiting...\n");
		return 1;
	}
	return 0;
}

int8_t sendMessage(message_t *in_message)
{
	printf("[socket_thread] TODO write this\n");
	return 1;

}
