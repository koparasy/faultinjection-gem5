#ifndef __FI_RELATIVE__
#define __FI_RELATIVE__


#include <iostream>

#include "fi/faultq.hh"
#include "fi/cpu_threadInfo.hh"
#include "fi/fi_system.hh"
using namespace std;

class FI_Inst;//forward declaration

extern FI_Inst fi_inst;

class FI_Inst
{
public:
  void 
  fi_activate_inst(Addr PCBAddr, uint64_t instCnt)
  {  
    DPRINTF(FaultInjection, "===Fault Injection Activation Instruction===\n");
    DPRINTF(FaultInjection, "\tFetched Instructions: %llu\n", instCnt);
    DPRINTF(FaultInjection, "\tSimulation Ticks: %llu\n", curTick());
    
    
    DPRINTF(FaultInjection, "\t Process Control Block(PCB) Addressx: %llx\n", PCBAddr);
    
    fi_activation_iter = fi_activation.find(PCBAddr);
    if (fi_activation_iter == fi_activation.end()) { //insert new
      fi_activation[PCBAddr] = true;
    } else { //flip old
      fi_activation[PCBAddr] = !fi_activation_iter->second;
    }
    
    fi_active = !fi_active;
    
    //mainInjectedFaultQueue.dump();
    
    DPRINTF(FaultInjection, "~==Fault Injection Activation Instruction===\n");
  }

  void
  fi_relative_inst(std::string name, Addr PCAddr, uint64_t instCnt, uint64_t ticks)
  {
    MagicInstVirtualAddr = PCAddr;
    MagicInstInstCnt = instCnt;
    MagicInstTickCnt = ticks;
    
    if (DTRACE(FaultInjection)) {
      std::cout << "===Fault Injection Relative Point Instruction===\n";
      std::cout << "\tMagicInstVirtualAddr: " <<  MagicInstVirtualAddr << "\n";
      std::cout << "\tMagicInstInstCnt:" << MagicInstInstCnt << "\n";
      std::cout << "\tMagicInstTickCnt:" << MagicInstTickCnt << "\n";
      std::cout << "~==Fault Injection Relative Point Instruction===\n";
    }
    
    //Schedule remaining faults that can be scheduled
   // mainInjectedFaultQueue.scheduleRelativeFaults(name);
  }

};

#endif //__FI_RELATIVE__
