#!/bin/bash
set -e
set -u
REPOROOT_DIR="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && cd ../ && pwd )"
ruff "${REPOROOT_DIR}"/doc/source/*.py
ruff "${REPOROOT_DIR}"/src/simplebuild_dgcode/*.py
cd "${REPOROOT_DIR}"/src/simplebuild_dgcode/data
#TODO: Reduce this list!!!
IGNORE=E501,E713,F401,E741,E722,E711,E402,F841,F541,E701,F405,E731,E401,E703,F403
find ./pkgs/ -name '*.py' -exec ruff --ignore $IGNORE {} \;
find ./pkgs_val/ -name '*.py' -exec ruff {} \;
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
        ruff --ignore $IGNORE "$fn"
    done
done

#Skeleton should be kept pristine:
ruff pkgs_val/SkeletonSP/SkeletonSP/scripts/sim
ruff pkgs_val/SkeletonSP/SkeletonSP/scripts/scan
ruff pkgs_val/SkeletonSP/SkeletonSP/scripts/scanana
ruff pkgs_val/SkeletonSP/SkeletonSP/scripts/simanachain
