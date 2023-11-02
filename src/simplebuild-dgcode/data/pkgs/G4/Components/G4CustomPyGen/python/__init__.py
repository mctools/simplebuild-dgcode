from __future__ import print_function
__all__=['GenBase']
import sys,os

from G4CustomPyGen._GenBaseCpp import _GenBaseCpp
class GenBase(_GenBaseCpp):
    def __init__(self):

        #Calling _GenBaseCpp __init__, so pybind11 exports the C++ class correctly:
        super(GenBase,self).__init__()

        #Make sure we generate a proper name based on class name and file where it is implemented:
        self.setName(self.__construct_name())

        #if user implemented declare_parameters, call it:
        dp = getattr(self,'declare_parameters',None)
        if dp:
            dp()

        #Check if users added custom unlimited() on the python side:
        f = getattr(self,'unlimited',None)#optional
        if f:
            val = f()
            if not isinstance(val,bool):
                raise ValueError('unlimited must return True or False (and the value must not change)')
            self._py_set_unlimited(val)

        #register python user methods to C++ side, so callbacks can come at the appropriate time:
        f = getattr(self,'validate_parameters',None)#optional
        if f:
            self._regpyfct_validatePars(f)

        f = getattr(self,'init_generator',None)#optional
        if f:
            self._regpyfct_initGen(f)

        f = getattr(self,'generate_event',None)#required
        if not f:
            raise NotImplementedError("Must provide implementation of method generate_event(self,gun)");
        self._regpyfct_genEvt(f);

    def __construct_name(self):
        m = self.__module__
        if m == '__main__':
            m = os.path.basename(sys.argv[0])
        return os.path.join(m,self.__class__.__name__)

    def create_hist_sampler(self,histstr):
        def badusage():
            print('ERROR: Bad create_hist_sampler arguments:')
            print('ERROR:    Please supply a single string of the form "FILENAME:HISTKEY:UNIT"')
            print('ERROR:    FILENAME should either be an absolute path to an .shist file,')
            print('ERROR:    or of the form "<pkgname>/<filename>.')
            print('ERROR:    HISTKEY should be the key of the chosen 1D histogram in the file')
            print('ERROR:    UNIT is optional and should be a number or a name from Units.units')
            raise ValueError('create_hist_sampler: bad arguments')

        parts=histstr.split(':')
        if len(parts) not in (2,3):
            badusage()
        filename=parts[0]
        if len(filename.split('/'))==2:
            import Core.FindData
            pkg,fn=filename.split('/',1)
            filename=Core.FindData(pkg,fn)
            if not os.path.exists(filename):
                raise ValueError('No data file named "%s" found in package "%s"'%(fn,pkg))
        else:
            if not filename or not os.path.exists(filename):
                raise ValueError('File not found: "%s"'%filename)
        if not filename.endswith('.shist'):
            raise ValueError('File not a SimpleHist collection (.shist): "%s"'%filename)
        histkey=parts[1]
        import SimpleHists
        hc=SimpleHists.HistCollection(filename)
        if not hc.hasKey(histkey):
            raise ValueError('Histogram with key "%s" not found in file "%s"'%(histkey,filename))
        h=hc.hist(histkey)
        unit = 1.0
        if len(parts)==3:
            try:
                unit = float(parts[2])
            except ValueError:
                unit = None
            if unit == None:
                import Units
                if hasattr(Units.units,parts[2]):
                    unit = getattr(Units.units,parts[2])
                    if not isinstance(unit,float):
                        unit = None
                if unit == None:
                    raise ValueError('Unknown unit: "%s"'%(parts[2]))
        assert isinstance(unit,float)
        import SimpleHistsUtils.Sampler
        s=SimpleHistsUtils.Sampler.Sampler(h,unit)
        def sampler():
            return s.sample(self.rand())
        return sampler
