//found on http://www.3drealtimesimulation.com/osg/osg_faq_1.htm#f48

#include "WireframeUtils.hh"
#include <osg/PolygonMode>

void G4OSG::forcedWireFrameModeOn( osg::Node *srcNode ) {

  // ---------------------------------------------------------------------------
  //  Public  Function
  //
  //   Force the Node and its children to be wireframe, overrides parents state
  //
  // ---------------------------------------------------------------------------

  //
  // Quick sanity check , we need a node
  //

  if( srcNode == NULL )
    return;

  // --------------------------------------------------------------------------------
  //   Grab the state set of the node, this  will a StateSet if one does not exist
  // --------------------------------------------------------------------------------

  osg::StateSet *state = srcNode->getOrCreateStateSet();

  // ----------------------------------------------------------------------------------------------
  //   We need to retireve the Poylgon mode of the  state set, and create one if does not have one
  // ----------------------------------------------------------------------------------------------

  osg::PolygonMode *polyModeObj;

  polyModeObj = dynamic_cast< osg::PolygonMode* >( state->getAttribute( osg::StateAttribute::POLYGONMODE ));

  if ( !polyModeObj ) {
    polyModeObj = new osg::PolygonMode;
    state->setAttribute( polyModeObj );
  }

  // --------------------------------------------
  //  Now we can set the state to WIREFRAME
  // --------------------------------------------

  polyModeObj->setMode(  osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE );

}  // forceWireFrameModeOn

void G4OSG::forcedWireFrameModeOff( osg::Node *srcNode ){

  // ---------------------------------------------------------------------------
  //  Public  Function
  //
  //   Force the Node and its children to be drawn filled, overrides parents state
  //
  // ---------------------------------------------------------------------------

  //
  // Quick sanity check , we need a node
  //

  if( srcNode == NULL )
    return;

  // --------------------------------------------------------------------------------
  //   Grab the state set of the node, this  will a StateSet if one does not exist
  // --------------------------------------------------------------------------------

  osg::StateSet *state = srcNode->getOrCreateStateSet();

  // ----------------------------------------------------------------------------------------------
  //   We need to retireve the Poylgon mode of the  state set, and create one if does not have one
  // ----------------------------------------------------------------------------------------------

  osg::PolygonMode *polyModeObj;

  polyModeObj = dynamic_cast< osg::PolygonMode* > ( state->getAttribute( osg::StateAttribute::POLYGONMODE ));

  if ( !polyModeObj ) {
    polyModeObj = new osg::PolygonMode;
    state->setAttribute( polyModeObj );
  }

  // --------------------------------------------
  //  Now we can set the state to Filled
  // --------------------------------------------

  polyModeObj->setMode(  osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL );

}  // forceWireFrameModeOff




