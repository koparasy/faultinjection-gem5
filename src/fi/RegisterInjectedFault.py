from m5.params import *
from CPUInjectedFault import CPUInjectedFault

class RegisterInjectedFault(CPUInjectedFault):
    type = 'RegisterInjectedFault'

    Register = Param.Int(0, "Register") 
    RegType = Param.String("int", "Register Type")




