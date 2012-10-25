#ifndef __REGISTER_INJECTED_FAULT_HH__
#define __REGISTER_INJECTED_FAULT_HH__

#include "fi/faultq.hh"
#include "fi/cpu_injfault.hh"
#include "fi/fi_system.hh"

class RegisterInjectedFaultParams;

class RegisterInjectedFault : public CPUInjectedFault
{

protected:
  //Type of register to be injected with fault
  typedef short RegisterType;
  static const RegisterType IntegerRegisterFault = 1;
  static const RegisterType FloatRegisterFault   = 2;
  static const RegisterType MiscRegisterFault    = 3;

private:
  /*register faults can be scheduled as events at the intrinsic event queues of M5
   */
  class RegisterInjectedFaultEvent : public Event
  {
  private:
    /*pointer to the initial fault object*/
    RegisterInjectedFault *fault;
    /*pointer to the fault queue that the fault was scheduled*/
    InjectedFaultQueue *faultq;

  public:
    RegisterInjectedFaultEvent(InjectedFaultQueue *q, RegisterInjectedFault *f);
    void process();
    const char *description() const;
  };

  RegisterInjectedFaultEvent RegInjFaultEvent;
  
private:
  
  /* The number of the register that will be corrupted
   */
  int _register;
  /* The type of the register that will be corrupted (int, float, misc)
   */
  RegisterType _regType;

  /* The setXXX functions are used to assign values at the above described variable
   */
  void setRegister(int v) { _register = v;}
  void setRegType(RegisterType v){_regType = v;}
  
  void setRegType(std::string v) 
  { 
    if (v.compare("int") == 0) {
      _regType = RegisterInjectedFault::IntegerRegisterFault;
    }
    else if (v.compare("float") == 0) {
      _regType = RegisterInjectedFault::FloatRegisterFault;
    }
    else if (v.compare("misc") == 0) {
      _regType = RegisterInjectedFault::MiscRegisterFault;
    }
    else {
      std::cout << "RegisterInjectedFault::setRegType() -- Error parsing setRegType()\n";
      assert(0);
    }
  }

public:
  typedef RegisterInjectedFaultParams Params;
  
  const Params *params() const
  {
    return reinterpret_cast<const Params *>(_params); 
  }
  
  RegisterInjectedFault(Params *params);
  RegisterInjectedFault(RegisterInjectedFault &source,InjectedFaultQueue& myq);
	RegisterInjectedFault(Params *p, std::ifstream &os);
  ~RegisterInjectedFault();

  const std::string name() const;
  virtual const char *description() const;
  
  void dump() const;
  
  virtual void init();
  virtual void startup();
  virtual Port* getPort(const std::string &if_name, int idx = 0);
  RegisterInjectedFault* copyme(InjectedFaultQueue& myq){ 
    DPRINTF(FaultInjection,"COPY ME RegisterInjectedFaultParams\n ");
    return (new RegisterInjectedFault(*this,myq) ); 
  }

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

  /* The getXXX functions are a compliment to the setXXX functions and are used to get the values of the described variable
   */
  int getRegister() const { return _register;}
  RegisterType getRegType() const { return _regType;}
	virtual void store(std::ofstream &os);
};

#endif // __REGISTER_INJECTED_FAULT_HH__
