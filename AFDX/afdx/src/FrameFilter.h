//
// Copyright (C) 2013 OpenSim Ltd.
//
// Modified: Copyright (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __FRAMEFILTER_H__
#define __FRAMEFILTER_H__

#include "AFDXDefs.h"
#include "AFDXMessage_m.h"
#include "NetworkStatistics.h"

namespace afdx {

class FrameFilter : public cSimpleModule
{
private:
    static const int L_MIN = 64; // IEEE 802.3 minimum frame size (bytes)
    static const int L_MAX = 1518; // ARINC 664 P7 maximum frame size (bytes)

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

}
;
// namespace afdx

#endif
