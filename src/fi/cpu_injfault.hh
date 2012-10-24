#ifndef __CPU_INJECTED_FAULT_HH__
#define __CPU_INJECTED_FAULT_HH__

#include "fi/faultq.hh"
#include "cpu/base.hh"
#include "params/CPUInjectedFault.hh"


#include <iostream>
#include <fstream>


class CPUInjectedFaultParams;

class CPUInjectedFault : public InjectedFault
{
private:
  /*Pointer to the CPU object that the fault is meant to be injected
   */
  BaseCPU *_cpu;
  /* Number of thread context that the fault is intended for
   */
  int _tcontext;
  

  /* The setXXX functions are used to assign values at the above described variable
   */
  void setCPU(BaseCPU *v) { _cpu = v;}
  void setTContext(int v) { _tcontext = v;}

public:

  typedef CPUInjectedFaultParams Params;
  
  const Params *params() const
  {
    return reinterpret_cast<const Params *>(_params); 
  }

  CPUInjectedFault(Params *params);
  CPUInjectedFault(CPUInjectedFault &source);
	CPUInjectedFault(Params *p,  ifstream &os);
  ~CPUInjectedFault();

  const std::string name() const;
  virtual const char *description() const;
 
  /*Print the faults variable values
   */
  void dump() const;

  virtual void init();
  virtual void startup();
  virtual Port* getPort(const std::string &if_name, int idx = 0);
  CPUInjectedFault* copyme(InjectedFaultQueue &myq ){ 
    DPRINTF(FaultInjection, "copy me InjectedFault\n"); 
    return (new CPUInjectedFault(*this) ); }

  /* This function is called for the fault to manifest the implementation of each fault type should be seen for
   * a better insight on the matter.
   */
  virtual int process(){ assert(0);return 0;};


  /* The getXXX functions are a compliment to the setXXX functions and are used to get the values of the described variable
   */
  BaseCPU *
  getCPU() const { return _cpu;} 
  int
  getTContext() const { return _tcontext;}
  
  
protected:
	virtual void store(std::ofstream &os);
	
};

#endif // __CPU_INJECTED_FAULT_HH__
