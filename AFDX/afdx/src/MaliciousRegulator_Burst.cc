//
// Copyright (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

/*
 * MaliciousRegulator_Burst reproduces the honest regulator's per-VL BAG
 * pacing for every VL except targetVLID. Frames of targetVLID are instead
 * accumulated: they are held in a queue until burstSize of them have
 * arrived, then all released together in the same simulation instant. The
 * declared BAG/rho/sigma carried by the frame are left untouched, so the
 * average release rate for targetVLID still matches its declared contract -
 * only the release timing changes.
 */

#include "MaliciousRegulator_Burst.h"

namespace afdx {

Define_Module(MaliciousRegulator_Burst);

MaliciousRegulator_Burst::~MaliciousRegulator_Burst()
{
    // A burstSize that does not evenly divide the total number of frames
    // generated leaves a residual partial burst in the accumulator that is
    // never released. Those frames are owned by this module, not by the
    // simulation kernel (unlike scheduled self-messages), so they must be
    // disposed of explicitly here.
    while (!burstAccumulator.empty()) {
        delete burstAccumulator.front();
        burstAccumulator.pop();
    }
}

void MaliciousRegulator_Burst::initialize()
{
    this->maxVLIDQueueSize = par("maxVLIDQueueSize");
    this->targetVLID = par("targetVLID").intValue();
    this->burstSize = par("burstSize").intValue();

    if (this->burstSize < 1) {
        throw cRuntimeError("burstSize must be >= 1");
    }
}

void MaliciousRegulator_Burst::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        // Honest BAG timer
        int vlId = msg->getKind();
        if (bagQueue[vlId].empty()) {
            bagFlagsPerVL[vlId] = 0;
            delete msg;
        }
        else {
            AFDXMessage *afdx_msg = check_and_cast<AFDXMessage*>(bagQueue[vlId].front());
            double BAG = afdx_msg->getBagValue();
            send(bagQueue[vlId].front(), "out");

            bagQueue[vlId].pop();
            scheduleAt(simTime() + BAG, msg);
        }
        return;
    }

    AFDXMessage *afdx_msg = check_and_cast<AFDXMessage*>(msg);
    // Same instant at which the honest regulator timestamps the frame: the ES BAG
    // latency computed downstream in RedundancyController (simTime() -
    // RegLogEntryTime) must include the time spent waiting in the burst
    // accumulator, which is the point of this attack.
    afdx_msg->setRegLogEntryTime(simTime());
    int vlId = afdx_msg->getVirtualLinkId();

    if (vlId == this->targetVLID) {
        burstAccumulator.push(msg);
        if ((int)burstAccumulator.size() >= this->burstSize) {
            // Release every accumulated frame in this same simulation instant; the
            // downstream transmit queue and MAC serialize them at the physical link
            // rate, exactly as they already do for any other same-instant arrivals.
            while (!burstAccumulator.empty()) {
                send(burstAccumulator.front(), "out");
                burstAccumulator.pop();
            }
        }
        return;
    }

    // Honest per-VL BAG pacing, identical to RegulatorLogic::handleMessage(), for
    // every VL other than targetVLID.
    BagFlagsPerVL_t::iterator itr = bagFlagsPerVL.find(vlId);
    if ((itr != bagFlagsPerVL.end()) && (bagFlagsPerVL[vlId] == 1)) {
        if (bagQueue[vlId].size() >= (size_t)this->maxVLIDQueueSize) {
            throw std::runtime_error(
                    std::string("Max limit for VLID queue is reached.(vlid:" + std::to_string(vlId) + ")"));
        }

        bagQueue[vlId].push(msg);
    }
    else {
        bagFlagsPerVL[vlId] = 1;

        cMessage *bagTimer = new cMessage("bag_timer");
        bagTimer->setKind(vlId);
        scheduleAt(simTime() + afdx_msg->getBagValue(), bagTimer);

        send(msg, "out");
    }
}

}
;
// namespace afdx
