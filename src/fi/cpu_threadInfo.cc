#include <iostream>
#include <string>
#include <vector>
#include "fi/cpu_threadInfo.hh"
#include "cpu/o3/cpu.hh"
#include "fi/faultq.hh"
#include "fi/fi_system.hh"


cpuExecutedTicks:: cpuExecutedTicks(std:: string name)
{
  setName(name);
  setInstrExecuted(0);
  setTicksExecuted(0);
}

cpuExecutedTicks::cpuExecutedTicks(std::ifstream &os)
{
	int64_t temp;
	string t;
	os>>t;
	setName(t);
	os>>temp;
	setTicksExecuted(temp);
	os>>temp;
	setInstrExecuted(temp);
}


void
cpuExecutedTicks:: store(std::ofstream &os){
	os<<getName();
	os<<" ";
	os<<getTicksExecuted();
	os<<" ";
	os<<getInstrExecuted();
	os<<" ";
}


ThreadEnabledFault::ThreadEnabledFault(int threadId)
{
  setThreaId(threadId);
  setMyid();
  setRelative(false);
  setMagicInstTickCnt( -1);
  setMagicInstInstCnt(-1);
  setMagicInstVirtualAddr(-1);
  //findThreadFaults(threadId);
}

ThreadEnabledFault::ThreadEnabledFault(ifstream &os)
{
	int64_t temp;
	bool temp1;
	int i,size;
	string t;
	os>>temp;
	setThreaId(temp);
	os>>temp;
	setMagicInstInstCnt(temp);
	os>>temp;
	setMagicInstTickCnt(temp);
	os>>temp;
	setMagicInstVirtualAddr(temp);
	os>>temp1;
	setRelative(temp1);
	os>>temp;
	setMyid();
	os>>size;
	for(i = 0; i<size; i++){
		cores.push_back(new cpuExecutedTicks(os));
	}
}


void 
ThreadEnabledFault::store(std::ofstream &os){
	int i;
	os<<getThreaId();
	os<<" ";
	os<<getMagicInstInstCnt();
	os<<" ";
	os<<getMagicInstTickCnt();
	os<<" ";
	os<<getMagicInstVirtualAddr();
	os<<" ";
	os<<getRelative();
	os<<" ";
	os<<getMyId();
	os<<" ";
	for(i = 0 ; i < cores.size(); i++){
		cores[i]->store(os);
	}
}




void ThreadEnabledFault::findThreadFaults(int threadId){
  //  InjectedFault *p=mainInjectedFaultQueue.head;
   /* while(p){
      if(threadId == atoi((p->getThread()).c_str()))
	  threadMainInjectedFaultQueue.insert(p->copyme(threadMainInjectedFaultQueue));	
      p = p->nxt;
    }
    
    p=fetchStageInjectedFaultQueue.head;
    while(p){
      if(threadId == atoi((p->getThread()).c_str()))
	  threadFetchStageInjectedFaultQueue.insert(p->copyme(threadFetchStageInjectedFaultQueue));	
      p = p->nxt;
    }
    
    p=decodeStageInjectedFaultQueue.head;
    while(p){
      if(threadId == atoi((p->getThread()).c_str()))
	  threadDecodeStageInjectedFaultQueue.insert(p->copyme(threadDecodeStageInjectedFaultQueue));	
      p = p->nxt;
    }
    
    p=iewStageInjectedFaultQueue.head;
    while(p){
      if(threadId == atoi((p->getThread()).c_str()))
	  threadIewStageInjectedFaultQueue.insert(p->copyme(threadIewStageInjectedFaultQueue));	
      p = p->nxt;
    }
    */
   // dump();
}


void ThreadEnabledFault::executed_relative_instr(){
  int i;
  for(i  = 0 ; i < cores.size() ; i++){
    cores[i]->setInstrExecuted(0);
    cores[i]->setTicksExecuted(0);
  }
    
}


void ThreadEnabledFault::dump(){

  if (DTRACE(FaultInjection)) {
    std::cout<<"================\t"<<"ThreadEnabledFault "<<getMyId()<<" \t==========================\n"; 
    std::cout << "ThreadEnabledInfo InstrCnt:" <<getMagicInstInstCnt() <<" TickCnt: "<< getMagicInstTickCnt() <<" MagicInstVirtualAddr : "<<getMagicInstVirtualAddr()<<" ThreadId :"<<getThreaId() <<"\n";
    std::cout<<"================\t~ThreadEnabledFault~\t==========================\n";
  }
}

int ThreadEnabledFault:: increaseExecutedTicks(std:: string curCpu, int64_t ticks)
{
  int i;
  for(i  = 0 ; i < cores.size() ; i++){
    if(!((cores[i]->getName()).compare(curCpu))){
      cores[i]->increaseTicks(ticks); 
      return i;
    }
  }
  DPRINTF(FaultInjection, "===increaseExecutedTime %s===\n",curCpu);
  cores.push_back(new cpuExecutedTicks(curCpu));
  return i;
}


int ThreadEnabledFault:: increaseExecutedInstr(std:: string curCpu)
{
  int i;
  for(i  = 0 ; i < cores.size() ; i++){
    if(!((cores[i]->getName()).compare(curCpu))){
      cores[i]->increaseInstr();
      return i;
    }
  }
  DPRINTF(FaultInjection, "===increaseExecutedTime %s===\n",curCpu);
  cores.push_back(new cpuExecutedTicks(curCpu));
  return i;
}


void ThreadEnabledFault:: CalculateExecutedTime(std::string curCpu , int64_t *exec_time , uint64_t *exec_instr )
{
  int i;
  *exec_time = 0;
  *exec_instr = 0;
  for(i  = 0 ; i < cores.size() ; i++){
    if((curCpu.compare(cores[i]->getName()))==0);{
      *exec_time = cores[i]->getTicksExecuted();
      *exec_instr = cores[i]->getInstrExecuted();
      return;
    }
    if(curCpu.compare("all")==0){
      *exec_time  +=  	cores[i]->getTicksExecuted();
      *exec_instr +=  cores[i]->getInstrExecuted();
    }
  }
}
