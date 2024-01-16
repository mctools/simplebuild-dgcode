#ifndef Mesh_MeshFiller_hh
#define Mesh_MeshFiller_hh

#include <vector>
#include <limits>
#include <cassert>

// Utility class for collecting values in an N dimensional mesh. The mesh can be
// filled either at a given point, or (the raison d'etre for this class), by
// intersecting the mesh with a line-segment.
//
// Note that neither fill(..) or contentAt(..) are thread-safe, and usage of
// those should be protected properly in user-code in multi-threaded
// environments.

namespace Mesh {
  template <int NDIM, class TStorage = std::vector<double> >
  class MeshFiller {
  public:

    typedef TStorage storage_type;
    static const int ndim = NDIM;

    MeshFiller( const long(&ncells)[NDIM],
                const double(&cell_lower)[NDIM],
                const double(&cell_upper)[NDIM] );
    ~MeshFiller();

    //Add deposit to cell at specified position. Does nothing if position is
    //outside the ranges given in the constructor:
    void fill(double dep, const double(&position)[NDIM]);

    //Add deposit to any cells intersected by the given line-segment from pos0 to
    //pos1. Each cell gets a contribution dep*l/L where l is the length of the
    //intersection between the line segment and the cell, and L is the total
    //length of the line segment:
    void fill(double dep, const double(&pos0)[NDIM], const double(&pos1)[NDIM]);

    //Access contents, dump to stdout or write to file:
    double cellContent(long cell1d) const;
    double cellContent(const long(&cell)[NDIM]) const;
    long nCells() const { return m_ncells; }
    long nCells(int idim) const { assert(idim<NDIM); return m_axis[idim].n; }
    double cellLower(int idim) const { assert(idim<NDIM); return m_axis[idim].a; }
    double cellUpper(int idim) const { assert(idim<NDIM); return m_axis[idim].b; }
    double cellSpan(int idim) const { return cellUpper(idim)-cellLower(idim); }
    double cellWidth(int idim) const { return cellSpan(idim) / nCells(idim); }

    long cellIdCollapse(const long(&cell)[NDIM]) const;//Collapse N-dimensional cell id to 1D
    void cellIdExpand(long cell1d, long(&cell)[NDIM]) const;//Expand 1d cell id to N-dimensions
    void dump() const;

    //Access contents via point or line-segment (for the latter, contents of
    //crossed cells will be summed with weights l/L (analogously to the fill
    //method above).
    double contentAt(const double(&position)[NDIM]) const;
    double contentAt(const double(&pos0)[NDIM], const double(&pos1)[NDIM]) const;

    //intersect mesh and get pairs of (1d) cell indices and associated fraction
    //(l/L) of line-segment crossing the cell:
    void getIntersections( const double(&pos0)[NDIM], const double(&pos1)[NDIM],
                           std::vector<std::pair<long,double>>& res ) const;

    //Raw access to internal data structure:
    TStorage& data() { return m_data; }
    const TStorage& data() const { return m_data; }

    //clear contents and reshape:
    void reinit( const long(&ncells)[NDIM],
                 const double(&cell_lower)[NDIM],
                 const double(&cell_upper)[NDIM] );

    //Construct undefined object for persistency (must call "reinit" before usage):
    MeshFiller();
    bool isInvalid() const { return m_ncells==0; }

    //Test for compatiblity (identical cell layout, but not necessarily contents):
    bool compatible(MeshFiller& other) const;

  private:
    MeshFiller( const MeshFiller & );
    MeshFiller & operator= ( const MeshFiller & );
    struct Axis {
      long n; double a; double b; double invwidth;
      Axis() : n(0), a(0), b(0), invwidth(0) {}
      ~Axis(){}
      void set(long nn, double aa, double bb);
    };
    void toStdCoords(const double(&pt)[NDIM], double(&out)[NDIM]) const;
    long stdCoordToCell(const double(&pt)[NDIM]) const;
    TStorage m_data;
    long m_ncells;
    Axis m_axis[NDIM];
    long m_cellfactor[NDIM];
    mutable std::vector<std::pair<long,double>> m_intersection_cache;
    static long calcTotNCells(const long(&ncells)[NDIM]);
  };

}

#include "MeshFiller.icc"

#endif
