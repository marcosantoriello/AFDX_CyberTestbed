//
// Copyright (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package afdx;

//
// Malicious regulator for partial BAG violation.
// Paces frames with the same logic as RegulatorLogic, but against a reduced
// BAG: BAG_effective = BAG * bagReductionFactor. The ES therefore releases
// frames faster than the VL contract it declares, without bypassing pacing
// altogether (see MaliciousRegulator_Flooding for the full-bypass case).
//
simple MaliciousRegulator_PartialViolation like IRegulatorLogic
{
    @display("i=block/mac");
    int maxVLIDQueueSize = default(1000);
    double bagReductionFactor = default(1.0); // in (0.0, 1.0]: 1.0 behaves like the honest regulator
    gates:
        input in[];
        output out;
}
