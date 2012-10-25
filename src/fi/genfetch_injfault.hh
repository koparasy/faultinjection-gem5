#ifndef __GENERAL_FETCH_INJECTED_FAULT_HH__
#define __GENERAL_FETCH_INJECTED_FAULT_HH__

#include "config/the_isa.hh"
#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/o3cpu_injfault.hh"
#include "cpu/o3/cpu.hh"
#include "params/GeneralFetchInjectedFault.hh"
#include "fi/fi_system.hh"

class GeneralFetchInjectedFaultParams;

class GeneralFetchInjectedFault : public O3CPUInjectedFault
{

public:

  typedef GeneralFetchInjectedFaultParams Params;
  
  const Params *params() const
  {
    return reinterpret_cast<const Params *>(_params); 
  } 

  GeneralFetchInjectedFault(Params *params);
  GeneralFetchInjectedFault(GeneralFetchInjectedFault &source);
	GeneralFetchInjectedFault(Params *p, std::ifstream &os);
  ~GeneralFetchInjectedFault();

  const std::string name() const; 
  virtual const char *description() const;
  
  void dump() const;

  virtual void init();
  virtual void startup();
  GeneralFetchInjectedFault* copyme(InjectedFaultQueue& myq){ 
    DPRINTF(FaultInjection, "copy me GeneralFetchInjectedFault\n");
    return (new GeneralFetchInjectedFault(*this) ); }
  /* Manifestation of the fault
   */
  TheISA::MachInst process(TheISA::MachInst inst);


  /* check if the fault should be rescheduled (intermittent, permanent faults) and reschedule 
   *
   */
  void check4reschedule();
	virtual void store(std::ofstream &os);
	
};

#endif // __GENERAL_FETCH_INJECTED_FAULT_HH__
