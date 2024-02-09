#!/usr/bin/env bash
set -eu
set +x
export SIMPLEBUILD_CFG=
unset SIMPLEBUILD_CFG

echo "CMDPROMPT>mkdir sbverify"
mkdir sbverify

echo "CMDPROMPT>cd sbverify"
cd sbverify

echo "CMDPROMPT>sb --init dgcode_val"
sb --init dgcode_val

#In case we are not run from conda, prevent warning in output:
eval "$(sb --env-setup)"

echo "CMDPROMPT>sb --tests"
sb --tests

echo "CMDPROMPT>sb_core_extdeps --require ZLib NCrystal Numpy matplotlib Geant4 OSG && echo AllOK"
sb_core_extdeps --require ZLib NCrystal Numpy matplotlib Geant4 OSG && echo AllOK
