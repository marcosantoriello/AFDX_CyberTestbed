//
// Copyright (C) 2013 OpenSim Ltd.
// Copyright (C) 2021 Emre Atik
// Copyright (C) 2022 Ipek Gokce
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

/*
 * TODO Byte-based/frame-based filtering selection can be added.
 * In this model byte-based is applied. ACi is decreased as received frame's size each, not Smax.
 * */
#include "TrafficPolicy.h"
#include "AFDXMessage_m.h"
#include <fstream>

namespace afdx {

Define_Module(TrafficPolicy);

void TrafficPolicy::initialize()
{
    bandwidthMbps = par("bandwidth").intValue();
    minJitter = par("minJitter").doubleValue();
    minJitter /= 1000;

    this->configTableName = par("configTableName").str();
    this->configTableName = this->configTableName.substr(1, this->configTableName.length() - 2);
    this->getPolicingTable(this->policingTable, configTableName.c_str());
}

void TrafficPolicy::getPolicingTable(PolicingTableMap_t &table, const char *fileName)
{
    std::ifstream fileStream(fileName);

    if (!(fileStream.good())) {
        throw std::runtime_error(std::string("Policing Table Not Found!"));
    }

    std::size_t indexBegin;
    std::size_t indexEnd;

    int key;

    for (std::string line; std::getline(fileStream, line);) {
        if (line.empty() || line.at(0) == '*') {
            continue;
        }

        if ((indexEnd = line.find(':')) == line.npos) {
            throw std::runtime_error(std::string("Invalid Policing Table!"));
        }
        key = std::stoi(line.substr(0, indexEnd), 0, 16);

        if ((indexEnd = line.find('{')) == line.npos) {
            throw std::runtime_error(std::string("Invalid Policing Table!"));
        }
        indexBegin = indexEnd + 1;

        if ((indexEnd = line.find(',', indexBegin)) == line.npos) {
            throw std::runtime_error(std::string("Invalid Policing Table!"));
        }
        // stod, not stoi: both values are truncated to int below (as they always were
        // when read off the frame), but parsing as a double first accepts "15000",
        // "15000.0" and "1.5e+04" alike. stoi would silently return 1 for the last
        // form, which would look like an inexplicably harsh policing budget rather
        // than a parse error.
        int sigma_bit = static_cast<int>(std::stod(line.substr(indexBegin, indexEnd - indexBegin)));
        indexBegin = indexEnd + 1;

        if ((indexEnd = line.find('}', indexBegin)) == line.npos) {
            throw std::runtime_error(std::string("Invalid Policing Table!"));
        }
        int rho_bit = static_cast<int>(std::stod(line.substr(indexBegin, indexEnd - indexBegin)));

        table[key] = std::make_pair(sigma_bit, rho_bit);
    }

    fileStream.close();
}

void TrafficPolicy::handleMessage(cMessage *msg)
{
    const int phyOverhead_bit = 20 * 8;
    AFDXMessage *afdxMessage = check_and_cast<AFDXMessage*>(msg);
    int VLid = afdxMessage->getVirtualLinkId();
    int frameSize_bit = afdxMessage->getBitLength() + phyOverhead_bit;
    double obtainedCredit_bit = 0;
    bool isTokenSufficient = true;

    bool isRecordCreditEnabled = false;
    NetworkStatistics::SwitchDefinition sw = NetworkStatistics::getInstance()->getSwitchDefinition(this);
    if (NetworkStatistics::getInstance()->isInSwitchAPort(sw)) {
        isRecordCreditEnabled = true;
    }

    // The switch polices against its own configuration table, never against what the
    // frame itself declares (ARINC 664P7 sec. 4.2.2). A VL with no entry in this
    // switch's policing table has no authorized budget: reject it, the same outcome as
    // a token-bucket rejection below, since there is no budget to authorize it against.
    PolicingTableMap_t::iterator policingItr = policingTable.find(VLid);
    if (policingItr == policingTable.end()) {
        if (isRecordCreditEnabled)
            NetworkStatistics::getInstance()->record(afdx::DROPPED_FRAMES_TRAFFIC_POLICY_PER_VL, VLid, sw.index);
        delete msg;
        return;
    }
    int sigma_bit = policingItr->second.first;
    int rho_bit = policingItr->second.second;

    TokenBucketMap_t::iterator itr = tokenBucket.find(VLid);

    double timeDiff_s = 0;
    if (itr == tokenBucket.end()) {
        tokenBucket[VLid] = sigma_bit;
        if (isRecordCreditEnabled)
            afdx::NetworkStatistics::getInstance()->record(CREDIT_PER_VL_PER_SW, afdxMessage->getVirtualLinkId(),
                    tokenBucket[VLid], sw.index);
    }
    else {
        timeDiff_s = simTime().dbl() - lastTime[VLid];
        obtainedCredit_bit = timeDiff_s * rho_bit;

        if (tokenBucket[VLid] + obtainedCredit_bit > sigma_bit) {
            tokenBucket[VLid] = sigma_bit;
        }
        else {
            tokenBucket[VLid] += obtainedCredit_bit;
        }
        if (isRecordCreditEnabled)
            afdx::NetworkStatistics::getInstance()->record(CREDIT_PER_VL_PER_SW, afdxMessage->getVirtualLinkId(),
                    tokenBucket[VLid], sw.index);
    }
    if (tokenBucket[VLid] < frameSize_bit) {
        isTokenSufficient = false;
        int diff = tokenBucket[VLid] - frameSize_bit;
        if (isRecordCreditEnabled) {
            afdx::NetworkStatistics::getInstance()->record(CREDIT_PER_VL_PER_SW, afdxMessage->getVirtualLinkId(), diff,
                    sw.index);
            afdx::NetworkStatistics::getInstance()->record(CREDIT_PER_VL_PER_SW, afdxMessage->getVirtualLinkId(),
                    tokenBucket[VLid], sw.index);
        }
    }
    else {
        tokenBucket[VLid] -= frameSize_bit;
        if (isRecordCreditEnabled)
            afdx::NetworkStatistics::getInstance()->record(CREDIT_PER_VL_PER_SW, afdxMessage->getVirtualLinkId(),
                    tokenBucket[VLid], sw.index);
    }

    if (isTokenSufficient) {
        send(msg, "out");
    }
    else {
        if (isRecordCreditEnabled)
            NetworkStatistics::getInstance()->record(afdx::DROPPED_FRAMES_TRAFFIC_POLICY_PER_VL,
                    afdxMessage->getVirtualLinkId(), sw.index);
        std::cout << "TOKEN_INSUFFICIENT (VL:" << VLid << ") SW:" << sw.index << endl;
        delete msg;
    }
    lastTime[VLid] = simTime().dbl();
}

}
;
// namespace afdx

