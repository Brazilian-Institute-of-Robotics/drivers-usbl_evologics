#ifndef _DUMMYPROJECT_IMDATA_TYPES_HPP_
#define _DUMMYPROJECT_IMDATA_TYPES_HPP_
#include <string>
#include <stdint.h>
#include <vector>
#include <base/Time.hpp>
namespace usbl_evologics
{

    // Kind of Command sent to AUV
    enum IMData
    {
        // Pose. 0x50 (P)
        POSE = 0x50,
        // Base command to AUV. 0x43 (C)
        AUVCOMMAND = 0x43
    };

    // Kind of Command sent to AUV
    enum Command2AVU
    {
        // Go to surface. 0x53 (S)
        SURFACE = 0x53,
        // Back to DockStation. 0x44 (D)
        DOCK = 0x44,
        // Do specific task. To be defined.
        TASK0 = 0x30,
        TASK1 = 0x31,
        TASK2 = 0x32,
        TASK3 = 0x33,
        TASK4 = 0x34,
        TASK5 = 0x35,
        TASK6 = 0x36,
        TASK7 = 0x37,
        TASK8 = 0x38,
        TASK9 = 0x39
    };

    enum Frame
    {
        // USBL frame. 0x55 (U)
        USBL = 0x55,
        // World frame, if applicable . 0x57 (W)
        WORLD = 0x57,
    };

    struct CommandAUV
    {
        base::Time time;
        Command2AVU command;
    };

    struct AUVPose
    {
        base::Time time;
        int64_t x;
        int64_t y;
        int64_t z;
        int64_t roll;
        int64_t pitch;
        int64_t yaw;
        int64_t accuracy;
        Frame frame;
    };
}
#endif
