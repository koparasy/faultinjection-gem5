#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/genfetch_injfault.hh"
#include "params/GeneralFetchInjectedFault.hh"

using namespace std;


GeneralFetchInjectedFault::GeneralFetchInjectedFault(Params *p)
  : O3CPUInjectedFault(p)
{
	setFaultType(InjectedFault::GeneralFetchInjectedFault);
  fetchStageInjectedFaultQueue.insert(this);
}

GeneralFetchInjectedFault::GeneralFetchInjectedFault(Params *p,std::ifstream &os)
  : O3CPUInjectedFault(p,os)
{
	setFaultType(InjectedFault::GeneralFetchInjectedFault);
  fetchStageInjectedFaultQueue.insert(this);
}


GeneralFetchInjectedFault::GeneralFetchInjectedFault(GeneralFetchInjectedFault &source)
  : O3CPUInjectedFault(source)
{
	setFaultType(InjectedFault::GeneralFetchInjectedFault);
}

GeneralFetchInjectedFault::~GeneralFetchInjectedFault()
{
}


void
GeneralFetchInjectedFault::store(std::ofstream &os)
{
	O3CPUInjectedFault::store(os);
}

const std::string
GeneralFetchInjectedFault::name() const
{
  return params()->name;
}

const char *
GeneralFetchInjectedFault::description() const
{
    return "GeneralFetchInjectedFault";
}

void
GeneralFetchInjectedFault::init()
{
  DPRINTF(FaultInjection, "GeneralFetchInjectedFault:init()\n");
}

void
GeneralFetchInjectedFault::startup()
{
  O3CPUInjectedFault::startup();

  DPRINTF(FaultInjection, "GeneralFetchInjectedFault:startup()\n");

  dump();

  /* We can not schedule faults that manifest in between the pipeline stages 
  *  we scan for them explisitly on every simulation tick
  */

}

GeneralFetchInjectedFault *
GeneralFetchInjectedFaultParams::create()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "GeneralFetchInjectedFaultParams:create()\n";
  }
  return new GeneralFetchInjectedFault(this);
}

void GeneralFetchInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "===GeneralFetchInjectedFault::dump()===\n";
    O3CPUInjectedFault::dump();
    std::cout << "~==GeneralFetchInjectedFault::dump()===\n";
  }
}

TheISA::MachInst
GeneralFetchInjectedFault::process(TheISA::MachInst inst)
{
  TheISA::MachInst retInst = 0;

  DPRINTF(FaultInjection, "===GeneralFetchStageInjectedFault::process()===\n");

  retInst = manifest(inst, getValue(), getValueType());

  check4reschedule();

  DPRINTF(FaultInjection, "~==GeneralFetchStageInjectedFault::process()===\n");
  return retInst; 
}

void
GeneralFetchInjectedFault::check4reschedule()
{
  if (!isManifested()) {
    setManifested(true);
  }
  
  if (getOccurrence() == 1) {//as these faults have not been removed from the fault queue we need to do so on their last occurrence
    getQueue()->remove(this);
    return;
  }
  else {//if the fault has more occurrences increase its manifestation timing value and reschedule it
    Tick cycles = getTiming() + getCPU()->ticks(1);
    int insts = getTiming() + 1;
    Addr addr = getTiming() + (getCPU()->getContext(getTContext())->pcState().nextInstAddr() - getCPU()->getContext(getTContext())->pcState().instAddr());
    increaseTiming(cycles, insts, addr);
  }

  if (getOccurrence() > 1) {//decrease the fault's occurrence
    decreaseOccurrence();
  }

}
