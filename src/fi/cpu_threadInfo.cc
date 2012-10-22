#include <iostream>
#include <string>
#include <vector>
#include "fi/cpu_threadInfo.hh"
#include "cpu/o3/cpu.hh"
#include "fi/faultq.hh"

std::vector<ThreadEnabledFault*> threadList; 
std::vector <cpuExecutedTicks * > coresCount; 
int cores = -1;
/*
ThreadEnabledFault( params *p )
{
  cores = p->cores;
  for(vectorpos = 0 ; vectorpos < cores ; vectorpos++){
    threadList.insert(new ThreadEnabledFault(-1));
    (threadList[i]->cores).insert("DUMMY");
  }
  
}
*/

int increase_instr_executed(std:: string curCpu , ThreadEnabledFault *curThread){
   char *dummy= (char*)(curCpu.c_str());
   dummy = dummy + 10;
   int cpuindex = atoi(dummy);     // the following code is to keep the logistics of the cores and the threads
   coresCount[cpuindex]->increaseInstr();
    if(curThread)
      curThread->increaseExecutedInstr(curCpu);
    return (1);
}


int increase_fi_counters(std :: string curCpu , ThreadEnabledFault *curThread, int64_t ticks){
    char *dummy= (char*)(curCpu.c_str());
    
    dummy = dummy + 10;
    int cpuindex = atoi(dummy);
    
    if(curThread)
      curThread->increaseExecutedTicks(curCpu,ticks);
    else if( coresCount.size() > cpuindex)
      coresCount[cpuindex]->increaseTicks(ticks);     // the following code is to keep the logistics of the cores and the threads
   
    return (1);
}



int get_fi_counters( InjectedFault *p , ThreadEnabledFault &thread,std::string curCpu , int64_t *exec_time , uint64_t *exec_instr ){
  int i;
  char *dummy= (char*)(curCpu.c_str());
  dummy = dummy + 10;
  int cpuindex = atoi(dummy);
//  if(p->getRelative()){
    if((p->getWhere().compare(curCpu))==0 && (p->getThread()).compare("all") != 0 && thread.getThreaId() == atoi( (p->getThread()).c_str() ) ){ // case thread_id - cpu_id
	thread.CalculateExecutedTime(curCpu,exec_time,exec_instr);
	return(1);
    }
    else if((p->getWhere().compare("all") == 0) && (p->getThread()).compare("all") != 0 && thread.getThreaId() == atoi( (p->getThread()).c_str() )){// case thread_id - all
	thread.CalculateExecutedTime("all",exec_time,exec_instr);
	if(p->getFaultType() == p->RegisterInjectedFault || p->getFaultType() == p->PCInjectedFault ){
	    p->setCPU(reinterpret_cast<BaseCPU *>(p->find(curCpu.c_str())));
//	    DPRINTF(FaultInjection, "===setCore ===\n");
	}
	else if(p->getFaultType() == p->GeneralFetchInjectedFault || p->getFaultType() == p->OpCodeInjectedFault ||
		p->getFaultType() == p->RegisterDecodingInjectedFault || p->getFaultType() == p->ExecutionInjectedFault){
	    p->setCPU(reinterpret_cast<BaseO3CPU *>(p->find(curCpu.c_str())));
//	      DPRINTF(FaultInjection, "===setCore ===\n");
	}
	return(2);      
    }
    else if( ( (p->getWhere().compare(curCpu)) == 0  ) && (((p->getThread()).compare("all")) == 0)  ){ //case cpu_id - all
      *exec_instr = coresCount[cpuindex]->getInstrExecuted();
      *exec_time  = coresCount[cpuindex]->getTicksExecuted();
      return(3);
    }
    else if( ((p->getThread()).compare("all") == 0) && ((p->getWhere().compare("all")) == 0) ){ //case all - all
      *exec_instr = 0;
      *exec_time  = 0;
      for( i = 0 ; i < cores ; i++){
	*exec_instr += coresCount[i]->getInstrExecuted();
	*exec_time  += coresCount[i]->getTicksExecuted();
      }
      if(p->getFaultType() == p->RegisterInjectedFault || p->getFaultType() == p->PCInjectedFault ){
	  p->setCPU(reinterpret_cast<BaseCPU *>(p->find(curCpu.c_str())));
      }
      else if(p->getFaultType() == p->GeneralFetchInjectedFault || p->getFaultType() == p->OpCodeInjectedFault ||
	      p->getFaultType() == p->RegisterDecodingInjectedFault || p->getFaultType() == p->ExecutionInjectedFault){
	  p->setCPU(reinterpret_cast<BaseO3CPU *>(p->find(curCpu.c_str())));
      }
      return(4);
    }
/*  }
  else{
    if(p->getWhere().compare("all") == 0){
	*exec_instr = 0;
	*exec_time  = 0;
	for( i = 0 ; i < cores ; i++){
	  *exec_instr += coresCount[i]->getInstrExecuted();
	  *exec_time  += coresCount[i]->getTicksExecuted();
	}
	if(p->getFaultType() == p->RegisterInjectedFault || p->getFaultType() == p->PCInjectedFault ){
	    p->setCPU(reinterpret_cast<BaseCPU *>(p->find(curCpu.c_str())));
	}
	else if(p->getFaultType() == p->GeneralFetchInjectedFault || p->getFaultType() == p->OpCodeInjectedFault ||
		p->getFaultType() == p->RegisterDecodingInjectedFault || p->getFaultType() == p->ExecutionInjectedFault){
	    p->setCPU(reinterpret_cast<BaseO3CPU *>(p->find(curCpu.c_str())));
	}
	return(5);
    }
    else if(p->getWhere().compare(curCpu) == 0){
      *exec_instr = coresCount[cpuindex]->getInstrExecuted();
      *exec_time  = coresCount[cpuindex]->getTicksExecuted();
      return(6);
    }
  }
*/
    return 0;
}



cpuExecutedTicks:: cpuExecutedTicks(std:: string name)
{
  setName(name);
  setInstrExecuted(0);
  setTicksExecuted(0);
}

ThreadEnabledFault::ThreadEnabledFault(int threadId)
//  threadMainInjectedFaultQueue("Main Fault Queue"),
// threadFetchStageInjectedFaultQueue("Fetch Stage Fault Queue"),
//  threadDecodeStageInjectedFaultQueue("Decode Stage Fault Queue"),
//  threadIewStageInjectedFaultQueue("IEW Stage Fault Queue")
{
  setThreaId(threadId);
  setMyid();
  setRelative(false);
  setMagicInstTickCnt( -1);
  setMagicInstInstCnt(-1);
  setMagicInstVirtualAddr(-1);
  //findThreadFaults(threadId);
  
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