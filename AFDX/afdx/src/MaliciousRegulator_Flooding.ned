//
// Copyright (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package afdx;

//
// Malicious regulator for VL Flooding
// Forwards every incoming frame immediately, bypassing BAG enforcement.
//
simple MaliciousRegulator_Flooding like IRegulatorLogic
{
    @display("i=block/mac");
    gates:
        input in[];
        output out;
}