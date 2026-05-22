# AFDX-CyberTestbed

Attack injection and analysis testbed for AFDX avionics networks.  
Measures impact on bounded latency, jitter, and traffic isolation.


Master's Thesis - Marco Santoriello, University of Salerno.

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


## License
**ANCAT**: LGPLv3 (original © 2022 Ipek Gökçe, modifications © 2026 Marco Santoriello)

**AFDX OMNeT++ module** (simulation model): LGPLv3
Original © Ipek Gökçe, Emre Atik, https://github.com/badapplexx/AFDX
Based on https://github.com/omnetpp-models/afdx
Modifications © 2026 Marco Santoriello