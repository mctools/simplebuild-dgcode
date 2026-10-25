#include "sbldpy/sbldpy.hh"
#include "Mesh/Mesh.hh"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

//Compiled part of the Mesh3D Python module (see python/__init__.py).

namespace Mesh3D_py {

  class MeshFile {
  public:
    MeshFile( const std::string& filename )
    {
      Mesh::Mesh<3> mesh( filename );
      m_name = mesh.name();
      m_comments = mesh.comments();
      m_cellunits = mesh.cellunits();
      for ( auto& e : mesh.statMap() ) {
        m_statkeys += ( m_statkeys.empty() ? "" : "\n" ) + e.first;
        m_statvals.push_back( e.second );
      }
      for ( int i = 0; i < 3; ++i )
        m_cells[i] = std::make_tuple( std::int64_t( mesh.filler().nCells( i ) ),
                                      mesh.filler().cellLower( i ), mesh.filler().cellUpper( i ) );
      auto& datavect = mesh.filler().data();
      m_data.assign( datavect.size(), 0.0 );
      std::size_t iblock = 0;
      const std::size_t nblocks = datavect.blockCount();
      while ( ( iblock = datavect.nextAllocBlock( iblock ) ) < nblocks ) {
        auto blockdata = datavect.block( iblock );
        std::memcpy( &m_data[datavect.block_size * iblock], &( (*blockdata)[0] ), sizeof(double) * blockdata->size() );
        ++iblock;
      }
    }
    const std::string& name() const { return m_name; }
    const std::string& comments() const { return m_comments; }
    const std::string& cellunits() const { return m_cellunits; }
    const std::string& statkeys() const { return m_statkeys; }
    const std::vector<double>& statvals() const { return m_statvals; }
    const std::tuple<std::int64_t,double,double>& cells( std::int64_t i ) const
    {
      if ( i < 0 || i > 2 )
        throw std::out_of_range("axis index out of range");
      return m_cells[i];
    }
    const std::vector<double>& data() const { return m_data; }
  private:
    std::string m_name, m_comments, m_cellunits, m_statkeys;
    std::vector<double> m_statvals;
    std::tuple<std::int64_t,double,double> m_cells[3];
    std::vector<double> m_data;
  };

  //Input files separated by newlines:
  void mergeFiles( const std::string& output_file, const std::string& input_files )
  {
    std::vector<std::string> files;
    std::size_t pos = 0;
    while ( pos <= input_files.size() ) {
      auto next = input_files.find( '\n', pos );
      if ( next == std::string::npos )
        next = input_files.size();
      files.push_back( input_files.substr( pos, next - pos ) );
      pos = next + 1;
    }
    if ( files.size() < 2 )
      throw std::invalid_argument("List of files to merge must be at least length 2");
    Mesh::Mesh<3> mesh( files.front() );
    for ( std::size_t i = 1; i < files.size(); ++i )
      mesh.merge( files[i] );
    mesh.saveToFile( output_file );
  }

}

SBLDPY_MODULE( _impl )
{
  using Mesh3D_py::MeshFile;
  m.add_class<MeshFile>( "MeshFile" )
    .def_init<std::string>( { "filename" } )
    .def( "name", []( const MeshFile& f ) { return f.name(); } )
    .def( "comments", []( const MeshFile& f ) { return f.comments(); } )
    .def( "cellunits", []( const MeshFile& f ) { return f.cellunits(); } )
    .def( "statkeys", []( const MeshFile& f ) { return f.statkeys(); } )
    .def( "nstats", []( const MeshFile& f ) -> std::int64_t { return f.statvals().size(); } )
    .def( "statval", []( const MeshFile& f, std::int64_t i )
    {
      if ( i < 0 || static_cast<std::size_t>( i ) >= f.statvals().size() )
        throw std::out_of_range("index out of range");
      return f.statvals()[static_cast<std::size_t>( i )];
    } )
    .def( "cells", []( const MeshFile& f, std::int64_t i ) { return f.cells( i ); } )
    .def( "data", []( const MeshFile& f ) { return sbldpy::NumpyCopy<double>( f.data() ); } );
  m.def( "merge_files", &Mesh3D_py::mergeFiles, { "output_file", "input_files" } );
}
