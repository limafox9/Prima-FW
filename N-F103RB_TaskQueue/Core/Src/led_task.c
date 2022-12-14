
#include "led_task.h"

void StartLedTask() {

	for (;;) {
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		osDelay(200);
	}

}
