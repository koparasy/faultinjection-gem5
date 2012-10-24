#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/o3cpu_injfault.hh"

using namespace std;


O3CPUInjectedFault::O3CPUInjectedFault(Params *p)
  : InjectedFault(p)
{
  setTContext(p->tcontext);
}

O3CPUInjectedFault::O3CPUInjectedFault(O3CPUInjectedFault &source)
  : InjectedFault(source)
{
  setTContext(source.getTContext());
}

O3CPUInjectedFault::O3CPUInjectedFault(Params *p, std::ifstream &os)
: InjectedFault(p,os){
	int t;
	os>>t;
	setTContext(t);
}


O3CPUInjectedFault::~O3CPUInjectedFault()
{
}

const std::string
O3CPUInjectedFault::name() const
{
  return params()->name;
}

const char *
O3CPUInjectedFault::description() const
{
    return "O3CPUInjectedFault";
}


void 
O3CPUInjectedFault::store(std::ofstream &os){
	InjectedFault::store(os);
	os<<getTContext();
	os<<" ";
	
} 

void
O3CPUInjectedFault::init()
{
  DPRINTF(FaultInjection, "O3CPUInjectedFault:init()\n");
}

void
O3CPUInjectedFault::startup()
{
  /*TODO: need to check if the returned class is of class BaseCPU
   */
  setCPU(reinterpret_cast<BaseO3CPU *>(find(getWhere().c_str())));
  if (!getCPU() && ((getWhere()).compare("all"))) { 
    std::cout << "O3CPUInjectedFault::startup() -- CPU name is not correct: "<< getWhere() << "\n";
    assert(0);
  }
  DPRINTF(FaultInjection, "O3CPUInjectedFault:startup()\n");

  dump();
}

O3CPUInjectedFault *
O3CPUInjectedFaultParams::create()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "O3CPUInjectedFaultParams:create()\n";
  }
  return new O3CPUInjectedFault(this);
}

void
O3CPUInjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "===O3CPUInjectedFault::dump()===\n";
    InjectedFault::dump();
    //std::cout << "\tCPU module name: " << getCPU()->name() << "\n";
    std::cout << "\ttcontext: " << getTContext() << "\n";
    std::cout << "~==O3CPUInjectedFault::dump()===\n";
  }
}

/*
TheISA::MachInst
O3CPUInjectedFault::manifest(TheISA::MachInst inst)
{   
  std::cout << "O3CPUInjectedFault::manifest() -- Should never be called\n";
  assert(0);
  return inst;
}
*/

/*
Port *
FetchStageInjectedFault::getPort(const string &if_name, int idx)
{
  std::cout << "FetchStageInjectedFault:getPort() " << "if_name: " << if_name << " idx: " << idx <<  "\n";
  panic("No Such Port\n");
}
*/
