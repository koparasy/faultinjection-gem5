#include "base/types.hh"
#include "base/bitfield.hh"
#include "fi/faultq.hh"
#include "fi/opcode_injfault.hh"
#include "params/OpCodeInjectedFault.hh"
#include "fi/fi_system.hh"

using namespace std;


OpCodeInjectedFault::OpCodeInjectedFault(OpCodeInjectedFault &source)
  : O3CPUInjectedFault(source){
    setFaultType(InjectedFault::OpCodeInjectedFault);
  }

OpCodeInjectedFault::OpCodeInjectedFault(Params *p)
  : O3CPUInjectedFault(p)
{
  setFaultType(InjectedFault::OpCodeInjectedFault);
  fi_system->fetchStageInjectedFaultQueue.insert(this);
}


OpCodeInjectedFault::OpCodeInjectedFault(Params *p,std::ifstream &os)
  : O3CPUInjectedFault(p,os)
{
  setFaultType(InjectedFault::OpCodeInjectedFault);
  fi_system->fetchStageInjectedFaultQueue.insert(this);
}


OpCodeInjectedFault::~OpCodeInjectedFault()
{
}

const std::string
OpCodeInjectedFault::name() const
{
  return params()->name;
}

void OpCodeInjectedFault::store(std::ofstream &os)
{
	O3CPUInjectedFault::store(os);
}

const char *
OpCodeInjectedFault::description() const
{
    return "OpCodeInjectedFault";
}

void
OpCodeInjectedFault::init()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "OpCodeInjectedFault:init()\n";
  }
}

void
OpCodeInjectedFault::startup()
{
  O3CPUInjectedFault::startup();
  if (DTRACE(FaultInjection)) {
    std::cout << "OpCodeInjectedFault:startup()\n";
  }
  dump();
}

OpCodeInjectedFault *
OpCodeInjectedFaultParams::create()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "OpCodeInjectedFaultParams:create()\n";
  }
  return new OpCodeInjectedFault(this);
}

void 
OpCodeInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "OpCodeInjectedFault::dump()\n";
    InjectedFault::dump();
    //std::cout << "CPU module name: " << getCPU()->name() << "\n";
    std::cout << "tcontext: " << getTContext() << "\n";
  }
}


TheISA::MachInst
OpCodeInjectedFault::process(TheISA::MachInst inst)
{
  TheISA::MachInst retInst = 0;

  if (DTRACE(FaultInjection)) {
    std::cout << "===OpCodeInjectedFault::process()===\n";
  }

#ifdef ALPHA_ISA

  switch (getValueType()) {
  case (InjectedFault::ImmediateValue): 
    {
      if (DTRACE(FaultInjection)) {
	std::cout << "\tImmediateValue\n";
	std::cout << "\tinstruction before FI: "<< inst << "\n";
      }
      retInst = insertBits(inst, 31, 26, getValue());
      if (DTRACE(FaultInjection)) {
	std::cout << "\tinstruction after FI: "<< inst << "\n";
      }
      break;
    }
  case (InjectedFault::MaskValue): 
    {
      if (DTRACE(FaultInjection)) {
	std::cout << "\tMaskValue\n";
	std::cout << "\tinstruction before FI: "<< inst << "\n";
      }
      retInst = insertBits(inst, 31,26, bits(inst, 31,26) ^ getValue());
      if (DTRACE(FaultInjection)) {
	std::cout << "\tinstruction after FI: "<< inst << "\n";
      }
      break;
    }
  default: 
    {
      std::cout << "getValueType Default case Error\n";
      assert(0);
      break;
    }
  }

#endif
#ifndef ALPHA_ISA
  assert(0);
#endif

  check4reschedule();

  if (DTRACE(FaultInjection)) {
    std::cout << "~==OpCodeInjectedFault::process()===\n";
  }
  return retInst; 
}


void
OpCodeInjectedFault::check4reschedule()
{
  if (!isManifested()) {
    setManifested(true);
  }
  
  if (getOccurrence() == 1) {//not relative fault injection has already been removed from the queue and inserted into another queue.
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
