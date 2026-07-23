//
// Copyright (C) 2026 Marco Santoriello
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package afdx;

//
// Interface for regulator logic modules in an AFDX End System.
// All implementations must expose the same gate signature as RegulatorLogic.
//

moduleinterface IRegulatorLogic
{
    gates:
        input in[];
        output out;
}