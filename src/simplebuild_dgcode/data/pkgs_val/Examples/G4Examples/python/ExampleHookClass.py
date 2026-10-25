from ._ExampleHookClass import create_handle as _create_handle
from G4Interfaces.ToPy import component_creator as _component_creator
ExampleHookClass = _component_creator( _create_handle )
