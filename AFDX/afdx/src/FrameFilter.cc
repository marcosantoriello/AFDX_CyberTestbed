//
// Copyright (C) 2013 OpenSim Ltd.
//
// Modified: Copyright (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

/*
 * FrameFilter enforces the ARINC 664 P7 frame size constraints at the
 * switch ingress port. Frames outside the valid Ethernet size range
 * [L_MIN=64B, L_MAX=1518B] are dropped.
 */

#include "FrameFilter.h"

namespace afdx {

Define_Module(FrameFilter);

void FrameFilter::initialize()
{

}

void FrameFilter::handleMessage(cMessage *msg)
{
    AFDXMessage *afdxMessage = check_and_cast<AFDXMessage*>(msg);

    int frameSize = afdxMessage->getByteLength();

    // drop frames with illegal frame size
    if (frameSize < L_MIN || frameSize > L_MAX) {

        int vlId = afdxMessage->getVirtualLinkId();

        NetworkStatistics::SwitchDefinition sw = NetworkStatistics::getInstance()->getSwitchDefinition(this);
        // Record the drop only on Switch A to avoid double-counting
        // due to the dual-redundant (A/B) switch architecture.
        if (NetworkStatistics::getInstance()->isInSwitchAPort(sw)) {
            NetworkStatistics::getInstance()->createRecorder(DROPPED_FRAMES_FRAME_FILTER_PER_VL, vlId);
            NetworkStatistics::getInstance()->record(DROPPED_FRAMES_FRAME_FILTER_PER_VL, vlId, 1.0);

            EV << "FrameFilter: dropping frame on VL" << vlId
                            << " - Invalid Frame Size of " << frameSize << " bytes.\n";

                    delete msg;
                    return;
        }
    }

    // Frame is within valid size bounds: forward to the next stage
    send(msg, "out");
}

}
;
// namespace afdx

