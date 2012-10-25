#include "fi/faultq.hh"
#include "fi/pc_injfault.hh"
#include "params/PCInjectedFault.hh"
#include "fi/fi_system.hh"

#ifdef ALPHA_ISA
#include "arch/alpha/utility.hh"
#endif
using namespace std;


PCInjectedFault::PCInjectedFaultEvent::PCInjectedFaultEvent(InjectedFaultQueue *q, PCInjectedFault *f)
  : Event(49), fault(f), faultq(q)
{
  DPRINTF(Event, "PCInjectedFault::PCInjectedFaultEvent::PCInjectedFaultEvent()\n");
  DPRINTF(FaultInjection, "PCInjectedFault::PCInjectedFaultEvent::PCInjectedFaultEvent()\n");
}


void
PCInjectedFault::PCInjectedFaultEvent::process()
{
  DPRINTF(Event, "PCInjectedFault::PCInjectedFaultEvent::process()\n");
  DPRINTF(FaultInjection, "PCInjectedFault::PCInjectedFaultEvent::process()\n");
    
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
      DPRINTF(FaultInjection, "PCInjectedFault::PCInjectedFaultEvent::process() - PCInjectedFaultEvent not in user mode\n");	  
    }
  }
  else {
    DPRINTF(FaultInjection, "PCInjectedFault::PCInjectedFaultEvent::process() - PCInjectedFaultEvent fault injection is not activated yet\n");
  }

}

const char *
PCInjectedFault::PCInjectedFaultEvent::description() const
{
    return "PC Injected Fault Event\n";
}

PCInjectedFault::PCInjectedFault(PCInjectedFault &source,InjectedFaultQueue& myq)
  : CPUInjectedFault(source), PCInjFaultEvent(&myq, this)
{
  setFaultType(InjectedFault::PCInjectedFault);
  fi_system->mainInjectedFaultQueue.insert(this);
}

PCInjectedFault::PCInjectedFault(Params *p, std::ifstream &os)
	:CPUInjectedFault(p,os), PCInjFaultEvent(&(fi_system->mainInjectedFaultQueue), this){
	 setFaultType(InjectedFault::PCInjectedFault);
	 fi_system->mainInjectedFaultQueue.insert(this);
}

PCInjectedFault::PCInjectedFault(Params *p)
  : CPUInjectedFault(p), PCInjFaultEvent(&(fi_system->mainInjectedFaultQueue), this)
{
  setFaultType(InjectedFault::PCInjectedFault);

  fi_system->mainInjectedFaultQueue.insert(this);
}

void
PCInjectedFault::store(std::ofstream &os){
	CPUInjectedFault::store(os);
}


PCInjectedFault::~PCInjectedFault()
{
}

const std::string
PCInjectedFault::name() const
{
  return params()->name;
}

const char *
PCInjectedFault::description() const
{
    return "PCInjectedFault";
}

void
PCInjectedFault::init()
{
  DPRINTF(FaultInjection, "PCInjectedFault:init()\n");
}


void
PCInjectedFault::schedule(bool remove)
{
  /*
  if (getTimingType()  == InjectedFault::TickTiming) {//schedule at tick Event Queue
    mainEventQueue.schedule(&PCInjFaultEvent, Tick(getTiming()));
  }
  else if (getTimingType()  == InjectedFault::InstructionTiming) {//schedule at instruction Event Queue
    getCPU()->comInstEventQueue[getTContext()]->schedule(&PCInjFaultEvent, Tick(getTiming()));
  }
  */
  if (remove && (getTimingType() != InjectedFault::VirtualAddrTiming))//remove from the fault queue (only tick and instruction timing queues were schedule so do not remove virtual address timing faults)
    getQueue()->remove(this);

}

void
PCInjectedFault::startup()
{
  CPUInjectedFault::startup();

  if (DTRACE(FaultInjection)) {
    std::cout << "PCInjectedFault:startup()\n";
  }

  dump();

  if (!getRelative()) {
    schedule(true);
  }
  else {
    // Not relative faults stay in the queue and will be scheduled when the referenced instruction is executed (check decoder.isa file)
  }
}

PCInjectedFault *
PCInjectedFaultParams::create()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "PCInjectedFaultParams:create()\n";
  }
    return new PCInjectedFault(this);
}


void 
PCInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "===PCInjectedFault::dump()===\n";
    CPUInjectedFault::dump();
    std::cout << "~==PCInjectedFault::dump()===\n";
  }
}


Port *
PCInjectedFault::getPort(const string &if_name, int idx)
{
  std::cout << "PCInjectedFault:getPort() " << "if_name: " << if_name << " idx: " << idx <<  "\n";
  panic("No Such Port\n");
}


int
PCInjectedFault::process()
{
  DPRINTF(FaultInjection, "===PCInjectedFault::process()===\n");

  //wrong with the new implementation
  uint64_t pcval = getCPU()->getContext(getTContext())->pcState().instAddr();
  DPRINTF(FaultInjection, "\tPC value before FI: %lx\n", pcval);
  uint64_t mask = manifest(pcval, getValue(), getValueType());
  //wrong with the new implementation
  getCPU()->getContext(getTContext())->pcState(TheISA::PCState(mask));
  //getCPU()->getContext(getTContext())->pcState().set(mask, false);
  /*
   * In previous versions we also changed the NextPC to point to the next address, however, this is incorrect
   *
   * getCPU()->getContext(getTContext())->setNextPC(mask + sizeof(TheISA::MachInst), false);
   */
  
  dump();
  check4reschedule();

  DPRINTF(FaultInjection, "~==PCInjectedFault::process()===\n");

  return 0;
}

void
PCInjectedFault::check4reschedule()
{
  if (!isManifested()) {
    setManifested(true);
  }
  
  if (getOccurrence() != 1) {//if the fault has more occurrences increase its menifestation timing value and reschedule it
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
  

  if ((getOccurrence() == 1) ) {
    getQueue()->remove(this);
  }
  
  if (getOccurrence() > 1) {//decrease the fault's occurrence
    decreaseOccurrence();
  }

}
