//
// Copyright (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __MALICIOUSREGULATOR_FLOODING_H__
#define __MALICIOUSREGULATOR_FLOODING_H__

#include <omnetpp.h>

using namespace omnetpp;

namespace afdx {

//
// Malicious regulator for VL Flooding.
// Bypasses BAG scheduling: every received frame is forwarded immediately.
//
class MaliciousRegulator_Flooding : public cSimpleModule
{
  protected:
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace afdx

#endif
