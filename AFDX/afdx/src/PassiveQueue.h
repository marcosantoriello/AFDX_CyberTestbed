//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// Modified (C) 2026 Marco Santoriello
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifndef __QUEUEING_PASSIVE_QUEUE_H
#define __QUEUEING_PASSIVE_QUEUE_H

#define AFDX_PQ

#include <map>
#include "QueueingDefs.h"
#include "IPassiveQueue.h"
#include "SelectionStrategies.h"
#include "Job.h"

#ifdef AFDX_PQ
#include "NetworkStatistics.h"
#include "AFDXMessage_m.h"
#endif

#ifdef AFDX_PQ
using namespace queueing;

namespace afdx
#else
namespace queueing
#endif
{
/**
 * A passive queue, designed to co-operate with IServer using method calls.
 */
class PassiveQueue : public cSimpleModule, public IPassiveQueue
{
private:

#ifdef AFDX_PQ
    int swPortIndex;
#endif
    simsignal_t droppedSignal;
    simsignal_t queueLengthSignal;
    simsignal_t queueingTimeSignal;

    bool fifo;
    int capacity;
    cQueue queue;
#ifdef AFDX_PQ
    // When true, dequeues by ascending VLID (static priority) instead of arrival order.
    bool perVLPriority;
    std::map<int, cQueue> queuesByVL;
    // Defers the dispatch decision to a zero-delay self-message so that every frame
    // arriving at the same simulation instant is enqueued into queuesByVL before any
    // of them is actually sent out — otherwise the very first arrival would always be
    // sent immediately (bypassing the per-VL ordering) regardless of its VLID.
    cMessage *dispatchCheckMsg = nullptr;
#endif
    SelectionStrategy *selectionStrategy = nullptr;
    void sendJob(Job *job, int gateIndex);

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;

public:
    virtual ~PassiveQueue();
    // The following methods are called from IServer:
    virtual int length() override;
    virtual void request(int gateIndex) override;
};

}
;
//namespace

#endif

