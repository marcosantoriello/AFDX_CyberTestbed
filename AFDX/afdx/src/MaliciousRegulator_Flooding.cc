//
// Copyright (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "MaliciousRegulator_Flooding.h"
#include "AFDXMessage_m.h"

namespace afdx {

Define_Module(MaliciousRegulator_Flooding);

void MaliciousRegulator_Flooding::handleMessage(cMessage *msg)
{
    // malicious ES ignores BAG: forward every frame immediately
    AFDXMessage *afdxMsg = check_and_cast<AFDXMessage*>(msg);
    afdxMsg->setRegLogEntryTime(simTime());
    send(msg, "out");
}

} // namespace afdx
