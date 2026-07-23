//
// Copyright (C) 2013 OpenSim Ltd.
// Modified (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package afdx;

//
// TODO auto-generated module
//
simple RegulatorLogic like IRegulatorLogic
{
    @display("i=block/mac");
    int maxVLIDQueueSize = default(1000);
    gates:
        input in[];
        output out;
}
