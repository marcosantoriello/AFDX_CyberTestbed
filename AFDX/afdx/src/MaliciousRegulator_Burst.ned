//
// Copyright (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package afdx;

//
// Malicious regulator for periodic burst injection.
// Every VL except targetVLID is paced honestly, exactly like RegulatorLogic.
// targetVLID is instead accumulated: frames are held until burstSize of them
// have arrived, then released together in the same simulation instant. Over
// time the average release rate for targetVLID still matches its declared
// BAG (nothing is added or withheld), only the temporal shape changes.
//
simple MaliciousRegulator_Burst like IRegulatorLogic
{
    @display("i=block/mac");
    int maxVLIDQueueSize = default(1000); // size limit of the honest per-VL queue, non-target VLs only
    int targetVLID; // VL-ID subjected to burst shaping; every other VL is paced honestly
    int burstSize = default(2); // number of frames accumulated before a release (N)
    gates:
        input in[];
        output out;
}
