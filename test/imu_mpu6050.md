/*
 * test_imu.c
 *
 * Simple test for IMU + pose delta functions (no float printf support)
 */
```
#include "main.h"
#include "imu_data.h"
#include <stdio.h>
#include <stdbool.h>

void imu_test(void)
{
    float dx = 0, dy = 0, dtheta = 0;

    // Step 1: Initialize IMU
    init_imu();
    printf("=== IMU Test Started ===\n");

    // Step 2: Reset pose deltas before event (like before turn)
    get_pose_delta(&dx, &dy, &dtheta, true);
    printf("Pose reset -> dx=%ld.%03ld, dy=%ld.%03ld, dtheta=%ld.%03ld\n",
           (long)dx, (long)((dx - (long)dx) * 1000),
           (long)dy, (long)((dy - (long)dy) * 1000),
           (long)dtheta, (long)((dtheta - (long)dtheta) * 1000));

    // Step 3: Simulate reading during a "turn"
    for (int i = 0; i < 50; i++)   // 50 samples (~1s if DT=0.02)
    {
        HAL_Delay(20);  // match DT = 0.02s

        get_pose_delta(&dx, &dy, &dtheta, false);

        printf("Step %02d -> dx=%ld.%03ld, dy=%ld.%03ld, dtheta=%ld.%03ld\n",
               i,
               (long)dx, (long)((dx - (long)dx) * 1000),
               (long)dy, (long)((dy - (long)dy) * 1000),
               (long)dtheta, (long)((dtheta - (long)dtheta) * 1000));
    }

    // Step 4: End of event
    printf("=== Turn Completed ===\n");
    printf("Final delta -> dx=%ld.%03ld, dy=%ld.%03ld, dtheta=%ld.%03ld\n",
           (long)dx, (long)((dx - (long)dx) * 1000),
           (long)dy, (long)((dy - (long)dy) * 1000),
           (long)dtheta, (long)((dtheta - (long)dtheta) * 1000));
}
