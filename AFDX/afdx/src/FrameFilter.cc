//
// Copyright (C) 2013 OpenSim Ltd.
//
// Modified: Copyright (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

/*
 * FrameFilter enforces, at the switch ingress port, the ARINC 664 P7 frame
 * size constraints (frames outside [L_MIN=64B, L_MAX=1518B] are dropped) and
 * the VL-ID/ingress-port admission control mandated by sec. 4.2.1 (a frame
 * whose VL-ID is not allowed on the port it arrived on is dropped).
 */

#include "FrameFilter.h"
#include <fstream>

namespace afdx {

Define_Module(FrameFilter);

void FrameFilter::initialize()
{
    this->ingressVLTableName = par("ingressVLTableName").str();
    this->ingressVLTableName = this->ingressVLTableName.substr(1, this->ingressVLTableName.length() - 2);
    this->getIngressPortToVLMappings(this->allowedVLsByPort, ingressVLTableName.c_str());
    this->swPortIndex = this->getParentModule()->getIndex();
}

void FrameFilter::getIngressPortToVLMappings(PortToVLIdMap_t &mapping, const char *fileName)
{
    std::ifstream fileStream(fileName);

    if (!(fileStream.good())) {
        throw std::runtime_error(std::string("Ingress VL Table Not Found!"));
    }

    std::size_t indexBegin;
    std::size_t indexEnd;

    int key, value;

    for (std::string line; std::getline(fileStream, line);) {
        indexBegin = 0;
        indexEnd = 0;

        if (line.empty() || line.at(0) == '*') {
            continue;
        }

        if ((indexEnd = line.find(':')) == line.npos) {
            throw std::runtime_error(std::string("Invalid Ingress VL Table!"));
        }

        key = std::stoi(line.substr(indexBegin, indexEnd));

        if ((indexEnd = line.find('{')) == line.npos) {
            throw std::runtime_error(std::string("Invalid Ingress VL Table!"));
        }

        indexBegin = indexEnd + 1;

        while (((indexEnd = line.find(',', indexBegin)) != line.npos)) {
            value = std::stoi(line.substr(indexBegin, indexEnd), 0, 16);
            indexBegin = indexEnd + 1;

            mapping.insert(std::pair<int, int>(key, value));
        }

        if ((indexEnd = line.find('}', indexBegin)) != line.npos) {
            value = std::stoi(line.substr(indexBegin, indexEnd), 0, 16);

            mapping.insert(std::pair<int, int>(key, value));
        }
        else {
            throw std::runtime_error(std::string("Invalid Ingress VL Table!"));
        }
    }

    fileStream.close();
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

    int vlId = afdxMessage->getVirtualLinkId();
    auto range = allowedVLsByPort.equal_range(swPortIndex);
    bool allowed = false;
    for (auto itr = range.first; itr != range.second; ++itr) {
        if (itr->second == vlId) {
            allowed = true;
            break;
        }
    }

    if (!allowed) {
        NetworkStatistics::SwitchDefinition sw = NetworkStatistics::getInstance()->getSwitchDefinition(this);
        // Record the drop only on Switch A to avoid double-counting
        // due to the dual-redundant (A/B) switch architecture.
        if (NetworkStatistics::getInstance()->isInSwitchAPort(sw)) {
            NetworkStatistics::getInstance()->createRecorder(DROPPED_FRAMES_VL_PORT_FILTER_PER_VL, vlId);
            NetworkStatistics::getInstance()->record(DROPPED_FRAMES_VL_PORT_FILTER_PER_VL, vlId, 1.0);

            EV << "FrameFilter: dropping frame on VL" << vlId
                            << " - not admitted on ingress port " << swPortIndex << ".\n";

                    delete msg;
                    return;
        }
    }

    // Frame is within valid size bounds and admitted on this ingress port: forward to the next stage
    send(msg, "out");
}

}
;
// namespace afdx

