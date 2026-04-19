/*
 * system_flags.h
 *
 *  Created on: Jan 20, 2026
 *      Author: kaver
 */

#ifndef INC_SYSTEM_FLAGS_H_
#define INC_SYSTEM_FLAGS_H_

#include "cmsis_os.h"

/* Event flag bits */
#define OBSTACLE_PRESENT        (1U << 0)
#define ESP_APRILTAG_ENABLE     (1U << 1)
#define SYSTEM_START    		(1U << 2)

/* Global event flag handle */
extern osEventFlagsId_t obstacleFlags;


#endif /* INC_SYSTEM_FLAGS_H_ */
