#include "fi/faultq.hh"
#include "fi/mem_injfault.hh"
#include "params/MemoryInjectedFault.hh"

using namespace std;


MemoryInjectedFault::MemoryInjectedFaultEvent::MemoryInjectedFaultEvent(InjectedFaultQueue *q, MemoryInjectedFault *f)
  : Event(49), fault(f), faultq(q)
{
  DPRINTF(Event, "MemoryInjectedFault::MemoryInjectedFaultEvent::MemoryInjectedFaultEvent()\n");
  DPRINTF(FaultInjection, "MemoryInjectedFault::MemoryInjectedFaultEvent::MemoryInjectedFaultEvent()\n");
}


void
MemoryInjectedFault::MemoryInjectedFaultEvent::process()
{
  DPRINTF(Event, "MemoryInjectedFault::MemoryInjectedFaultEvent::process()\n");
  DPRINTF(FaultInjection, "MemoryInjectedFault::MemoryInjectedFaultEvent::process()\n");

  /*
  * Memory is unrelated to the execution mode of the processor or if an application is currently executing
  * so we make no check for that.
  * Something that we maybe intrested in is relating a fault injection to a specific virtual address space (of an application or so)
  */

  if (fi_active) {
    fault->process();
  }
  else {
    DPRINTF(FaultInjection, "MemoryInjectedFault::MemoryInjectedFaultEvent::process() -- MemoryInjectedFaultEvent fault injection is not activated yet\n");
  }

}

const char *
MemoryInjectedFault::MemoryInjectedFaultEvent::description() const
{
    return "Memory Injected Fault Event\n";
}



MemoryInjectedFault::MemoryInjectedFault(MemoryInjectedFault &source,InjectedFaultQueue& myq):InjectedFault(source),
  MemInjFaultEvent(&myq, this){
     setAddress(source.getAddress());
     setFaultType(InjectedFault::MemoryInjectedFault);
     pMem = reinterpret_cast<PhysicalMemory *>(find((source.getWhere()).c_str()));  
  }

MemoryInjectedFault::MemoryInjectedFault(Params *p)
  : InjectedFault(p), MemInjFaultEvent(&mainInjectedFaultQueue, this), _address(p->address)
{
  DPRINTF(FaultInjection, "MemoryInjectedFault::MemoryInjectedFault()\n");

  setFaultType(InjectedFault::MemoryInjectedFault);
  pMem = reinterpret_cast<PhysicalMemory *>(find(p->where.c_str()));  

  mainInjectedFaultQueue.insert(this);
}


MemoryInjectedFault::MemoryInjectedFault(Params *p, std::ifstream &os)
	: InjectedFault(p,os), MemInjFaultEvent(&mainInjectedFaultQueue, this){
		Addr k;
		os>>k;
		setAddress(k);
		setFaultType(InjectedFault::MemoryInjectedFault);
		pMem = reinterpret_cast<PhysicalMemory *>(find((getWhere()).c_str())); 
		mainInjectedFaultQueue.insert(this);
	}

void 
MemoryInjectedFault::store(std::ofstream &os){
	InjectedFault::store(os);
	os<<getAddress();
	os<<" ";
}

MemoryInjectedFault::~MemoryInjectedFault()
{
}

const std::string
MemoryInjectedFault::name() const
{
  return params()->name;
}

const char *
MemoryInjectedFault::description() const
{
    return "MemoryInjectedFault";
}

void
MemoryInjectedFault::init()
{
  DPRINTF(FaultInjection, "MemoryInjectedFault:init()\n");
}

void
MemoryInjectedFault::schedule(bool remove)
{
  /* Memory faults injection timing can only be specified in simulation ticks we could support and instructions 
   * but it will be coupled to a specific CPU status
   */

  if (getTimingType()  == InjectedFault::TickTiming) {//schedule at tick Event Queue
    mainEventQueue.schedule(&MemInjFaultEvent, Tick(getTiming()));
  }
  else {
    std::cout << "MemoryInjectedFault::schedule() -- Instruction timing type not supported\n";
    assert(0);
  }

  if (remove && (getTimingType() != InjectedFault::VirtualAddrTiming))//remove from the fault queue (only tick and instruction timing queues were schedule so do not remove virtual address timing faults)
    getQueue()->remove(this);
}

void
MemoryInjectedFault::startup()
{
  DPRINTF(FaultInjection, "MemoryInjectedFault:startup()\n");
  
  dump();
  
  if (!getRelative()) {
    schedule(true);
  }
  else {
    // Not relative faults stay in the queue and will be scheduled when the referenced instruction is executed (check decoder.isa file)
  }

}

MemoryInjectedFault *
MemoryInjectedFaultParams::create()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "MemoryInjectedFaultParams:create()\n";
  }
    return new MemoryInjectedFault(this);
}

void MemoryInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "~==MemoryInjectedFault::dump()===\n";
    InjectedFault::dump();
    std::cout << "\tAddress is: " << _address << "\n";
    std::cout << "~==MemoryInjectedFault::dump()===\n";
  }
}



Port *
MemoryInjectedFault::getPort(const string &if_name, int idx)
{
  std::cout << "MemoryInjectedFault:getPort() " << "if_name: " << if_name << " idx: " << idx <<  "\n";
  panic("No Such Port\n");
}

int
MemoryInjectedFault::process()
{
  DPRINTF(FaultInjection, "===MemoryInjectedFault::process()===\n");

  uint8_t memval = pMem->pmemAddr[_address];
  uint8_t mask = manifest(memval, (uint8_t)getValue(), getValueType());
  pMem->pmemAddr[_address] = mask;
 
  dump();

  DPRINTF(FaultInjection, "~==MemoryInjectedFault::process()===\n");

  return 0;
}


void
MemoryInjectedFault::check4reschedule()
{
  if (!isManifested()) {
    setManifested(true);
  }
  
  if (getOccurrence() != 1) {//if the fault has more occurrences increase its menifestation timing value and reschedule it
    Tick cycles = getTiming() + 1;
    int insts = 0;
    Addr addr = 0;
    increaseTiming(cycles, insts, addr);
    
    schedule(false);
  }

  
  if (getOccurrence() > 1) {//decrease the fault's occurrence
    decreaseOccurrence();
  }

}
