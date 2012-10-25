#ifndef __PC_INJECTED_FAULT_HH__
#define __PC_INJECTED_FAULT_HH__

#include "fi/faultq.hh"
#include "fi/cpu_injfault.hh"

class PCInjectedFaultParams;

class PCInjectedFault : public CPUInjectedFault
{
private:
  /*PC faults can be scheduled as events at the intrinsic event queues of M5
   */
  class PCInjectedFaultEvent : public Event
  {
  private:
    /*pointer to the initial fault object*/
    PCInjectedFault *fault;
    /*pointer to the fault queue that the fault was scheduled*/
    InjectedFaultQueue *faultq;

  public:
    PCInjectedFaultEvent(InjectedFaultQueue *q, PCInjectedFault *f);
    void process();
    const char *description() const;
  };

  PCInjectedFaultEvent PCInjFaultEvent;
  
  
public:
  typedef PCInjectedFaultParams Params;
  
  const Params *params() const
  {
    return reinterpret_cast<const Params *>(_params); 
  }
   
  PCInjectedFault(Params *params);
  PCInjectedFault(PCInjectedFault &source,InjectedFaultQueue& myq);
	PCInjectedFault(Params *p, std::ifstream &os);
  ~PCInjectedFault();

  const std::string name() const;  
  virtual const char *description() const;
  
  void dump() const;
  PCInjectedFault* copyme(InjectedFaultQueue& myq){ 
     DPRINTF(FaultInjection,"copy : PCInjectedFault\n");
    return (new PCInjectedFault(*this,myq) ); 
    
  }
  virtual void init();
  virtual void startup();
  virtual Port* getPort(const std::string &if_name, int idx = 0);
  
  /* Used to schedule the fault into an event queue of the M5
   * remove: if the fault should be removed from the fault queue or not
   */
  void schedule(bool remove);

  /* check if the fault should be rescheduled (intermittent, permanent faults) and reschedule 
   *
   */
  void check4reschedule();

  /* Manifestation of the fault
   */
  int process();
	virtual void store(std::ofstream &os);
};

#endif // __PC_INJECTED_FAULT_HH__
