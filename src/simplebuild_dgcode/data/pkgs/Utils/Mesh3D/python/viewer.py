from PyAna import plt, np
from Mesh3D import Mesh3D
import PyAna.dyncmap#FIXME: PyAna.misc ?
import matplotlib.widgets as mplwidgets

class Mesh3DViewer:
    def __cellinfos(self,iaxis=None):
        iaxis = iaxis if iaxis is not None else self.__axis
        ii=[0,1,2]
        assert iaxis is not None
        ii.remove(iaxis)
        return [self.__mesh.cellinfo[i] for i in range(3) if i!=iaxis]

    def __extent(self,iaxis):
        assert iaxis is not None
        cellh,cellv = self.__cellinfos(iaxis)
        return cellh[1],cellh[2],cellv[1],cellv[2]

    def __init__(self,mesh):
        mesh = mesh if isinstance(mesh,Mesh3D) else Mesh3D(mesh)
        self.__mesh = mesh
        self.__fig, self.__ax = plt.subplots()
        plt.subplots_adjust(left=0.1, bottom=0.4,right=0.8)
        #self.__ax.set_axis_bgcolor('black')
        self.__blockcount = 0
        self.__block()
        #data image:
        self.__axis = None
        self.__icell0 = [0,0,0]
        self.__icell1 = list(a-1 for a in mesh.data.shape)
        #create image with dummy data and extent (will be updated):
        dummydata=[[0,1],[2,3]]
        self.__obj_img = plt.imshow(dummydata,interpolation='nearest',aspect='auto',
                                    extent=(0.0,1.0,0.0,1.0),
                                    origin='lower',picker=True)
        if True:
            ax_cb = plt.axes([0.82, 0.4, 0.02, 0.5])#todo: better placement?
            self.__obj_colbar = plt.colorbar(self.__obj_img,cax=ax_cb)#,ticks=[])
        self.__fig.canvas.mpl_connect('pick_event', self.__pick_image)

        plt.suptitle('%s\nUser comments: "%s"'%(mesh.name,mesh.comments) if mesh.comments else mesh.name)

        self.__ax_1d = plt.axes([0.1, 0.1, 0.7, 0.2])
        self.__poly1d = None

        ax_radio = plt.axes([0.82, 0.1, 0.1, 0.2],facecolor=(1,1,1,0.0))
        axischooser = mplwidgets.RadioButtons(ax_radio, ('  XY', '  YZ', '  XZ'), active=0)
        axischooser.on_clicked(self.__update_axischooser)
        #self.__axischooser = axischooser # keep alive
        #self.__ax_radio = ax_radio # keep alive
        self.__lastupdate=(None,None,None)
        self.__update_axischooser('XY')
        self.__unblock()
        plt.show()

    def __pick_image(self,event):
        evt=event.mouseevent
        if not evt or event.artist!=self.__obj_img:
            return
        if not self.__obj_img.contains(evt):
            return
        x0,x1,y0,y1 = self.__extent(self.__axis)
        cellh,cellv = self.__cellinfos()
        nh,h0,h1 = cellh
        nv,v0,v1 = cellv
        rh = (evt.xdata-h0)/(h1-h0)
        rv = (evt.ydata-v0)/(v1-v0)
        ih=int(rh*nh)
        iv=int(rv*nv)
        if ih<0 or ih >= nh or iv < 0 or iv >= nv:
            return
        cont = self.__extract_data(mask_zeroes=False).T[iv][ih]
        print("Mesh3DViewer: content in selected cell (%i,%i) is %g"%(ih,iv,cont))

    def __block(self):
        self.__blockcount += 1

    def __unblock(self):
        self.__blockcount -= 1
        if self.__blockcount == 0:
            self.__update()

    def __update_axischooser(self,choice):
        choice = choice.strip()
        ia = {'XY':2,'YZ':0,'XZ':1}[choice]
        if self.__axis is None or self.__axis != ia:
            self.__block()
            self.__obj_img.set_extent(self.__extent(ia))
            self.__axis = ia
            cu=self.__mesh.cellunits
            self.__ax.set_xlabel('%s [%s]'%(choice[0],cu) if cu else choice[0])
            self.__ax.set_ylabel('%s [%s]'%(choice[1],cu) if cu else choice[1])
            n1d,x0,x1=self.__mesh.cellinfo[ia]
            x=np.linspace(x0,x1,n1d,endpoint=False)
            y=self.__extract_data1d()
            self.__spanselect=None
            self.__poly1d = None
            self.__ax_1d.clear()

            self.__ax_1d.bar(x,y,(x1-x0)/n1d,0.0,linewidth=0.0)
            self.__ax_1d.set_xlabel('%s [%s]'%('XYZ'[ia],cu) if cu else 'XYZ'[ia])
            self.__ax_1d.set_xlim(x0, x1)
            self.__ax_1d.grid()
            y0,y1=min(0.0,y.min()), max(0.0,y.max())
            if y0==y1:
                y1=1.0
            y0 *= 1.1
            y1 *= 1.1
            self.__ax_1d.set_ylim(y0,y1)
            self.__spanselect = mplwidgets.SpanSelector(self.__ax_1d, self.__spanselected,
                                                        'horizontal', useblit=True,
                                                        props=dict(alpha=0.5, facecolor='red'))
            self.__unblock()
            self.__update(None)

    def __spanselected(self,s0,s1):
        assert self.__axis is not None
        n1d,x0,x1=self.__mesh.cellinfo[self.__axis]
        import math
        i0 = max(0,int(math.floor((s0-x0)*n1d/(x1-x0))))
        i1 = min(n1d-1,int(math.floor((s1-x0)*n1d/(x1-x0))))
        if s0==s1:
            i0,i1 = 0,n1d-1#double-click selects all
        self.__block()
        self.__icell0[self.__axis] = i0
        self.__icell1[self.__axis] = i1
        self.__unblock()
        self.__update()

    def __update(self,*dummy):
        if self.__blockcount > 0:
            return
        assert self.__axis is not None
        ia = self.__axis
        c0 = self.__icell0[ia]
        c1 = self.__icell1[ia]
        lu=(ia,c0,c1)
        if self.__lastupdate==lu:
            return
        self.__lastupdate = lu
        self.__icell0[ia] = c0
        self.__icell1[ia] = c1
        n1d,x0,x1=self.__mesh.cellinfo[self.__axis]
        if self.__poly1d:
            self.__poly1d.remove()
        self.__poly1d = self.__ax_1d.axvspan( x0 + c0*(x1-x0)/n1d,
                                              x0 + (c1+1)*(x1-x0)/n1d,
                                              facecolor='g', alpha=0.5, zorder=99 )
        data = self.__extract_data()
        PyAna.dyncmap.dynamic_2d_colormap(data,self.__obj_img)
        self.__obj_img.set_data(data.T)
        self.__obj_img.changed()
        self.__fig.canvas.draw_idle()

    def __extract_data(self,mask_zeroes=True):
        assert self.__axis is not None
        ia = self.__axis
        c0 = self.__icell0[ia]
        c1 = self.__icell1[ia]
        assert c1>=c0
        data=self.__mesh.data
        if mask_zeroes:
            data = np.ma.masked_where(data == 0.0, data)
        n = data.shape[ia]
        s = slice(c0,c1+1) if c1+1<n else (slice(c0,None) if c0 else slice(0,n))
        if ia==0:
            d=np.sum(data[s,:,:], axis=ia)
        elif ia==1:
            d=np.sum(data[:,s,:], axis=ia)
        elif ia==2:
            d=np.sum(data[:,:,s], axis=ia)
        else:
            assert False
        return d

    def __extract_data1d(self):
        assert self.__axis is not None
        return np.sum(self.__mesh.data,tuple(i for i in range(3) if i!=self.__axis))

def experimental_volume_rendering(mesh):
    mesh = mesh if isinstance(mesh,Mesh3D) else Mesh3D(mesh)
    #from numpy import array, random, linspace, pi, ravel, cos, sin, empty
    ok=False
    try:
        from mayavi import mlab
        from tvtk.api import tvtk
        ok=True
    except ImportError:
        print()
        print("Error: Could not import required modules for experimental volume rendering.")
        print()
        print("       Perhaps you need to install Mayavi (see mayavi.sourceforge.net/)?")
        print("       On Fedora this is done with: sudo yum install Mayavi")
        print()
    if not ok:
        return
    def image_data(msh):
        c=msh.cellinfo
        a,b,c=c[0][2]-c[0][1], c[1][2]-c[1][1], c[2][2]-c[2][1]
        d=max(a,b,c)
        i = tvtk.ImageData(spacing=(a/d, b/d, c/d), origin=(0,0,0))
        i.point_data.scalars = msh.data.ravel('F')#'F' correct?
        i.point_data.scalars.name = msh.name
        i.dimensions = msh.data.shape
        return i
    mlab.figure(bgcolor=(0,0,0))
    #vol=mlab.pipeline.volume(image_data(mesh),vmin=0.01,vmax=0.999)
    mlab.show()

