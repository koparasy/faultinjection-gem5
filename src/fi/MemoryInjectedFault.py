from m5.params import *
from InjectedFault import InjectedFault

class MemoryInjectedFault(InjectedFault):
    type = 'MemoryInjectedFault'

    address = Param.UInt64(0, "Example Num") 
#    when = Param.String("", "Example Num") 
#    where = Param.String("", "Example Num") 
#    what = Param.String("", "Example Num") 





