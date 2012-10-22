from m5.params import *
from O3CPUInjectedFault import O3CPUInjectedFault

class IEWStageInjectedFault(O3CPUInjectedFault):
    type = 'IEWStageInjectedFault'

    tcontext = Param.Int(0, "Example Num") 





