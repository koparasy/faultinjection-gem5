#ifndef _FI_SYSTEM

#include <map>
#include <utility> 
#include <iostream>
#include <fstream>

#include "config/full_system.hh"
#include "config/the_isa.hh"
#include "base/types.hh"
#include "arch/types.hh"
#include "base/trace.hh"
#include "debug/FaultInjection.hh"

class Fi_System;
extern Fi_System *fi_system;


class Fi_System : public MemObject{
public :
    ofstream ouput;
    ifstream input;
    string in_name;
    string out_name;
    
    InjectedFaultQueue mainInjectedFaultQueue("Main Fault Queue");
    InjectedFaultQueue fetchStageInjectedFaultQueue("Fetch Stage Fault Queue");
    InjectedFaultQueue decodeStageInjectedFaultQueue("Decode Stage Fault Queue");	
    InjectedFaultQueue iewStageInjectedFaultQueue("IEW Stage Fault Queue");
    
    std::map<Addr, int> fi_activation;
    std::map<Addr, int>::iterator fi_activation_iter;

    int fi_active;
    int vectorpos;
    
    Addr MagicInstVirtualAddr;

    uint64_t MagicInstInstCnt;

    int64_t  MagicInstTickCnt;
    
     std::vector <ThreadEnabledFault * > threadList;
     std::vector <cpuExecutedTicks * > coresCount; 
     int cores;
public: 
  Fi_system(Params *p);
  ~Fi_system();
  
  
  int increase_fi_counters(std :: string curCpu , ThreadEnabledFault *curThread , int64_t ticks);
  int increase_instr_executed(std:: string curCpu , ThreadEnabledFault *curThread);
  int get_fi_counters(InjectedFault *p , ThreadEnabledFault &thread,std::string curCpu , int64_t *exec_time , uint64_t *exec_instr );
  void storeToFile(std::ofstream &os);
  void getFromFile(Params *p, std::ifstream &os);
  
     
}



#endif //_FI_SYSTEM