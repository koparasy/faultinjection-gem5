#ifndef __IEW_STAGE_INJECTED_FAULT_HH__
#define __IEW_STAGE_INJECTED_FAULT_HH__

#include "base/types.hh"
#include "fi/faultq.hh"
#include "cpu/o3/cpu.hh"
#include "fi/o3cpu_injfault.hh"
#include "params/IEWStageInjectedFault.hh"

class IEWStageInjectedFaultParams;

class IEWStageInjectedFault : public O3CPUInjectedFault
{

public:

  typedef IEWStageInjectedFaultParams Params;
  
  const Params *params() const
  {
    return reinterpret_cast<const Params *>(_params); 
  } 

  IEWStageInjectedFault(Params *params);
  IEWStageInjectedFault(IEWStageInjectedFault &source);
	IEWStageInjectedFault(Params *p, std::ifstream &os);
  ~IEWStageInjectedFault();

  const std::string name() const;  
  virtual const char *description() const;
  
  void dump() const;

  virtual void init();
  virtual void startup();
  IEWStageInjectedFault* copyme(InjectedFaultQueue& myq){ 
    DPRINTF(FaultInjection, "copy me IEWStageInjectedFault\n");
    return (new IEWStageInjectedFault(*this) ); }

  // did not create template due to prints
  /*
  virtual void process(void);
  virtual uint64_t process(uint64_t v);
  virtual float process(float v);
  virtual double process(double v);
  */

  template <class T>
  void
  process(void)
  { 
    std::cout << "IEWStageInjectedFault::process(void) -- Should never be called\n";
    getQueue()->remove(this);  
    assert(0);
  }
 
  template <class T> T
  process(T v)
  { 
    T retVal = v;
    
    DPRINTF(FaultInjection, "===IEWStageInjectedFault::process(T)===\n");
    
#ifdef ALPHA_ISA
  retVal = manifest(v, getValue(), getValueType());
#endif
#ifndef ALPHA_ISA
    assert(0);
#endif
    
    check4reschedule();
    
    DPRINTF(FaultInjection, "~==IEWStageInjectedFault::process(T)===\n");
    return retVal;
  }


  void check4reschedule();
  //Port* getPort(const std::string &if_name, int idx = 0);
	virtual void store(std::ofstream &os);
};

#endif // __IEW_STAGE_INJECTED_FAULT_HH__
