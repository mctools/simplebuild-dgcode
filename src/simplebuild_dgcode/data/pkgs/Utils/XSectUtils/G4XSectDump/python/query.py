import Core.System as Sys
import re
import pathlib
import os

def rung4_extract_xs(element_or_isotope,physlist='QGSP_BIC_HP_EMZ',particle='neutron',verbose=False,
                     parse_results_fct = None):
    if parse_results_fct is None:
        import XSectParse.ParseXSectFile
        parse_results_fct = XSectParse.ParseXSectFile.parse
    parts=[e.strip() for e in re.split(r'(\d+)', element_or_isotope) if e.strip() ]#split on digits
    if len(parts)==1:
        parts+=['0']
    if not len(parts)==2 or not parts[0].isalpha() or not parts[1].isdigit():
        raise RuntimeError(f'Invalid element specification: "{element_or_isotope}"'
                           +' (examples of valid ones are "H", "Fe", Fe56", "B10", ...)')
    element,A = parts[0],int(parts[1])
    matname = f'gasmix::{element}{A or ""}'
    g4cmd=['sb_g4xsectdump_query',f'-p{particle}',f'-l{physlist}',f'-m{matname}','--noshow']
    g4cmd = Sys.quote_cmd(g4cmd)
    with Sys.work_in_tmpdir():
        print("Working in temporary directory: %s"%os.getcwd())
        print("Launching: %s"%g4cmd)
        Sys.system_throw(g4cmd,catch_output=not verbose)
        print("Parsing output.")

        expected_outfilename = f'xsects_discreteprocs_{particle}__{matname}__{physlist}.txt'
        outfile = pathlib.Path(expected_outfilename)
        if not outfile.exists():
            raise RuntimeError(f'Did not find expected output file: {expected_outfilename}')
        return parse_results_fct(outfile.resolve(strict=True))

def rung4_extract_collapsed_neutronxs(element_or_isotope,**kwargs):
    import XSectParse.ParseXSectFile
    return rung4_extract_xs( element_or_isotope,
                             parse_results_fct = XSectParse.ParseXSectFile.extract_neutron_xs,
                             **kwargs )
