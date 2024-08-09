#Here are the ones for geant4 11.2.2 (with .0 on the data versions where needed
#for safety):
cached_noarch_pkgs_def = """
geant4-data-abla =3.3.0
geant4-data-emlow =8.5.0
geant4-data-ensdfstate =2.3.0
geant4-data-incl =1.2.0
geant4-data-ndl =4.7.1
geant4-data-particlexs =4.0.0
geant4-data-photonevaporation =5.7.0
geant4-data-pii =1.3.0
geant4-data-radioactivedecay =5.6.0
geant4-data-realsurface =2.2.0
geant4-data-saiddata =2.0.0
"""

cached_noarch_pkgs = [ l.strip().split('=') for l in cached_noarch_pkgs_def.splitlines() if l.strip() ]

def produce_yml():
    print("""name: cached_noarch_pkgs
channels:
  - nodefaults
  - conda-forge
dependencies:""")
    for pn,pv in cached_noarch_pkgs:
        print(f'  - {pn}={pv}')

def verify_active_env_use_correct_versions( allow_no_usage = False ):
    import json
    import subprocess
    cmd = ['conda','list','--json',
           '(%s)'%('|'.join( name for name,version in cached_noarch_pkgs ))
           ]
    #import shlex
    #print( ' '.join(shlex.quote(e) for e in cmd))
    cl = subprocess.run(cmd,capture_output=True,check=True)
    conda_list = json.loads(cl.stdout)
    for pkg in conda_list:
        pn, pv = pkg['name'], pkg['version']
        l = [ expected_version for name, expected_version in cached_noarch_pkgs if name == pn ]
        if len(l) != 1:
            raise SystemExit(f'Unexpected error: package {pn} was found in the current'
                             ' conda env but does not appear exactly once in the definition. This should never happen.')
        expected_version = l[0]
        if expected_version != pv:
            raise SystemExit(f'Error: Active conda env has unexpected package version of package {pn} (found {pv} but'
                             f' expected {expected_version}). Time to update the hardwired cache definition?')
        print(f'  --> Verified usage of {pn}={pv}')

    if not allow_no_usage:
        for pn,pv in cached_noarch_pkgs:
            if not any( pkg['name']==pn for pkg in conda_list ):
                raise SystemExit(f'Error: Active conda env did not use cached package {pn}. Time to update the hardwired cache definition?')
    print(f'  --> Verified all usages.')

if __name__=='__main__':
    import sys
    if '--yml' in sys.argv[1:]:
        produce_yml()
    elif '--verify-usage' in sys.argv[1:]:
        verify_active_env_use_correct_versions()
    else:
        raise SystemExit('Please specify --yml or --verify-usage')
