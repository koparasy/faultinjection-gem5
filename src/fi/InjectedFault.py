from m5.params import *
from MemObject import MemObject

class InjectedFault(MemObject):
    type = 'InjectedFault'

    when = Param.String("", "Example String") 
    where = Param.String("", "Example String") 
    what = Param.String("", "Example String") 
    relative = Param.Bool( False, "relevant use of timming")
    occurrence = Param.Int( 1, "Occurrence of fault")
    threadId = Param.String("", "Example String") 
    cores = Param.Int(1,"Number Of cores")



