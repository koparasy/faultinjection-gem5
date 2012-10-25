#ifndef __O3CPU_INJECTED_FAULT_HH__
#define __O3CPU_INJECTED_FAULT_HH__

#include "config/the_isa.hh"
#include "base/types.hh"
#include "fi/faultq.hh"
#include "cpu/o3/cpu.hh"
#include "params/O3CPUInjectedFault.hh"
#include "fi/fi_system.hh"

class O3CPUInjectedFaultParams;

class O3CPUInjectedFault : public InjectedFault
{

private:
  /*Pointer to the CPU object that the fault is meant to be injected
   */
  BaseO3CPU *_cpu;
  /* Number of thread context that the fault is intended for
   */
  int _tcontext;
  
  /* The setXXX functions are used to assign values at the above described variable
   */
  void setCPU(BaseO3CPU *v) { _cpu = v;}
  void setTContext(int v) { _tcontext = v;}

public:

  typedef O3CPUInjectedFaultParams Params;
  
  const Params *params() const
  {
    return reinterpret_cast<const Params *>(_params); 
  }

  O3CPUInjectedFault(Params *params);
  O3CPUInjectedFault(O3CPUInjectedFault &source);
	O3CPUInjectedFault(Params *p,std::ifstream &os);
  ~O3CPUInjectedFault();

  const std::string name() const;  
  virtual const char *description() const;
  
  void dump() const;
  O3CPUInjectedFault* copyme(InjectedFaultQueue& myq){ 
    DPRINTF(FaultInjection, "copy me O3CPUInjectedFault\n");
    return (new O3CPUInjectedFault(*this) ); }
  virtual void init();
  virtual void startup();
  //Port* getPort(const std::string &if_name, int idx = 0);
  
  /* This function is called for the fault to manifest the implementation of each fault type should be seen for
   * a better insight on the matter.
   */
  //virtual void manifest(void) = 0;
  virtual TheISA::MachInst process(TheISA::MachInst inst) { std::cout << "O3CPUInjectedFault::manifest() -- virtual\n"; assert(0); return inst;};
  virtual StaticInstPtr process(StaticInstPtr inst) { std::cout << "O3CPUInjectedFault::manifest() -- virtual\n"; assert(0); return inst;};

  /* The getXXX functions are a compliment to the setXXX functions and are used to get the values of the described variable
   */
  BaseCPU *
  getCPU() const { return _cpu;} 
  int
  getTContext() const { return _tcontext;}
	virtual void store(std::ofstream &os);
};

#endif // __O3CPU_INJECTED_FAULT_HH__
