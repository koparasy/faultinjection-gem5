#ifndef __CPU_THREAD_FAULT_INFO__
#define __CPU_THREAD_FAULT_INFO__




#include "config/full_system.hh"
#include "config/the_isa.hh"
#include "base/types.hh"
#include "arch/types.hh"
#include "base/trace.hh"
#include "debug/FaultInjection.hh"
#include "fi/faultq.hh"
#include "fi/fi_system.hh"
#include "mem/mem_object.hh"
#include "params/InjectedFault.hh"


class cpuExecutedTicks ; //forward declaration
class ThreadEnabledFault; 



class InjectedFaultQueue;

class cpuExecutedTicks {
  private:
    int64_t instrExexuted;
    int64_t ticksExecuted;
    std::string _name;
  public:
    
    cpuExecutedTicks(std:: string name);
		cpuExecutedTicks(std:: ifstream &os);
    ~cpuExecutedTicks();
    
    void setName(std:: string v){_name = v;}
    void setInstrExecuted(int64_t v){instrExexuted = v;}
    void setTicksExecuted(int64_t v){ticksExecuted = v ;}
    
    int64_t getInstrExecuted(){return instrExexuted;}
    int64_t getTicksExecuted() {return ticksExecuted;}
    std::string getName() {return _name;}
    
    void increaseTicks(int64_t ticks) {ticksExecuted +=ticks;}
    void increaseInstr() {instrExexuted++;}
    
    void store(std::ofstream &os);
};




class ThreadEnabledFault {
  friend class InjectedFault;
  private :
    int64_t MagicInstInstCnt;
    int64_t MagicInstTickCnt;
    Addr MagicInstVirtualAddr;
    bool Relative;
    int threadId;
    int myId;
  protected :
    std::vector<cpuExecutedTicks*> cores; 
   
  public:
//    InjectedFaultQueue threadMainInjectedFaultQueue;
//    InjectedFaultQueue threadFetchStageInjectedFaultQueue;
//    InjectedFaultQueue threadDecodeStageInjectedFaultQueue;
//    InjectedFaultQueue threadIewStageInjectedFaultQueue;
    
    
    ThreadEnabledFault( int threadId );
		ThreadEnabledFault(std::ifstream &os);
    ~ThreadEnabledFault();
  
    void setMagicInstInstCnt(int64_t v){ MagicInstInstCnt  = v; }
    void setMagicInstTickCnt(int64_t v){ MagicInstTickCnt  = v; }
    void setMagicInstVirtualAddr(Addr v){ MagicInstVirtualAddr  = v; }
    void setThreaId(int v){ threadId  = v; }
    void setRelative(bool v){Relative = v;}
    void setMyid(){
	static int my_id_counter = 0;
      myId = my_id_counter++;
    }
    void setMyid(int64_t v);
/*	{
			myId = v;
			if(myId>my_id_counter)
				my_id_counter= v;
		}*/
    int getMyId(){return myId ;}
    int64_t getMagicInstInstCnt() { return MagicInstInstCnt; }
    int64_t getMagicInstTickCnt() { return MagicInstTickCnt; }
    Addr getMagicInstVirtualAddr() { return MagicInstVirtualAddr; }
    int getThreaId(){ return threadId; }
    bool getRelative(){ return Relative; }
    
    InjectedFault *copyFault(InjectedFault &source);
    void dump();
    
    void findThreadFaults(int ThreadsId);
    int increaseExecutedTicks(std :: string curCpu , int64_t ticks);
    int increaseExecutedInstr(std:: string curCpu);
    void CalculateExecutedTime(std:: string curCpu,int64_t *exec_time , uint64_t *exec_instr);
    void executed_relative_instr();
    
		
		void store(std::ofstream &os);
};

#endif //  __CPU_THREAD_FAULT_INFO__
