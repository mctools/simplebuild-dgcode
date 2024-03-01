#!/bin/bash
set -e
set -u
REPOROOT_DIR="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && cd ../ && pwd )"
ruff check "${REPOROOT_DIR}"/doc/source/*.py
ruff check  "${REPOROOT_DIR}"/src/simplebuild_dgcode/*.py
cd "${REPOROOT_DIR}"/src/simplebuild_dgcode/data
#TODO: Reduce this list!!!
IGNORE=E501,E713,F401,E741,E722,E711,E402,F841,F541,E701,F405,E731,E401,E703,F403
find ./pkgs/ -name '*.py' -exec ruff check --ignore $IGNORE {} \;
find ./pkgs_val/ -name '*.py' -exec ruff check {} \;
IGNORE=E501,E713,F401,E741,E722,E711,E402,F841,F541,E701,F405,E731,E401,E703,F403,E702,E721,E712

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
ruff check pkgs_val/SkeletonSP/SkeletonSP/scripts/sim
ruff check pkgs_val/SkeletonSP/SkeletonSP/scripts/scan
ruff check pkgs_val/SkeletonSP/SkeletonSP/scripts/scanana
ruff check pkgs_val/SkeletonSP/SkeletonSP/scripts/simanachain
