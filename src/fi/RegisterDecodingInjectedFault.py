from m5.params import *
from O3CPUInjectedFault import O3CPUInjectedFault

class RegisterDecodingInjectedFault(O3CPUInjectedFault):
    type = 'RegisterDecodingInjectedFault'

    regDec = Param.String("", "Register Decoding Error: <Src/Dst>:<Number>:<Value>") 





