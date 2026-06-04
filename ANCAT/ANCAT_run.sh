#!/usr/bin/env bash

# Usage: ./ANCAT_run.sh <experiment_name>
# Example: ./ANCAT_run.sh exp2-2
#          ./ANCAT_run.sh my_custom_experiment
#
# Looks for:  experiments/<experiment_name>.xlsx
# Saves to:   reports/REP_<experiment_name>.pdf
# Attack mode (optional): place an attack patch file alongside the experiment file.
# Example: ./ANCAT_run.sh exp-T6a
#          ./ANCAT_run.sh exp-T6a  (with experiments/exp-T6a_attack.ini present)
#
# Looks for:  experiments/<experiment_name>_attack.ini  (optional)
# If found:   attack patch is appended to simulation arguments, overriding
#             baseline parameters (e.g. attacker ES regulator type, timing).
# If absent:  simulation runs in baseline mode with no changes.

OMNET_PATH="$HOME/afdx-workspace/omnetpp-6.0"
AFDX_PATH="$HOME/afdx-workspace/afdx-20220904"
ANCAT_PATH="$(cd "$(dirname "$0")" && pwd)"

EXP_NAME="${1:?Usage: ./ANCAT_run.sh <experiment_name>  (es: ./ANCAT_run.sh exp2-2)}"

XLSX_FILE="$ANCAT_PATH/experiments/${EXP_NAME}.xlsx"
REPORT_NAME="REP_${EXP_NAME}"
REPORT_PATH="$ANCAT_PATH/experiments/reports/"
ATTACK_INI="$ANCAT_PATH/experiments/${EXP_NAME}_attack.ini"

SIM_DIR="$AFDX_PATH/afdx/simulations"
SRC_DIR="$AFDX_PATH/afdx/src"
QLIB_DIR="$AFDX_PATH/queueinglib"
SIM_EXE="$SRC_DIR/afdx"
OPP_ENV_WORKSPACE="$HOME/afdx-workspace"

if [ ! -f "$XLSX_FILE" ]; then
    echo "Error: experiment file not found: $XLSX_FILE"
    exit 1
fi

mkdir -p "$REPORT_PATH"

echo ">> Running experiment: ${EXP_NAME}.xlsx"
echo ">> Report will be saved as: ${REPORT_NAME}.pdf"

# PreProcessor
python3 "$ANCAT_PATH/PreProcessor.py" -iPath "$XLSX_FILE" -oPath "$SIM_DIR/"

if [ -f "$ATTACK_INI" ]; then
    echo ">> Attack patch found: ${EXP_NAME}_attack.ini"
    SIM_ARGS="AutoNetwork.ini \"$ATTACK_INI\""
else
    SIM_ARGS="AutoNetwork.ini"
fi

# Simulation
SIM_SCRIPT=$(mktemp)
cat > "$SIM_SCRIPT" << SIMEOF
export DYLD_LIBRARY_PATH="$QLIB_DIR:\${DYLD_LIBRARY_PATH:-}"
cd "$SIM_DIR"
"$SIM_EXE" -m -u Cmdenv -c General -n "$SRC_DIR:$QLIB_DIR" $SIM_ARGS
exit
SIMEOF

opp_env shell -w "$OPP_ENV_WORKSPACE" --no-chdir < "$SIM_SCRIPT"
rm "$SIM_SCRIPT"

# PostProcessor
python3 "$ANCAT_PATH/PostProcessor.py" -iPath "$SIM_DIR/results/" -oPath "$REPORT_PATH" -oFile "$REPORT_NAME"

# Cleanup
rm -rf "$ANCAT_PATH"/ANCAT_figures_*/
