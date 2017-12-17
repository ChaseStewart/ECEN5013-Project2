#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mqueue.h>
#include <errno.h>
#include <pthread.h>

#include "../BeagleboneBlack/inc/common.h"
#include "sysUnderTest.h"
#include "unity.h"
#include "unit_test.h"

#define WEIRD_INVALID_QUEUE 1345 
void test_queue(void)
{
        mqd_t main_queue ;
	int retval; 
	printf("Close all queues by running sudo rm -rf /dev/mqueue/*\n");
        printf("press enter to continue\n");
        getchar();

        printf("First ensure project 1 is running!\n");
        printf("if it is, close it!\n");
        printf("press enter to continue\n");
        getchar();

        printf("Test open w wrong name\n");
        main_queue = mq_open("definitely wrong name", O_CREAT | O_WRONLY | O_NONBLOCK, 0755, NULL);
        TEST_ASSERT_EQUAL_INT32(-1, (int32_t)main_queue);
        TEST_ASSERT_EQUAL_INT32(EINVAL, (int32_t ) errno);

        printf("Test open w different wrong name, should fail\n");
        main_queue = mq_open("almostright\x00", O_WRONLY | O_NONBLOCK, 0755, NULL);
        TEST_ASSERT_EQUAL_INT32(-1, (int32_t)main_queue);
        
	printf("Test open w right name\n");
        main_queue = mq_open(MAIN_QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, 0755, NULL);
        TEST_ASSERT_GREATER_THAN(-1, (int32_t)main_queue);
        
	printf("test close invalid queue \n");
	retval = mq_close(WEIRD_INVALID_QUEUE);	
        TEST_ASSERT_EQUAL_INT32(-1, (int32_t)retval);
        TEST_ASSERT_EQUAL_INT32(EBADF, (int32_t ) errno);
	
	printf("test close valid queue \n");
	retval = mq_close(main_queue);	
        TEST_ASSERT_EQUAL_INT32(0, (int32_t)retval);

}

void test_messages(void)
{
        mqd_t main_queue ;
	int retval, mylen; 
	char message[] = "Hello";	
	char in_buffer[1024];
	
	mylen= strlen(message);

	TEST_ASSERT_GREATER_THAN(0, 1);
	printf("Test open w right name\n");
        main_queue = mq_open(MAIN_QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, 0755, NULL);
        TEST_ASSERT_GREATER_THAN(-1, (int32_t)main_queue);
       
	retval = mq_send(main_queue, (const char *) &message, mylen, 0 );
        TEST_ASSERT_EQUAL_INT32(0, (int32_t)retval);
	
	retval = mq_receive(main_queue, in_buffer, SIZE_MAX, NULL );
        TEST_ASSERT_GREATER_THAN(0, (int32_t)retval);
 
}

void test_currentTemperature_celcius(void)
{
	int16_t temp = 6656;
	int16_t ret;
	ret = tempConversion(temp);
	TEST_ASSERT_EQUAL_INT16(ret, 26);
}

void test_tempConversionPositive(void)
{
	int16_t temp = 0x1700;
	temp = tempConversion(temp);
	TEST_ASSERT_EQUAL_INT16(temp,23);
}

void test_tempConversion0(void)
{
	int16_t temp = 0;
	temp = tempConversion(temp);
	TEST_ASSERT_EQUAL_INT16(temp,0);
}

void test_tempConversionNegative(void )
{
	int16_t temp = 0xFF00;
	temp = tempConversion(temp);
	TEST_ASSERT_EQUAL_INT16(temp,-1);	
}

void test_lightConversionPos(void)
{
	float ret = 0;
	ret = lightConversion(2.0, 4.0);
	TEST_ASSERT_EQUAL_FLOAT(ret, 0);
}

void test_lightConversion1(void)
{
	float ret = 0;
	ret = lightConversion(2.0, 1.0);
	TEST_ASSERT_EQUAL_FLOAT(ret, -0.026);
}	

void test_lightConversion2(void)
{
	float ret = 0;
	ret = lightConversion(20.0, 15.0);
	TEST_ASSERT_EQUAL_FLOAT(ret, 0.0265);
}	
int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_queue);
	RUN_TEST(test_messages);
	RUN_TEST(test_currentTemperature_celcius);
	RUN_TEST(test_tempConversionPositive);
	RUN_TEST(test_tempConversion0);
	RUN_TEST(test_tempConversionNegative);
	RUN_TEST(test_lightConversionPos);
	RUN_TEST(test_lightConversion1);
	RUN_TEST(test_lightConversion2);
	return UNITY_END();
}
