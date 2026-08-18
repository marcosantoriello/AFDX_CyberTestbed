//
// Copyright (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

/*
 * MaliciousRegulator_PartialViolation reproduces the honest regulator's
 * per-VL BAG pacing, but releases frames against a reduced BAG:
 * BAG_effective = BAG * bagReductionFactor. The declared BAG carried by the
 * frame is left untouched, so the switch still polices the VL against the
 * contract the ES claims to respect.
 */

#include "MaliciousRegulator_PartialViolation.h"

namespace afdx {

Define_Module(MaliciousRegulator_PartialViolation);

void MaliciousRegulator_PartialViolation::initialize()
{
    this->maxVLIDQueueSize = par("maxVLIDQueueSize");
    this->bagReductionFactor = par("bagReductionFactor").doubleValue();

    // A factor of 0 would schedule every release at the current instant, i.e. no
    // pacing at all: that is full BAG bypass, already modelled by
    // MaliciousRegulator_Flooding. Reject it here instead of silently degenerating.
    if (this->bagReductionFactor <= 0.0 || this->bagReductionFactor > 1.0) {
        throw cRuntimeError("bagReductionFactor must be in (0.0, 1.0]; "
                "use MaliciousRegulator_Flooding for full BAG bypass");
    }
}

void MaliciousRegulator_PartialViolation::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        //Let's say a message is received & bag_queue[vlid] was empty -> afdxMsg is sent out.
        //In this case a self message is scheduled for BAG time later. When received, bag_queue will be checked for any other messages that
        //might have received in the mean time.
        int vlId = msg->getKind();
        if (bagQueue[vlId].empty()) {
            // no messages received in the mean time
            bagFlagsPerVL[vlId] = 0;
            delete msg;

        }
        else // some message received in the mean time.
        {
            // Pop and send that message
            // Schedule a self message for the REDUCED bag time later to process further messages(if any)
            AFDXMessage *afdx_msg = check_and_cast<AFDXMessage*>(bagQueue[vlId].front());
            double BAG = afdx_msg->getBagValue() * this->bagReductionFactor;
            send(bagQueue[vlId].front(), "out");

            bagQueue[vlId].pop();
            scheduleAt(simTime() + BAG, msg);
        }
    }
    else {
        //If queue is already empty, send message directly and schedule a self message for BAG time later. In the mean time:
        // if message received, push to queue and wait for scheduled self message to come.
        // else, bag_flags_per_vl[ vl_id ] flag will be cleaned by the code above
        AFDXMessage *afdx_msg = check_and_cast<AFDXMessage*>(msg);
        // Same instant at which the honest regulator timestamps the frame: the ES
        // BAG latency is computed downstream in RedundancyController as
        // simTime() - RegLogEntryTime, so this must not be moved.
        afdx_msg->setRegLogEntryTime(simTime());
        int vlId = afdx_msg->getVirtualLinkId();

        BagFlagsPerVL_t::iterator itr = bagFlagsPerVL.find(vlId);
        if ((itr != bagFlagsPerVL.end()) && (bagFlagsPerVL[vlId] == 1)) {
            //Queue is not empty! Push and wait for already scheduled self-message to be sent after BAG(s) of time.
            if (bagQueue[vlId].size() >= this->maxVLIDQueueSize) {
                throw std::runtime_error(
                        std::string("Max limit for VLID queue is reached.(vlid:" + std::to_string(vlId) + ")"));
            }

            bagQueue[vlId].push(msg);
        }
        else	//This is the first message for this VLID or queue is empty
        {
            bagFlagsPerVL[vlId] = 1; //raise the flag to notify upper code about there is a message in the queue

            cMessage *bagTimer = new cMessage("bag_timer");
            bagTimer->setKind(vlId);
            scheduleAt(simTime() + afdx_msg->getBagValue() * this->bagReductionFactor, bagTimer);

            send(msg, "out"); //send away! Nothing to wait :)
        }
    }
}

}
;
// namespace afdx
