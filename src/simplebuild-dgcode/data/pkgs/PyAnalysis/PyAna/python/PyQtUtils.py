#!/usr/bin/env python3

"""Convenience base class for making PyQt5-based GUI apps, supporting
matplotlib widgets and layouts done with Qt designer

"""

__all__ = [ 'MyGuiAppBase','launch_qtapp' ]

# import os
import sys

#better support for 4k displays:
#if not "QT_AUTO_SCREEN_SCALE_FACTOR" in os.environ:
#    os.environ["QT_AUTO_SCREEN_SCALE_FACTOR"] = "1"

try:
    import PyQt5
except ImportError:
    raise SystemExit('Missing PyQt5 module. Perhaps you can install with "python3 -mpip install PyQt5" ?')

from PyQt5 import QtCore, QtWidgets

try:
    import matplotlib
    if hasattr(matplotlib,'compare_versions') and not matplotlib.compare_versions(matplotlib.__version__,'3.1.3'):
        print("WARNING: Your version of matplotlib is too old to work properly with pyqt5. Please upgrade.")
        print('         If using pip, this can perhaps be done by: python3 -mpip install --upgrade matplotlib')
except ImportError:
    pass

import pathlib

class MyGuiAppBase:

    #NB: To add cmdline args, add a create_argparser static method on the
    #derived class like this, and it will receive the arguments in it's
    #constructor in a parameter named "args":
    #
    #  @staticmethod
    #  def create_argparser():
    #      parser = argparse.ArgumentParser()
    #      parser.add_argument(...)
    #      return parser
    #
    #  Can also perform optional validation like this:
    #
    #  @staticmethod
    #  def validate_argparser(parser,args):
    #      if args.bla and not args.foo:
    #          parser.error('Do not specify --bla without a foo.')
    #

    def __init__(self,uicfile=None):
        self.__qmainwindow = QtWidgets.QMainWindow()
        #self.__qmainwindow.setMouseTracking(True) ?

        if uicfile and not pathlib.Path(uicfile).exists():
            import Core.FindData3
            fn=Core.FindData3(uicfile)
            if not fn:
                raise ValueError(f'Could not find file {uicfile}')
            uicfile=fn
        if uicfile:
            import PyQt5.uic
            PyQt5.uic.loadUi(uicfile, self.__qmainwindow)

    def findWidget(self,name,theclass):
        if isinstance(theclass,str):
            theclass = getattr(QtWidgets,theclass)
        return self.__qmainwindow.findChild(theclass,name)

    def mainWindow(self):
        return self.__qmainwindow

    def show(self):
        return self.__qmainwindow.show()

    def connectAction(self,action_name,fct):
        action = self.__qmainwindow.findChild(QtWidgets.QAction, action_name)
        if not action:
            raise ValueError(f'Could not find QAction named {action_name}')

        return action.triggered.connect(fct)

    def connectSignal(self,name,theclass,slotname,fct):
        w = self.findWidget(name,theclass)
        getattr(w,slotname).connect(fct)
        return w

    def getMPLWidget(self,name):
        import PyAna.PyQtMPLWidgets
        w=self.findWidget(name,PyAna.PyQtMPLWidgets.MplWidget)
        if not w:
            raise ValueError(f'Could not find MplWidget named {name} (did you forget to promote in qtdesigner?)')
        return w

    def setStatusBarMsg(self,msg=None,*,permanent=False):
        permlbl = getattr(self,'_SBPermanentLabel',None)
        if permanent and not permlbl :
            permlbl = QtWidgets.QLabel()
            self._SBPermanentLabel = permlbl
            self.__qmainwindow.statusbar.addWidget(permlbl)
        if not msg:
            #clear all!
            self.__qmainwindow.statusBar().showMessage('')
            if permlbl:
                permlbl.setText('')
        else:
            if permanent:
                permlbl.setText(msg)
            else:
                self.__qmainwindow.statusBar().showMessage(msg)

def launch_qtapp(window_factory,*,argv=None,argparser=None):

    #Handle command line arguments carefully. If user passed us an argument
    #parser, we first run than (on the passed argv or else sys.argv). But we
    #allow unused args, and pass those to qt. If any are still unused, it is an
    #error.
    argv = argv or sys.argv

    if not argparser and hasattr(window_factory,'create_argparser'):
        argparser = window_factory.create_argparser()

    if argparser:
        import argparse
        assert isinstance(argparser,argparse.ArgumentParser)#perhaps we don't need to require this strictly...
        parsed_args, unparsed_args = argparser.parse_known_args(argv[1:])
        argv = [argv[0]]+unparsed_args
        if hasattr(window_factory,'validate_argparser'):
            assert hasattr(window_factory,'create_argparser')
            window_factory.validate_argparser(argparser,parsed_args)

    app = QtWidgets.QApplication(argv)
    unused_args = app.arguments()[1:]
    if unused_args:
        msg=f'Unused arguments: {" ".join(unused_args)}'
        if argparser:
            argparser.error(msg)
        else:
            raise SystemExit(f'ERROR {msg}')
    w = window_factory(args=parsed_args) if argparser else window_factory()
    w.show()
    app.exec_()
