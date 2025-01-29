#ifndef G4NCrystalRel_hh
#define G4NCrystalRel_hh

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  This file is part of NCrystal (see https://mctools.github.io/ncrystal/)   //
//                                                                            //
//  Copyright 2015-2023 NCrystal developers                                   //
//                                                                            //
//  Licensed under the Apache License, Version 2.0 (the "License");           //
//  you may not use this file except in compliance with the License.          //
//  You may obtain a copy of the License at                                   //
//                                                                            //
//      http://www.apache.org/licenses/LICENSE-2.0                            //
//                                                                            //
//  Unless required by applicable law or agreed to in writing, software       //
//  distributed under the License is distributed on an "AS IS" BASIS,         //
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  //
//  See the License for the specific language governing permissions and       //
//  limitations under the License.                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////
// Convenience header for all G4NCrystal interfaces //
//////////////////////////////////////////////////////

#ifndef G4NCrystalRel_Install_hh
#  include "G4NCrystalRel/G4NCInstall.hh"
#endif
#ifndef G4NCrystalRel_Manager_hh
#  include "G4NCrystalRel/G4NCManager.hh"
#endif
#ifndef G4NCrystalRel_MatHelper_hh
#  include "G4NCrystalRel/G4NCMatHelper.hh"
#endif

#include "NCrystal/ncapi.h"
#if NCRYSTAL_VERSION >= 3009080
#  include "NCrystal/factories/NCMatCfg.hh"
#else
#  include "NCrystal/NCMatCfg.hh"
#endif

#endif
