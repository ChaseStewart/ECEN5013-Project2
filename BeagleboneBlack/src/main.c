/*****************************************************
 * Redistribution, modification or use of this software in source or binary forms
 * is permitted as long as the files maintain this copyright. Users are permitted to
 * modify this and use it to learn about the field of embedded software but don't copy
 * my (Chase E Stewart's) work for class, I worked really hard on this. Alex Fosdick and
 * the University of Colorado and Chase E Stewart are not liable for any misuse of this material.
 * License copyright (C) 2017 originally from Alex Fosdick, code by Chase E Stewart.
 *****************************************************/
/*
 * @file main.c
 * @brief A server to receive commands in userspace and then process them
 *
 * @author Chase E Stewart
 * @date November 30 2017
 * @version 1.0
 *
 */

#include "common.h"
#include "main.h"

/* buffers for input and output */
//static char to_send[INPUT_LEN];
static char in_message[INPUT_LEN];
int server_state = STATE_RUNNING;
logger_args *my_log_args;

/* Initialize all states to started */
volatile int logger_state = STATE_RUNNING;
volatile int main_state   = STATE_STARTUP;
volatile int socket_state = STATE_RUNNING;

void my_print_help(void)
{
	printf("Usage: plant_server [-f] filename [-h]\n");
}

void handleCtrlC(int sig)
{
	printf("Got here!\n");
	main_state = STATE_SHUTDOWN;
	raise(HEARTBEAT_SIGNO);
}

int update_data(int msg_type, int value)
{
	printf("Would have input value %d for type %d", value, msg_type);
	return 0;
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
	
	/* Create queue for light thread */
	(*socket_queue) = mq_open(SOCKET_QUEUE_NAME, O_CREAT | O_WRONLY, 0755, NULL);
	if ((*socket_queue) == (mqd_t) -1)
	{
		printf("Failed to initialize queue! Exiting...\n");
		return 1;
	}
	return 0;	
}

int main(int argc, char **argv)
{
	/* socket state vars */ 
	int socket_id, client_id, retval, client_sockaddr_len, input_value, recv_valid_msg, curr_arg;
	int *thread_client_id;
	struct sockaddr_in bbg_server;
	char out_file_name[MAX_FILELEN];
	char *first_word;
	char *second_word;

	/* pthread and queue vars */
	pthread_t logger_thread, socket_thread;
	mqd_t main_queue, logger_queue, socket_queue;

	/* if no args provided, print help and exit */
	if (argc == 1)
	{
		my_print_help();
		return 0;
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
	
	/* catch Necessary Signals */
	signal(SIGINT, handleCtrlC);
	signal(SIGTERM, handleCtrlC);
	
	/* initialize queues */
	retval = initMainQueues(&main_queue, &logger_queue, &socket_queue);
	printf("Initialized queues!\n");

	/* block for needed signals */
	retval = blockAllSigs();
	if (retval != 0)
	{
		printf("Failed to set sigmask.\n");
		return 1;
	}


	/* create the logger thread */
	my_log_args = (logger_args *)malloc(sizeof(logger_args));
	my_log_args->filename = out_file_name;
	my_log_args->length = strlen(out_file_name);
	if(pthread_create(&logger_thread, NULL, mainLogger, my_log_args) != 0)
	{
		printf("Failed to create logger thread!\n");
		return 1;
	}
	
	/* establish socket */
	socket_id = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_id == -1)
	{
		printf("[bbg_server] could not create socket\n");
	}

	/* setup bind params and then bind */
	bbg_server.sin_addr.s_addr = inet_addr(BBG_SERVER_HOST);
	bbg_server.sin_family = AF_INET;
	bbg_server.sin_port = htons(BBG_SERVER_PORT);
	if (bind(socket_id, (struct sockaddr *)&bbg_server, sizeof(bbg_server)) < 0)
	{
		printf("[bbg_server] Failed to bind to port %d\n", BBG_SERVER_PORT);
		return 1;
	}

	/* listen for MAX_NUM_CONNS simultaneous connections*/
	listen(socket_id, MAX_NUM_CONNS);
	printf("[bbg_server] Listening on %s:%d\n", BBG_SERVER_HOST, BBG_SERVER_PORT);

	/* For each accepted connection, listen for messages on client_id*/
	client_sockaddr_len = sizeof(struct sockaddr_in);

	while (server_state == STATE_RUNNING)
	{
		/* Now spawn a thread to handle the connection */
		client_id = accept(socket_id, (struct sockaddr *)&client_id, (socklen_t *)&client_sockaddr_len );
		if (client_id < 0)
		{
			printf("[bbg_server] Failed to accept connection\n");
			continue;
		}
		printf("[bbg_server] Connected!\n");
		
		/* create the socket thread */
		if(pthread_create(&socket_thread, NULL, mainSocket, &client_id) != 0)
		{
			logFromMain(logger_queue, LOG_CRITICAL, "Failed to create socket thread!\n");
			return 1;
		}
		/*** FIX THIS ****/
		retval = recv(client_id, in_message, INPUT_LEN, 0);
		if(retval < 0)
		{
			printf("[bbg_server] Failed to receive from remote server, got value %d and errno %d \n", retval, errno);
			server_state = STATE_ERROR;
		}
		else if (retval == 0 )
		{
			printf("[bbg_server] Connection closed by client\n");
			server_state = STATE_SHUTDOWN;
		}
		else 
		{

			first_word = strtok(in_message, DELIM_STR);
			if (strcmp(first_word, "light") == 0)
			{
				second_word = strtok(NULL, DELIM_STR);
				sscanf(second_word, "%d", &input_value);
				if (input_value > LIGHT_MAX || input_value < LIGHT_MIN)
				{
					printf("Invalid input light value\n");
				}
				else
				{
					update_data(LIGHT_DATA, input_value);
				}					
			}
			else if (strcmp(first_word, "temp") == 0)
			{
				second_word = strtok(NULL, DELIM_STR);
				sscanf(second_word, "%d", &input_value);
				if (input_value > TEMP_MAX || input_value < TEMP_MIN)
				{
					printf("Invalid input temp value\n");
				}
				else
				{
					update_data(TEMP_DATA, input_value);
				}					
			}
			else if (strcmp(first_word, "moist") == 0)
			{
				second_word = strtok(NULL, DELIM_STR);
				sscanf(second_word, "%d", &input_value);
				if (input_value > HUMID_MAX || input_value < HUMID_MIN)
				{
					printf("Invalid input humid value\n");
				}
			}

		}
	}
	printf("[bbg_server] Terminating server\n");
	close(socket_id);
	return server_state;
}


int8_t logFromMain(mqd_t queue, int prio, char *message)
{
	int retval;
	message_t msg;

	/* create packet and send it */
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
	return 0;
}
