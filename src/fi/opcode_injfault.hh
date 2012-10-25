#ifndef __OPCODE_INJECTED_FAULT_HH__
#define __OPCODE_INJECTED_FAULT_HH__

#include "config/the_isa.hh"
#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/o3cpu_injfault.hh"
#include "cpu/o3/cpu.hh"
#include "params/OpCodeInjectedFault.hh"
#include "fi/fi_system.hh"

class OpCodeInjectedFaultParams;

class OpCodeInjectedFault : public O3CPUInjectedFault
{

public:

  typedef OpCodeInjectedFaultParams Params;
  
  const Params *params() const
  {
    return reinterpret_cast<const Params *>(_params); 
  } 

  OpCodeInjectedFault(Params *params);
  OpCodeInjectedFault(OpCodeInjectedFault &source);
	OpCodeInjectedFault(Params *p, std::ifstream &os);
  ~OpCodeInjectedFault();

  const std::string name() const;  
  virtual const char *description() const;
  
  void dump() const;

  virtual void init();
  virtual void startup();
  
  TheISA::MachInst process(TheISA::MachInst inst);
  OpCodeInjectedFault* copyme(InjectedFaultQueue& myq){ 
    DPRINTF(FaultInjection,"copy me OpCodeInjectedFault\n");
    return (new OpCodeInjectedFault(*this) ); }

  void check4reschedule();

	virtual void store(std::ofstream &os);

};

#endif // __OPCODE_INJECTED_FAULT_HH__
