//
// Copyright (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __MALICIOUSREGULATOR_BURST_H__
#define __MALICIOUSREGULATOR_BURST_H__

#include <map>
#include <queue>
#include "AFDXDefs.h"
#include "AFDXMessage_m.h"
#include "NetworkStatistics.h"

namespace afdx {

//
// Malicious regulator for periodic burst injection (S2-3).
// targetVLID is accumulated up to burstSize frames, then released together
// in the same simulation instant. Every other VL sharing this regulator is
// paced honestly, with the same per-VL BAG scheme as RegulatorLogic.
//
class MaliciousRegulator_Burst : public cSimpleModule
{
private:
    typedef int TVirtualLinkId;
    typedef std::map<TVirtualLinkId, bool> BagFlagsPerVL_t;
    typedef std::map<TVirtualLinkId, std::queue<cMessage*>> BagQueuePerVL_t;

    int maxVLIDQueueSize; // size limit of the honest per-VL queue, non-target VLs only
    int targetVLID;
    int burstSize;

    // Honest per-VL BAG pacing state (RegulatorLogic scheme), used for every VL except targetVLID.
    BagFlagsPerVL_t bagFlagsPerVL;
    BagQueuePerVL_t bagQueue;

    // Burst accumulator for targetVLID only.
    std::queue<cMessage*> burstAccumulator;

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

public:
    virtual ~MaliciousRegulator_Burst();
};

}
;
// namespace afdx

#endif
