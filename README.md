# AFDX-CyberTestbed

A validated, cyber-capable testbed for Avionics Full-Duplex Switched Ethernet (AFDX), extended with the capability to inject attacker behavior at the End System and switch level and to measure its impact on AFDX's deterministic guarantees.

Master's Thesis — Marco Santoriello, University of Salerno.

## About the Project

AFDX (ARINC 664 Part 7) is the switched Ethernet backbone used by Integrated Modular Avionics to carry flight-control commands, sensor measurements, navigation data, and system-status information between avionics subsystems, under strict deterministic guarantees: bounded latency, controlled jitter, and traffic isolation between the many functions sharing the same physical infrastructure.

Existing work on AFDX either proves these guarantees analytically or validates a simulator against the standard, both under nominal, fault-free conditions, or catalogs threats qualitatively without measuring their effect. This project closes that gap: it takes an existing OMNeT++ AFDX simulation framework, validates it against ARINC 664 Part 7 for the functions a set of attack experiments will depend on, and extends it with attacker-controlled components at the End System and switch level. A representative, threat-model-driven set of nine attack experiments, drawn from a STRIDE-based threat catalog and reframed by attacker position, is then carried out on the resulting testbed to measure how far AFDX's deterministic guarantees actually degrade, comparing attacker impact across a compromised End System, a compromised switch, and both compromised together.

## Repository Structure

```
AFDX-CyberTestbed/
├── AFDX/                   # OMNeT++ simulation model
│   ├── afdx/
│   │   ├── src/            # End System, switch, and attack-injection modules
│   │   └── simulations/    # Network topology and run configuration
│   └── queueinglib/        # Supporting OMNeT++ queueing library
├── ANCAT/                  # Avionics Network Configuration & Analysis Tool
│   ├── PreProcessor.py     # Builds simulation *.ini configs from an Excel message set
│   ├── PostProcessor.py    # Turns simulation results into a PDF report
│   ├── SimProcessor.py     # Cross-platform simulation runner
│   ├── ANCAT_run.sh        # One-shot pipeline: pre-process -> simulate -> report
│   └── experiments/        # One subfolder per attack experiment (message set + attack patch)
├── requirements.txt
└── LICENSE
```

`AFDX/afdx/src` contains the End System (`EndSystem`), the switch (`Switch`, `SwitchFabric`, `SwitchPort`, `MAC`), the traffic-shaping and policing logic (`RegulatorLogic`/`IRegulatorLogic`, `TrafficPolicy`), the forwarding and admission-control logic (`VLRouter`, `FrameFilter`), the receiver-side integrity and redundancy-management logic (`IntegrityChecker`, `RedundancyChecker`, `RedundancyController`), and the attacker-controlled regulator variants used by the attack experiments (`MaliciousRegulator_Flooding`, `MaliciousRegulator_PartialViolation`, `MaliciousRegulator_Burst`).

## Threat Model & Attack Experiments

Nine attack experiments are carried out, spanning three of the four attacker positions in the threat model (a network-level attacker with no compromised device is deferred to future work):

| Experiment | Attacker position | Compromised component | What is tested |
|---|---|---|---|
| S2-1 | Compromised End System (source) | Regulator, BAG bypassed entirely | Isolation across End Systems vs. within a shared End System |
| S2-2 | Compromised End System (source) | Regulator, BAG paced at a reduced rate | Same, across increasing violation severity |
| S2-3 | Compromised End System (source) | Regulator, timing shaped into bursts, same volume | Same, from timing alone, with no extra traffic sent |
| S2-4 | Compromised End System (receiving) | Redundancy check disabled | Transparency of the A/B frame redundancy management |
| S3-1 / S3-2 | Compromised switch | Forwarding table | Delivery integrity (S3-1) vs. information disclosure (S3-2) |
| S3-3 / S3-4 | Compromised switch | Policing table (σ/ρ) | Policing removed entirely, as a control (S3-3), vs. tightened against one compliant VL as a selective denial of service (S3-4) |
| S4-1 | Compromised End System **and** switch | Regulator and policing table, same VL | Compositional failure: bounded latency and isolation, for every VL sharing the switch |

## Requirements & Setup

### 1. Install opp_env
```bash
pip3 install opp-env
```

### 2. Set up the AFDX workspace
```bash
mkdir ~/afdx-workspace
cd ~/afdx-workspace
opp_env init
opp_env install afdx-latest
```
This automatically installs both OMNeT++ 6.x and the AFDX model (afdx-20220904)
in `~/afdx-workspace`.

### 3. Install Python dependencies
```bash
pip3 install -r requirements.txt
```

## Running an Experiment

From within `ANCAT/`:
```bash
./ANCAT_run.sh <experiment_name>
```
This looks for `experiments/<experiment_name>.xlsx`, pre-processes it into the simulation's `*.ini` configuration, runs the simulation via `opp_env`, and post-processes the results into `experiments/reports/REP_<experiment_name>.pdf`. If an `experiments/<experiment_name>_attack.ini` file is present alongside the message set, its attack patch (e.g. a malicious regulator, a tampered policing table) is applied on top of the baseline configuration; otherwise the experiment runs in baseline mode.

## License
**ANCAT**: LGPLv3 (original © 2022 Ipek Gökçe, modifications © 2026 Marco Santoriello)

**AFDX OMNeT++ module** (simulation model): LGPLv3
Original © Ipek Gökçe, Emre Atik, https://github.com/badapplexx/AFDX
Based on https://github.com/omnetpp-models/afdx
Modifications © 2026 Marco Santoriello
