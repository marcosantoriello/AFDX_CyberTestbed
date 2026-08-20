//
// Copyright (C) 2013 OpenSim Ltd.
// Copyright (C) 2021 Emre Atik
// Copyright (C) 2022 Ipek Gokce
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __TRAFFICPOLICY_H__
#define __TRAFFICPOLICY_H__

#include <map>

#include "AFDXDefs.h"
#include "NetworkStatistics.h"

namespace afdx {

class TrafficPolicy : public cSimpleModule
{
private:
    typedef int VirtualLinkId_t;
    typedef double TokenCredit_t;
    typedef std::map<VirtualLinkId_t, TokenCredit_t> TokenBucketMap_t;
    typedef std::map<VirtualLinkId_t, double> LastTimeRecordMap_t;
    // VLID -> {sigma_bit, rho_bit}, read once from an independent switch-resident
    // table at initialize(), never from the frame being policed.
    typedef std::map<VirtualLinkId_t, std::pair<int, int>> PolicingTableMap_t;
    TokenBucketMap_t tokenBucket;
    LastTimeRecordMap_t lastTime;
    PolicingTableMap_t policingTable;
    double minJitter;
    int bandwidthMbps;
    std::string configTableName;

    void getPolicingTable(PolicingTableMap_t &table, const char *fileName);

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

}
;
// namespace afdx

#endif
