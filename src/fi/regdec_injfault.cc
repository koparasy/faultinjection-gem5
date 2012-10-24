#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/regdec_injfault.hh"
#include "params/RegisterDecodingInjectedFault.hh"

using namespace std;

RegisterDecodingInjectedFault::RegisterDecodingInjectedFault(RegisterDecodingInjectedFault &source)
  :O3CPUInjectedFault(source)
{
 setChangeToReg(source.getChangeToReg());
 setRegToChange(source.getRegToChange());
 setSrcOrDst(source.getSrcOrDst());
 setFaultType(InjectedFault::RegisterDecodingInjectedFault);
}

RegisterDecodingInjectedFault::RegisterDecodingInjectedFault(Params *p)
  : O3CPUInjectedFault(p)
{
  parseRegDec(p->regDec);
  decodeStageInjectedFaultQueue.insert(this);
	setFaultType(InjectedFault::RegisterDecodingInjectedFault);
}

RegisterDecodingInjectedFault::RegisterDecodingInjectedFault(Params *p, std::ifstream &os)
	:O3CPUInjectedFault(p,os)
{
	string s;
	os>>s;
	parseRegDec(s);
	decodeStageInjectedFaultQueue.insert(this);
	setFaultType(InjectedFault::RegisterDecodingInjectedFault);
}



RegisterDecodingInjectedFault::~RegisterDecodingInjectedFault()
{
}

const std::string
RegisterDecodingInjectedFault::name() const
{
  return params()->name;
}


void 
RegisterDecodingInjectedFault:: store(ofstream &os)
{
		O3CPUInjectedFault::store(os);
		if(getSrcOrDst()==SrcRegisterInjectedFault)
			os<<"Src:";
		else
			os<<"Dst:";
	
			
	os<<getRegToChange();
	os<<":";
	os<<getChangeToReg();
	os<<" ";
}


const char *
RegisterDecodingInjectedFault::description() const
{
    return "RegisterDecodingInjectedFault";
}

void
RegisterDecodingInjectedFault::init()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "RegisterDecodingInjectedFault:init()\n";
  }
}

void
RegisterDecodingInjectedFault::startup()
{
  O3CPUInjectedFault::startup();
  if (DTRACE(FaultInjection)) {
    std::cout << "RegisterDecodingInjectedFault:startup()\n";
  }
  dump();
}

RegisterDecodingInjectedFault *
RegisterDecodingInjectedFaultParams::create()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "RegisterDecodingInjectedFaultParams:create()\n";
  }
  return new RegisterDecodingInjectedFault(this);
}

void 
RegisterDecodingInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "===RegisterDecodingInjectedFault::dump()===\n";
    O3CPUInjectedFault::dump();
    std::cout << "\tSrc Or Dst: " << getSrcOrDst() << "\n";
    std::cout << "\tRegister to Change: " << getRegToChange() << "\n";
    std::cout << "\tChange to Reg: " << getChangeToReg() << "\n";
    std::cout << "~==RegisterDecodingInjectedFault::dump()===\n";
  }
}

StaticInstPtr
RegisterDecodingInjectedFault::process(StaticInstPtr staticInst)
{
  if (DTRACE(FaultInjection)) {
    std::cout << "===RegisterDecodingInjectedFault::process()===\n";
  }
  
  if (getSrcOrDst() == RegisterDecodingInjectedFault::SrcRegisterInjectedFault) {
    int rTc = getRegToChange();
      if (rTc < staticInst->numSrcRegs()) {
	staticInst->_srcRegIdx[rTc] = getChangeToReg();
      }
  }
  else {
    int rTc = getRegToChange();
      if (rTc < staticInst->numDestRegs()) {
	staticInst->_destRegIdx[rTc] = getChangeToReg();
      }
  }  

  
  check4reschedule();

  if (DTRACE(FaultInjection)) {
    std::cout << "~==RegisterDecodingInjectedFault::process()===\n";
  }
  return staticInst;
}

void
RegisterDecodingInjectedFault::check4reschedule()
{
  if (!isManifested()) {
    setManifested(true);
  }

  if (getOccurrence() == 1) {
    getQueue()->remove(this);
    return;
  }
  else {
    Tick cycles = getTiming() + getCPU()->ticks(1);
    int insts = getTiming() + 1;
    Addr addr = getTiming() + (getCPU()->getContext(getTContext())->pcState().nextInstAddr() - getCPU()->getContext(getTContext())->pcState().instAddr());
    increaseTiming(cycles, insts, addr);
  }

  if (getOccurrence() > 1) {
    decreaseOccurrence();
  }


}

int
RegisterDecodingInjectedFault::parseRegDec(std::string s)
{
  if (DTRACE(FaultInjection)) {
    std::cout << "RegisterDecodingInjecteFault::parseRegDec()\n";
  }
  size_t pos;

  if (s.compare(0,3,"Src",0,3) == 0) {
    setSrcOrDst(RegisterDecodingInjectedFault::SrcRegisterInjectedFault);

    std::string s2 = s.substr(4);
    pos = s2.find_first_of(":");

    setRegToChange(s2.substr(0,pos));
    setChangeToReg(s2.substr(pos+1));
  }
  else if (s.compare(0,3,"Dst",0,3) == 0) {
    setSrcOrDst(RegisterDecodingInjectedFault::DstRegisterInjectedFault);

    std::string s2 = s.substr(4);
    pos = s2.find_first_of(":");

    setRegToChange(s2.substr(0,pos));
    setChangeToReg(s2.substr(pos+1));
    
  }
  else {
    std::cout << "RegisterDecodingInjecteFault::parseRegDec(): " << s << "\n";
    assert(0);
    return 1;
  }
  
  return 0;
}
