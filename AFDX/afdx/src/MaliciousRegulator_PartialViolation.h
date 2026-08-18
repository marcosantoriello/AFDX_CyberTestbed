//
// Copyright (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __MALICIOUSREGULATOR_PARTIALVIOLATION_H__
#define __MALICIOUSREGULATOR_PARTIALVIOLATION_H__

#include <map>
#include <queue>
#include "AFDXDefs.h"
#include "AFDXMessage_m.h"
#include "NetworkStatistics.h"

namespace afdx {

//
// Malicious regulator for partial BAG violation.
// Same per-VL pacing structure as RegulatorLogic, except that the BAG carried
// by each frame is scaled down by bagReductionFactor before it is used to
// schedule the next release.
//
class MaliciousRegulator_PartialViolation : public cSimpleModule
{
private:
    typedef int TVirtualLinkId;
    typedef std::map<TVirtualLinkId, bool> BagFlagsPerVL_t;
    typedef std::map<TVirtualLinkId, std::queue<cMessage*>> BagQueuePerVL_t;

    int maxVLIDQueueSize; // size limit of the queue(for each VL id)
    double bagReductionFactor; // BAG_effective = BAG * bagReductionFactor
    BagFlagsPerVL_t bagFlagsPerVL;
    BagQueuePerVL_t bagQueue;

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

}
;
// namespace afdx

#endif
