/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>


#include "memory_manager.h"

#define MODULE_NAME main

LOG_MODULE_REGISTER(MODULE_NAME, LOG_LEVEL_DBG);


int main(void)
{
	int ret;
	size_t count = 0;
	uint32_t buffer = 0;
	const char* test_label[] = {"test_val_1", "test_val_2", "test_val_3"};

	LOG_DBG("Start NVS Test");
	LOG_DBG("KEYS: [%s], [%s], [%s]", test_label[0], test_label[1], test_label[2]);

	ret = memory_manager_init();
	if (ret) {
		LOG_DBG("Error Code: [%d]", ret);
	}

	while(1) {

		k_msleep(1000);
		LOG_DBG("---------------------------------");
		for(size_t i = 0; i < g_num_items; i++) {
			
			ret = memory_load(test_label[i], (uint8_t *)&buffer, sizeof(buffer));
			if (ret) {
				LOG_DBG("Error Code: [%d]", ret);
			}
			buffer = i + count;
			ret = memory_save(test_label[i], (uint8_t *)&buffer, sizeof(buffer));
			if (ret) {
				LOG_DBG("Error Code: [%d]", ret);
			}
			LOG_WRN("%s.value = %u", test_label[i], buffer);
			count++;
		}	
	}
	return 0;
}
