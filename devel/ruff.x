#!/bin/bash
set -e
set -u
REPOROOT_DIR="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && cd ../ && pwd )"
#Rules (mostly enabled by default in newer ruff versions) which we for now
#ignore everywhere: I001 (import sorting, order matters in many of our
#scripts), UP031 (printf-style formatting), C4 (comprehension style), SIM115
#(open without context manager), N999 (invalid module name):
NEWRULES_IGNORE=I001,UP031,C4,SIM115,N999
ruff check --ignore $NEWRULES_IGNORE "${REPOROOT_DIR}"/doc/source/*.py
ruff check --ignore $NEWRULES_IGNORE "${REPOROOT_DIR}"/src/simplebuild_dgcode/*.py
cd "${REPOROOT_DIR}"/src/simplebuild_dgcode/data
#TODO: Reduce this list!!!
IGNORE=E501,E713,F401,E741,E722,E711,E402,F841,F541,E701,F405,E731,E401,E703,F403,$NEWRULES_IGNORE
find ./pkgs/ -name '*.py' -exec ruff check --ignore $IGNORE {} +
find ./pkgs_val/ -name '*.py' -exec ruff check --ignore $NEWRULES_IGNORE {} +
IGNORE=E501,E713,F401,E741,E722,E711,E402,F841,F541,E701,F405,E731,E401,E703,F403,E702,E721,E712,$NEWRULES_IGNORE

for pkginfo in $(find . -name pkg.info); do
    sd="$(dirname "${pkginfo}")/scripts"
    if [ ! -d "${sd}" ]; then
        continue
    fi
    for fn in "${sd}/"*; do

        if [[ "$fn" == *'~'* ]];then
            #Ignore backup files
            echo "Ignoring $fn"
            continue
        fi

        ISNOTPY=0
        ( head -1 "${fn}"|grep -q 'usr/bin/env python' ) || ISNOTPY=1
        if [ $ISNOTPY -eq 1 ]; then
            continue
        fi
        ruff check --ignore $IGNORE "$fn"
    done
done

#Skeleton should be kept pristine:
ruff check --ignore $NEWRULES_IGNORE pkgs_val/SkeletonSP/SkeletonSP/scripts/sim
ruff check --ignore $NEWRULES_IGNORE pkgs_val/SkeletonSP/SkeletonSP/scripts/scan
ruff check --ignore $NEWRULES_IGNORE pkgs_val/SkeletonSP/SkeletonSP/scripts/scanana
ruff check --ignore $NEWRULES_IGNORE pkgs_val/SkeletonSP/SkeletonSP/scripts/simanachain
