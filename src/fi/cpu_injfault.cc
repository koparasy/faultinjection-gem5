#include "fi/faultq.hh"
#include "fi/cpu_injfault.hh"
#include "params/CPUInjectedFault.hh"

using namespace std;


CPUInjectedFault::CPUInjectedFault(Params *p)
  : InjectedFault(p)
{
  setTContext(p->tcontext);
}

CPUInjectedFault::CPUInjectedFault(CPUInjectedFault &source):InjectedFault(source){
  setTContext(source.getTContext());
}

CPUInjectedFault::~CPUInjectedFault()
{
}

const std::string
CPUInjectedFault::name() const
{
  return params()->name;
}

const char *
CPUInjectedFault::description() const
{
    return "CPUInjectedFault";
}

void
CPUInjectedFault::init()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "CPUInjectedFault:init()\n";
  }
}

void
CPUInjectedFault::startup()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "CPUInjectedFault:startup()\n";
  }

  /*TODO: need to check if the returned class is of class BaseCPU
   * it would be best if we can support multiple abstraction levels of CPUs (03, atomic, timing)
   */
  
  setCPU(reinterpret_cast<BaseCPU *>(find(getWhere().c_str())));
  if (!getCPU() && ((getWhere()).compare("all"))) { 
    std::cout << "CPUInjectedFault::startup() - CPU name is not correct: "<< getWhere() <<" "<<  find(getWhere().c_str()) << "\n";
    assert(0);
  }
}

CPUInjectedFault *
CPUInjectedFaultParams::create()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "CPUInjectedFaultParams:create()\n";
  }
    return new CPUInjectedFault(this);
}


 

void CPUInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "===CPUInjectedFault::dump()===\n";
    InjectedFault::dump();  
    //std::cout << "\tCPU module name: " << getCPU()->name() << "\n";
    std::cout << "\ttcontext: " << getTContext() << "\n";
    std::cout << "~==CPUInjectedFault::dump()===\n";
  }

}



Port *
CPUInjectedFault::getPort(const string &if_name, int idx)
{
  std::cout << "CPUInjectedFault:getPort() " << "if_name: " << if_name << " idx: " << idx <<  "\n";
  panic("No Such Port\n");
}

