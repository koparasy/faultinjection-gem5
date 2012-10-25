#ifndef __REGISTER_DECODING_INJECTED_FAULT_HH__
#define __REGISTER_DECODING_INJECTED_FAULT_HH__

#include "config/the_isa.hh"
#include "base/types.hh"
#include "fi/faultq.hh"
#include "fi/o3cpu_injfault.hh"
#include "cpu/o3/cpu.hh"
#include "params/RegisterDecodingInjectedFault.hh"
#include "fi/fi_system.hh"

class RegisterDecodingInjectedFaultParams;

class RegisterDecodingInjectedFault : public O3CPUInjectedFault
{
protected:
  typedef short RegisterDecodingInjectedFaultType;
  static const RegisterDecodingInjectedFaultType SrcRegisterInjectedFault = 1;
  static const RegisterDecodingInjectedFaultType DstRegisterInjectedFault = 2;


private:
  RegisterDecodingInjectedFaultType _srcOrDst;
  int _regToChange;
  int _changeToReg;

  int parseRegDec(std::string s);
  void setSrcOrDst(RegisterDecodingInjectedFaultType v) { _srcOrDst = v;}
  void setRegToChange(std::string v) { _regToChange = atoi(v.c_str());}
  void setChangeToReg(std::string v) { _changeToReg = atoi(v.c_str());}
  void setChangeToReg(int v){_changeToReg = v;}
  void setRegToChange(int v) {_regToChange =v;}


public:

  typedef RegisterDecodingInjectedFaultParams Params;
  
  const Params *params() const
  {
    return reinterpret_cast<const Params *>(_params); 
  }

  

  RegisterDecodingInjectedFault(Params *params);
  RegisterDecodingInjectedFault(RegisterDecodingInjectedFault &source);
	RegisterDecodingInjectedFault(Params *p, std::ifstream &os);
  ~RegisterDecodingInjectedFault();
  const std::string name() const;
  RegisterDecodingInjectedFault* copyme(InjectedFaultQueue& myq){ 
    DPRINTF(FaultInjection,"COPY ME RegisterDecodingInjectedFault\n ");
    return (new RegisterDecodingInjectedFault(*this) ); 
  }
  virtual const char *description() const;
  
  void dump() const;

  virtual void init();
  virtual void startup();
  
  StaticInstPtr process(StaticInstPtr inst);
  void check4reschedule();
    
  RegisterDecodingInjectedFaultType
  getSrcOrDst() const { return _srcOrDst;}
  int 
  getRegToChange() const { return _regToChange;}
  int 
  getChangeToReg() const { return _changeToReg;}

	virtual void store(std::ofstream &os);
};

#endif // __REGISTER_DECODING_INJECTED_FAULT_HH__
