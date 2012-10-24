#ifndef __MEM_INJECTED_FAULT_HH__
#define __MEM_INJECTED_FAULT_HH__

#include "mem/physical.hh"
#include "fi/faultq.hh"

class MemoryInjectedFaultParams;

class MemoryInjectedFault : public InjectedFault
{
private:
  /*memory faults can be scheduled as events at the intrinsic event queues of M5
   */
  class MemoryInjectedFaultEvent : public Event
  {
  private:
    /*pointer to the initial fault object*/
    MemoryInjectedFault *fault;
    /*pointer to the fault queue that the fault was scheduled*/
    InjectedFaultQueue *faultq;

  public:
    MemoryInjectedFaultEvent(InjectedFaultQueue *q, MemoryInjectedFault *f);
    void process();
    const char *description() const;
  };

  MemoryInjectedFaultEvent MemInjFaultEvent;
  

public:
  typedef MemoryInjectedFaultParams Params;
  
  const Params *params() const
  {
    return reinterpret_cast<const Params *>(_params); 
  }
  
  /* Memory address location that will be corrupted*/
  Addr _address;
  /* Pointer to the memory simulation object in which the fault will be injected*/
  PhysicalMemory *pMem;

  MemoryInjectedFault(Params *params);
  MemoryInjectedFault(MemoryInjectedFault &source , InjectedFaultQueue& myq);
	MemoryInjectedFault(Params *p,std::ifstream &os);
  ~MemoryInjectedFault();

  const std::string name() const; 
  virtual const char *description() const;
  
  void dump() const;
  MemoryInjectedFault* copyme(InjectedFaultQueue& myq){ 
    DPRINTF(FaultInjection, "copy me MemoryInjectedFault\n");
    return (new MemoryInjectedFault(*this,myq) ); }
  virtual void init();
  virtual void startup();
  virtual Port* getPort(const std::string &if_name, int idx = 0);
   void setAddress(Addr v){ _address = v ;}
   Addr getAddress(){return _address;};
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
protected:
	virtual void store(std::ofstream &os);
  
};

#endif // __MEM_INJECTED_FAULT_HH__
