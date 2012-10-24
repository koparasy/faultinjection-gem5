#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/iew_injfault.hh"

using namespace std;


IEWStageInjectedFault::IEWStageInjectedFault(Params *p)
  : O3CPUInjectedFault(p)
{
  /*TODO: need to check if the returned class is of class BaseCPU
   */
  //setTContext(p->tcontext);
  setFaultType(InjectedFault::ExecutionInjectedFault);
  iewStageInjectedFaultQueue.insert(this);
}

IEWStageInjectedFault::IEWStageInjectedFault(IEWStageInjectedFault &source)
  : O3CPUInjectedFault(source)
{
	setFaultType(InjectedFault::ExecutionInjectedFault);
}

IEWStageInjectedFault::IEWStageInjectedFault(Params *p,std::ifstream &os)
  : O3CPUInjectedFault(p,os)
{
	setFaultType(InjectedFault::ExecutionInjectedFault);
  iewStageInjectedFaultQueue.insert(this);
}

IEWStageInjectedFault::~IEWStageInjectedFault()
{
}

void 
IEWStageInjectedFault::store(std::ofstream &os){
	O3CPUInjectedFault::store(os);
}

const std::string
IEWStageInjectedFault::name() const
{
  return params()->name;
}

const char *
IEWStageInjectedFault::description() const
{
    return "IEWStageInjectedFault";
}

void
IEWStageInjectedFault::init()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "IEWStageInjectedFault:init()\n";
  }
}

void
IEWStageInjectedFault::startup()
{
  O3CPUInjectedFault::startup();

  /*
  setCPU(reinterpret_cast<BaseO3CPU *>(find(getWhere().c_str())));
  if (!getCPU()) { 
    std::cout << "CPU name is not correct: "<< getWhere() << "\n";
    exit(1);
  }
  */
  if (DTRACE(FaultInjection)) {
    std::cout << "IEWStageInjectedFault:startup()\n";
  }
  dump();

}

IEWStageInjectedFault *
IEWStageInjectedFaultParams::create()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "IEWStageInjectedFaultParams:create()\n";
  }
  return new IEWStageInjectedFault(this);
}

void
IEWStageInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "===IEWStageInjectedFault::dump()===\n";
    InjectedFault::dump();
//    std::cout << "\tCPU module name: " << getCPU()->name() << "\n";
    std::cout << "\ttcontext: " << getTContext() << "\n";
    std::cout << "~==IEWStageInjectedFault::dump()===\n";
  }
}


void
IEWStageInjectedFault::check4reschedule()
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

/*
float
IEWStageInjectedFault::process(float v)
{
  float retVal = v;

  if (DTRACE(FaultInjection)) {
    std::cout << "===IEWStageInjectedFault::process(float)===\n";
  }

#ifdef ALPHA_ISA 
  switch (getValueType()) {
  case (InjectedFault::ImmediateValue): 
    {
      if (DTRACE(FaultInjection)) {
	std::cout << "\tImmediateValue\n";
	printf("\tvalue before FI: %f\n", v);
      }
      retVal = IMMD(v,getValue());
      if (DTRACE(FaultInjection)) {
	printf("\tvalue after FI: %f\n", retVal);
      }
      break;
    }
  case (InjectedFault::MaskValue): 
    {
      if (DTRACE(FaultInjection)) {
	std::cout << "\tMaskValue\n";
	printf("\tvalue before FI: %f\n", v);
      }
      retVal = XOR(v, getValue());
      if (DTRACE(FaultInjection)) {
	printf("\tvalue after FI: %f\n", retVal);
      }
      break;
    }
  default: 
    {
      std::cout << "IEWStageInjectedFault::process() -- Default case Error\n";
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
    std::cout << "~==IEWStageInjectedFault::process(float)===\n";
  }
  return retVal;
}


double
IEWStageInjectedFault::process(double v)
{ 
  double retVal = v;

  if (DTRACE(FaultInjection)) {
    std::cout << "===IEWStageInjectedFault::process(double)===\n";
  }

#ifdef ALPHA_ISA 
  switch (getValueType()) {
  case (InjectedFault::ImmediateValue): 
    {
      if (DTRACE(FaultInjection)) {
	std::cout << "\tImmediateValue\n";
	printf("\tvalue before FI: %f\n", v);
      }
      retVal = IMMD(v, getValue());
      if (DTRACE(FaultInjection)) {
	printf("\tvalue after FI: %f\n", retVal);
      }
      break;
    }
  case (InjectedFault::MaskValue): 
    {
      if (DTRACE(FaultInjection)) {
	std::cout << "\tMaskValue\n";
	printf("\tvalue before FI: %f\n", v);
      }
      retVal = XOR(v, getValue());
      if (DTRACE(FaultInjection)) {
	printf("\tvalue after FI: %f\n", retVal);
      }
      break;
    }
  default: 
    {
      std::cout << "IEWStageInjectedFault::process() -- Default case Error\n";
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
    std::cout << "~==IEWStageInjectedFault::process(double)===\n";
  }
  return retVal;
}

uint64_t
IEWStageInjectedFault::process(uint64_t v)
{ 
  uint64_t retVal = v;

  if (DTRACE(FaultInjection)) {
    std::cout << "===IEWStageInjectedFault::process(uint64_t)===\n";
  }

#ifdef ALPHA_ISA 
  switch (getValueType()) {
  case (InjectedFault::ImmediateValue): 
    {
      if (DTRACE(FaultInjection)) {
	std::cout << "\tImmediateValue\n";
	printf("\tvalue before FI: %llu\n", v);
      }
      retVal = IMMD(v, getValue());
      if (DTRACE(FaultInjection)) {
	printf("\tvalue after FI: %llu\n", retVal);
      }
      break;
    }
  case (InjectedFault::MaskValue): 
    {
      if (DTRACE(FaultInjection)) {
	std::cout << "\tMaskValue\n";
	printf("\tvalue before FI: %llu\n", v);
      }
      retVal = XOR(v, getValue());
      if (DTRACE(FaultInjection)) {
	printf("\tvalue after FI: %llu\n", retVal);
      }
      break;
    }
  default: 
    {
      std::cout << "IEWStageInjectedFault::process() -- Default case Error\n";
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
    std::cout << "~==IEWStageInjectedFault::process(uint64_t)===\n";
  }
  return retVal;
}
*/
