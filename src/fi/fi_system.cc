#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "cpu/o3/cpu.hh"
#include "fi/faultq.hh"
#include "fi/cpu_threadInfo.hh"
#include "fi/fi_system.hh"
using namespace std;

Fi_System *fi_system;

Fi_System:Fi_system(Params *p)
:MemObject(p);
{
  
  input = p->in_name;
  output = p->out_name;
  
  fi_active = 0;
  vectorpos = 0;
  
  MagicInstVirtualAddr = 0;
  MagicInstInstCnt=0;
  MagicInstTickCnt=0;
  
  cores = -1;
  
  input.open (in_name, ifstream::in);
  getFromFile(p,input);
  input.close();
  
  fi_system = this;
  
  output.open(out_name,ofstream::trunc);
  
}

void
Fi_System:: getFromFile(Params *p, std::ifstream &os){
	string check;
	Addr _tmpAddr;
	int val;
	while(os.good()){
		os>>check;
		if(check.compare("CORES") == 0){
			os>>cores;
		}
		else if(check.compare("CORE:") == 0){
			coresCount.push_back(new cpuExecutedTicks(os));
		}
		else if(check.compare("ThreadEnabledFault") == 0){
			os>>_tmpAddr;
			fi_activation_iter = fi_activation.find(_tmpAddr);
		  if (fi_activation_iter == fi_activation.end()) { //insert new
		    fi_activation[_tmpAddr] = vectorpos;
				os>>val;
		    threadList.push_back(new ThreadEnabledFault(os));
		    (*(threadList[ vectorpos ] )).dump();
		    vectorpos++;
		    fi_active++;
		  }
		  else{
				DPRINTF(FaultInjection,"&&&&&&&&&&&&THIS SHOULD NEVER HAPPEN1 %d %llx\n",val,_tmpAddr);
			}
			if(val!=vectorpos-1){
				DPRINTF(FaultInjection,"&&&&&&&&&&&&THIS SHOULD NEVER HAPPEN2 %d %llx\n",val,_tmpAddr);
			}
		}
		else if(check.compare("CPUInjectedFault") ==0){
			new CPUInjectedFault(p,os);
		}
		else if(check.compare("InjectedFault") ==0){
			new InjectedFault(p,os);
		}
		else if(check.compare("GeneralFetchInjectedFault") ==0){
			new GeneralFetchInjectedFault(p,os);
		}
		else if(check.compare("IEWStageInjectedFault") ==0){
			new IEWStageInjectedFault(p,os);
		}
		else if(check.compare("MemoryInjectedFault") ==0){
			new MemoryInjectedFault(p,os);
		}
		else if(check.compare("O3CPUInjectedFault") ==0){
			new O3CPUInjectedFault(p,os);
		}
		else if(check.compare("OpCodeInjectedFault") ==0){
			new OpCodeInjectedFault(p,os);
		}
		else if(check.compare("PCInjectedFault") ==0){
			new PCInjectedFault(p,os);
		}
		else if(check.compare("RegisterInjectedFault") ==0){
			new RegisterInjectedFault(p,os);
		}
		else if(check.compare("RegisterDecodingInjectedFault") ==0){
			new RegisterDecodingInjectedFault(p,os);
		}
		else{
		  DPRINTF(FaultInjection,"THIS SHOULD NEVER HAPPEN3\n");
		}
	}
}

void
Fi_System:: storeToFile(std::ofstream &os)
{	
	int i;
	os<<"CORES ";
	os<<cores;
	os<<"\n";
	for( i = 0; i< coresCount.size(); i++){
		os<<"CORE: ";
		(*coresCount[i]).store(os);
		os<<"\n";
	}
	
	
	for(fi_activation_iter = fi_activation.begin(); fi_activation_iter != fi_activation.end(); ++fi_activation_iter){
		os<<"ThreadEnabledFault ";
		os<<fi_activation_iter->first ;
		os<<" ";
		os << fi_activation_iter->second ;
		os<<" ";
		(*threadList[fi_activation_iter->second]).store(os);
		os<<"\n";
	}
	
	
	//store all faults  in the FILE
	InjectedFault *p=mainInjectedFaultQueue.head;
	while(p){
		os<<p->description();
		os<<" ";
		p->store(os);
		os<<"\n";
	}
	
	p=fetchStageInjectedFaultQueue.head;
	while(p){
		os<<p->description();
		os<<" ";
		p->store(os);
		os<<"\n";
	}
	
	p=decodeStageInjectedFaultQueue.head;
	while(p){
		os<<p->description();
		os<<" ";
		p->store(os);
		os<<"\n";
	}
	
	p=iewStageInjectedFaultQueue.head;
	while(p){
		os<<p->description();
		os<<" ";
		p->store(os);
		os<<"\n";
	}
	
	
	
}


int
Fi_System:: increase_instr_executed(std:: string curCpu , ThreadEnabledFault *curThread){
   char *dummy= (char*)(curCpu.c_str());
   dummy = dummy + 10;
   int cpuindex = atoi(dummy);     // the following code is to keep the logistics of the cores and the threads
   coresCount[cpuindex]->increaseInstr();
    if(curThread)
      curThread->increaseExecutedInstr(curCpu);
    return (1);
}


int
Fi_System:: increase_fi_counters(std :: string curCpu , ThreadEnabledFault *curThread, int64_t ticks){
    char *dummy= (char*)(curCpu.c_str());
    
    dummy = dummy + 10;
    int cpuindex = atoi(dummy);
    
    if(curThread)
      curThread->increaseExecutedTicks(curCpu,ticks);
    else if( coresCount.size() > cpuindex)
      coresCount[cpuindex]->increaseTicks(ticks);     // the following code is to keep the logistics of the cores and the threads
   
    return (1);
}



int 
Fi_System:: get_fi_counters( InjectedFault *p , ThreadEnabledFault &thread,std::string curCpu , int64_t *exec_time , uint64_t *exec_instr ){
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
}