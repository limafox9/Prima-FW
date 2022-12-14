
#include "led_task.h"

extern osMessageQId myQueue01Handle;

void StartLedTask() {

	osEvent  evt;
	uint16_t flash;

	for (;;) {
	evt = osMessageGet(myQueue01Handle, osWaitForever);  // wait for message
		if (evt.status == osEventMessage) {
			flash = evt.value.p;
			for (int i = 0; i < flash; i++) {
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
				osDelay(200);
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);
				osDelay(200);
			}
		}
	}

}
