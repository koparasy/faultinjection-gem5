#include "fi/faultq.hh"
#include "fi/reg_injfault.hh"
#include "params/RegisterInjectedFault.hh"

#ifdef ALPHA_ISA
#include "arch/alpha/utility.hh"
#endif
using namespace std;


RegisterInjectedFault::RegisterInjectedFaultEvent::RegisterInjectedFaultEvent(InjectedFaultQueue *q, RegisterInjectedFault *f)
  : Event(49), fault(f), faultq(q)
{
  DPRINTF(Event, "RegisterInjectedFault::RegisterInjectedFaultEvent::RegisterInjectedFaultEvent()\n");
  DPRINTF(FaultInjection, "RegisterInjectedFault::RegisterInjectedFaultEvent::RegisterInjectedFaultEvent()\n");
}


void
RegisterInjectedFault::RegisterInjectedFaultEvent::process()
{
  DPRINTF(Event, "RegisterInjectedFault::RegisterInjectedFaultEvent::process()\n");
  DPRINTF(FaultInjection, "RegisterInjectedFault::RegisterInjectedFaultEvent::process()\n");

#if (FULL_SYSTEM == 1)
  Addr _tmpAddr = fault->getCPU()->getContext(0)->readMiscReg(AlphaISA::IPR_PALtemp23);
  if ((fi_activation.find(_tmpAddr) != fi_activation.end()) && (fi_activation.find(_tmpAddr)->second == -1)) {
#else
  if (fi_active) {
#endif
    if (TheISA::inUserMode(fault->getCPU()->getContext(fault->getTContext()))) {
      fault->process();
    }
    else {
      DPRINTF(FaultInjection, "RegisterInjectedFault::RegisterInjectedFaultEvent::process() - RegisterInjectedFaultEvent not in user mode\n");	  
    }
  }
  else {
    DPRINTF(FaultInjection, "RegisterInjectedFault::RegisterInjectedFaultEvent::process() - RegisterInjectedFaultEvent fault injection is not activated yet\n");
  }

}

const char *
RegisterInjectedFault::RegisterInjectedFaultEvent::description() const
{
    return "Injected Fault Event\n";
}

RegisterInjectedFault::RegisterInjectedFault(RegisterInjectedFault &source,InjectedFaultQueue& myq)
  : CPUInjectedFault(source), RegInjFaultEvent(&myq, this)
{
  setFaultType(InjectedFault::RegisterInjectedFault);
  setRegister(source.getRegister());
  setRegType(source.getRegType());

}


RegisterInjectedFault::RegisterInjectedFault(Params *p)
  : CPUInjectedFault(p), RegInjFaultEvent(&mainInjectedFaultQueue, this)
{
  setFaultType(InjectedFault::RegisterInjectedFault);
  setRegister(p->Register);
  setRegType(p->RegType);

  mainInjectedFaultQueue.insert(this);
}

RegisterInjectedFault::~RegisterInjectedFault()
{
}

const std::string
RegisterInjectedFault::name() const
{
  return params()->name;
}

const char *
RegisterInjectedFault::description() const
{
    return "RegisterInjectedFault";
}

void
RegisterInjectedFault::init()
{
  DPRINTF(FaultInjection, "RegisterInjectedFault:init()\n");
}

void
RegisterInjectedFault::schedule(bool remove)
{
  
  if (remove && (getTimingType() != InjectedFault::VirtualAddrTiming))//remove from the fault queue (only tick and instruction timing queues were schedule so do not remove virtual address timing faults)
    getQueue()->remove(this);

}

void
RegisterInjectedFault::startup()
{
  CPUInjectedFault::startup();

  DPRINTF(FaultInjection, "RegisterInjectedFault:startup()\n");
  
  dump();

  if (!getRelative()) {
    schedule(true);
  }
  else {
    // Not relative faults stay in the queue and will be scheduled when the referenced instruction is executed (check decoder.isa file)
  }
}

RegisterInjectedFault *
RegisterInjectedFaultParams::create()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "RegisterInjectedFaultParams:create()\n";
  }
  return new RegisterInjectedFault(this);
}


void RegisterInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "===RegisterInjectedFault::dump()===\n";
    CPUInjectedFault::dump();
    std::cout << "\tregister: " << getRegister() << "\n";
    std::cout << "\tregType: " << getRegType() << "\n";
    std::cout << "~==RegisterInjectedFault::dump()===\n";
  }
}


Port *
RegisterInjectedFault::getPort(const string &if_name, int idx)
{
  std::cout << "RegisterInjectedFault:getPort() " << "if_name: " << if_name << " idx: " << idx <<  "\n";
  panic("No Such Port\n");
}



int
RegisterInjectedFault::process()
{
  DPRINTF(FaultInjection, "===RegisterInjectedFault::process() ID: %d ===\n", getFaultID());

  switch (getRegType())
    {
    case(RegisterInjectedFault::IntegerRegisterFault):
      {
	TheISA::IntReg regval = getCPU()->getContext(getTContext())->readIntReg(getRegister());
	TheISA::IntReg mask = manifest(regval, getValue(), getValueType());
	getCPU()->getContext(getTContext())->setIntReg(getRegister(), mask);
	//getCPU()->getContext(getTContext())->setIntReg(getRegister(), mask, false);
	break;
      }
    case(RegisterInjectedFault::FloatRegisterFault):
      {
	TheISA::FloatReg regval = getCPU()->getContext(getTContext())->readFloatReg(getRegister());
	TheISA::FloatReg mask = manifest(regval, getValue(), getValueType());
	getCPU()->getContext(getTContext())->setFloatReg(getRegister(), mask);
	//getCPU()->getContext(getTContext())->setFloatReg(getRegister(), mask, false);
	break;
      }
    case(RegisterInjectedFault::MiscRegisterFault):
      {
	TheISA::MiscReg regval = getCPU()->getContext(getTContext())->readMiscReg(getRegister());
	TheISA::MiscReg mask = manifest(regval, getValue(), getValueType());
	getCPU()->getContext(getTContext())->setMiscReg(getRegister(), mask);
	//getCPU()->getContext(getTContext())->setMiscReg(getRegister(), mask, false);
	break;
      }
    default:
      {
	std::cout << "RegisterInjectedFault::process() -- Default case getRegType() Error\n";
	assert(0);
	break;
      }
    }
  
  //dump();
  check4reschedule();

  DPRINTF(FaultInjection, "~==RegisterInjectedFault::process() ID: %d ===\n", getFaultID());

  return 0;
}


void
RegisterInjectedFault::check4reschedule()
{

  if (!isManifested()) {
    setManifested(true);
  }
   
  if (getOccurrence() != 1) {//if the fault has more occurrences increase its manifestation timing value and reschedule it
    Tick cycles = getTiming() + getCPU()->ticks(1);
    int insts = getTiming() + 1;
    Addr addr = getTiming() + (getCPU()->getContext(getTContext())->pcState().nextInstAddr() - getCPU()->getContext(getTContext())->pcState().instAddr());
    increaseTiming(cycles, insts, addr);
    
    schedule(false);
  }


  /*******NOT Accurate anymore (read next comment)
    This is the place where we should be checking if a fault is to be removed from a queue as it has already been manifested.
   * Removal from the fault queues for Tick and Inst timing type faults is done on their first schedule(), as for Virtual Address timing faults as they are turned to another type we do not need to remove them from the queue(removal is done at the convertion point)
   */

  /*Imporove comment*/
  /* we only remove Address timing type fautls here, as all other faults have been removed already.
     this was not done previously because we changed Address timing type faults to another type after their first manifestation (because of loop cases)
     however the functionality of address timing type faults has changed. We now use them for injecting faults in parts of the application with great precision and hard faults occure on the same address each time
  */
  

  if ((getOccurrence() == 1)) {
    getQueue()->remove(this);
  }

  if (getOccurrence() > 1) {//decrease the fault's occurrence
    decreaseOccurrence();
  }

}
