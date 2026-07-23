//
// Copyright (C) 2013 OpenSim Ltd.
// Copyright (C) 2022 Ipek Gokce
//
// Modified: Copyright (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "IntegrityChecker.h"
#include "AFDXMessage_m.h"
#include "NetworkStatistics.h"

namespace afdx {

Define_Module(IntegrityChecker);

void IntegrityChecker::initialize()
{
    expectedSeqNum = 0;
}

bool IntegrityChecker::isSequenceNumberOk(int psn, int seqNo)
{
    //seqNo : [psn+1,psn+2]
    bool ret = false;
    int psnPlusOne = 0;
    int psnPlusTwo = 0;

    if ((psn != 0) && (seqNo == 0)) {
        ret = true;
    }
    else {
        if (psn >= 255) {
            psnPlusOne = 1;
            psnPlusTwo = 2;
        }
        else if (psn == 254) {
            psnPlusOne = 255;
            psnPlusTwo = 1;
        }
        else {
            psnPlusOne = psn + 1;
            psnPlusTwo = psn + 2;
        }

        if ((seqNo == psnPlusOne) || (seqNo == psnPlusTwo)) {
            ret = true;
        }
    }

    return ret;
}
void IntegrityChecker::handleMessage(cMessage *msg)
{
    // filter out redundant messages
    AFDXMessage *afdxMsg = check_and_cast<AFDXMessage*>(msg);
    int vlId = afdxMsg->getVirtualLinkId();

    //Check skew max

    // If first reception of this VLID, initialize psn
    if (psn.find(vlId) == psn.end()) {
        psn[vlId] = -1; //In order to handle seqNo = 0 case, psn is initialized with -1 for the first message of a vl
    }

    //This whether SN of successive messages are OK
    int currSeqNum = afdxMsg->getSeqNum();
    if (true == isSequenceNumberOk(psn[vlId], currSeqNum)) {
        send(afdxMsg, "out");
    }
    else {
        // Record the drop only on the "A" copy to avoid double-counting
        // due to the dual-redundant (A/B) network architecture.
        if (0 == strcmp(this->getName(), "integrityCheckerA")) {
            NetworkStatistics::getInstance()->createRecorder(DROPPED_FRAMES_INTEGRITY_CHECK_PER_VL, vlId);
            NetworkStatistics::getInstance()->record(DROPPED_FRAMES_INTEGRITY_CHECK_PER_VL, vlId, 1.0);
        }
        delete msg;
    }

    this->psn[vlId] = currSeqNum; //save this
}
}
;
// namespace afdx

