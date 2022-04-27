#include <stdio.h>
#include <unistd.h>

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <std_msgs/msg/int32.h>

#include <rclc/rclc.h>
#include <rclc/executor.h>

#include "components/mhz19b/mhz19b.c" //wrong impementation

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

#define RCCHECK(fn)                                                                      \
	{                                                                                    \
		rcl_ret_t temp_rc = fn;                                                          \
		if ((temp_rc != RCL_RET_OK))                                                     \
		{                                                                                \
			printf("Failed status on line %d: %d. Aborting.\n", __LINE__, (int)temp_rc); \
			vTaskDelete(NULL);                                                           \
		}                                                                                \
	}
#define RCSOFTCHECK(fn)                                                                    \
	{                                                                                      \
		rcl_ret_t temp_rc = fn;                                                            \
		if ((temp_rc != RCL_RET_OK))                                                       \
		{                                                                                  \
			printf("Failed status on line %d: %d. Continuing.\n", __LINE__, (int)temp_rc); \
		}                                                                                  \
	}

rcl_publisher_t publisher;
std_msgs__msg__Int32 msg;

int16_t co2;
mhz19b_dev_t dev;
char version[6];
uint16_t range;
bool autocal;
bool sensorIsReady = false;

void timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{

	RCLC_UNUSED(last_call_time);
	if (timer != NULL)
	{
		RCSOFTCHECK(rcl_publish(&publisher, &msg, NULL));
		msg.data = co2;
	}
}

void appMain(void *arg)
{
	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

	// sesnsor init
	RCCHECK(mhz19b_init(&dev, UART_NUM_1, 12, 13));

	while (!mhz19b_detect(&dev))
	{
		printf("MHZ-19B not detected, waiting...\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	mhz19b_get_version(&dev, version);
	printf("MHZ-19B firmware version: %s", version);

	printf("MHZ-19B set range and autocal");

	mhz19b_set_range(&dev, MHZ19B_RANGE_5000);
	mhz19b_set_auto_calibration(&dev, false);

	mhz19b_get_range(&dev, &range);
	printf("range: %d", range);

	mhz19b_get_auto_calibration(&dev, &autocal);
	printf("autocal: %s", autocal ? "ON" : "OFF");

	// create init_options
	RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

	// create node
	rcl_node_t node;
	RCCHECK(rclc_node_init_default(&node, "freertos_int32_publisher", "", &support));

	// create publisher
	RCCHECK(rclc_publisher_init_default(
		&publisher,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
		"freertos_int32_publisher"));

	// create timer,
	rcl_timer_t timer;
	const unsigned int timer_timeout = 1000;
	RCCHECK(rclc_timer_init_default(
		&timer,
		&support,
		RCL_MS_TO_NS(timer_timeout),
		timer_callback));

	// create executor
	rclc_executor_t executor;
	RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
	RCCHECK(rclc_executor_add_timer(&executor, &timer));

	msg.data = 0;

	while (mhz19b_is_warming_up(&dev, true)) // use smart warming up detection
	{
		printf("MHZ-19B is warming up\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	while (1)
	{
		if (mhz19b_is_ready(&dev))
		{
			mhz19b_read_co2(&dev, &co2);
		}
		rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
		usleep(100000);
	}

	// free resources
	RCCHECK(rcl_publisher_fini(&publisher, &node))
	RCCHECK(rcl_node_fini(&node))

	vTaskDelete(NULL);
}
