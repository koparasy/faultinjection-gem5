
#include <iostream>
#include <string>
#include <vector>
#include "cpu/o3/cpu.hh"
#include "fi/faultq.hh"
#include "fi/cpu_threadInfo.hh"
using namespace std;

InjectedFaultQueue mainInjectedFaultQueue("Main Fault Queue");
InjectedFaultQueue fetchStageInjectedFaultQueue("Fetch Stage Fault Queue");
InjectedFaultQueue decodeStageInjectedFaultQueue("Decode Stage Fault Queue");
InjectedFaultQueue iewStageInjectedFaultQueue("IEW Stage Fault Queue");


std::map<Addr, int> fi_activation;
std::map<Addr, int>::iterator fi_activation_iter;

int fi_active = 0;
int vectorpos = 0;
 

Addr MagicInstVirtualAddr = 0;
uint64_t MagicInstInstCnt = 0;
int64_t  MagicInstTickCnt = 0;




InjectedFault::InjectedFault(InjectedFault& source):MemObject(source.params()),manifested(false),nxt(NULL),prv(NULL){
  
  setFaultID(source.getFaultID());
  setWhen(source.getWhen());
  setWhere(source.getWhere());
  setWhat(source.getWhat());
  setRelative(source.getRelative());
  setThread(source.getThread());
  setOccurrence(source.getOccurrence());
  
  int ret;
  ret = parseWhen(source.getWhen());
  if (ret != 0) {
    std::cout << "InjectedFault::InjectedFault() -- Error while parsing When\n";
    assert(0);
  }
  ret = parseWhat(source.getWhat());
  if (ret != 0) {
    std::cout << "InjectedFault::InjectedFault() -- Error while parsing What\n";
    assert(0);
  }
  dump();
  
}


InjectedFault::InjectedFault(Params *p,fstream &os)
	:MemObject(p)
{
	std:: string _when, _what, _thread, _where ;
	int _occ,_rel;
	os>>_when;
	os>>_what;
	os>>_thread;
	os>>_where;
	os>>_occ;	
	os>>_rel;
	setWhen(_when);
	setWhere(_where);
	parseWhat(_what);
	parseWhen(_when);
	setFaultID();
	setOccurrence(_occ);
	setRelative(_rel);

}


InjectedFault::InjectedFault(Params *p)
  : MemObject(p), manifested(false), nxt(NULL), prv(NULL)
{
  std:: stringstream s1;
  int i;
  setFaultID();

  setWhen(p->when);
  setWhere(p->where);
  setWhat(p->what);

  setRelative(p->relative);
  setThread(p->threadId);
  setOccurrence(p->occurrence);
  
  int ret;
  ret = parseWhen(p->when);
  if(cores == -1){
    cores = p->cores;
    for(i = 0 ; i < cores ; i++){
      s1 << "system.cpu";
      s1 << i;
      coresCount.push_back(new cpuExecutedTicks(s1.str()));
      s1.str("");
    }
  }
  
  
  if (ret != 0) {
    std::cout << "InjectedFault::InjectedFault() -- Error while parsing When\n";
    assert(0);
  }
  ret = parseWhat(p->what);
  if (ret != 0) {
    std::cout << "InjectedFault::InjectedFault() -- Error while parsing What\n";
    assert(0);
  }
  dump();
}

InjectedFault::~InjectedFault()
{
  getQueue()->remove(this);
}

const std::string
InjectedFault::name() const
{
  return params()->name;
}

const char *
InjectedFault::description() const
{
    return "InjectedFault";
}

void
InjectedFault::init()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "InjectedFault:init()\n";
  }
}

void InjectedFault::storeWhen(std::fstream &os)
{
	if ( TickTiming )
	{
		os <<"Tick_";
		os<<getTiming();
	}
	else if ( InstructionTiming )
	{
		os<<"Inst_"
		os<<getTiming();
	}
	else if ( VirtualAddrTiming )
	{
		os<<"Addr_";
		os<<getTiming();
	}
}

void InjectedFault::storeWhat(std::fstream &os)
{
  if(InjectedFaultType == ImmediateValue)
	{
		os<<"Immd_";
		os<<getValue();
	}
	else if (InjectedFaultType == MaskValue)
	{
		os<<"Mask_";
		os<<getValue();
	}
	else if (InjectedFaultType == FlipBit)
	{
		os<<"Flip_";
		os<<getValue();
	}
	else if (InjectedFaultType == AllValue)
	{
		if(getValue())
			os<<"All1";
		else
			os<<"All0";
	}

}
 
void InjectedFault::store(std::fstream &os)
{
	
	storeWhen(os);
	storeWhat(os);
	os << getThread();
	os <<getWhere();
	os << getOccurrence();	
	os << getRelative();
	os << getOccurrence();
	os << getRelative();

}


void
InjectedFault::startup()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "InjectedFault:startup()\n";
  }

  dump();
}

InjectedFault *
InjectedFaultParams::create()
{
  if (DTRACE(FaultInjection)) {
    std::cout << "InjectedFaultParams:create()\n";
  }
  
  return new InjectedFault(this);
}


Port *
InjectedFault::getPort(const string &if_name, int idx)
{
  std::cout << "InjectedFault:getPort() " << "if_name: " << if_name << " idx: " << idx <<  "\n";
  panic("No Such Port\n");
}


int
InjectedFault::parseWhen(std::string s)
{
  if (s.compare(0,4,"Inst",0,4) == 0) {
    setTimingType(InjectedFault::InstructionTiming);
    setTiming(s.substr(5));
  }
  else if (s.compare(0,4,"Tick",0,4) == 0) {
    setTimingType(InjectedFault::TickTiming);
    setTiming(s.substr(5));
  }
  else if (s.compare(0,4,"Addr",0,4) == 0) {
    setTimingType(InjectedFault::VirtualAddrTiming);
    setTiming(s.substr(5));
  }
  else {
    std::cout << "InjecteFault::parseWhen() - Error Unsupported type " << s << "\n";
    assert(0);
    return 1;
  }
  
  return 0;
}

int
InjectedFault::parseWhat(std::string s)
{
  if (s.compare(0,4,"Immd",0,4) == 0) {
    setValueType(InjectedFault::ImmediateValue);
    setValue(s.substr(5));
  }
  else if (s.compare(0,4,"Mask",0,4) == 0) {
    setValueType(InjectedFault::MaskValue);
    setValue(s.substr(5));
  }
  else if (s.compare(0,4,"Flip",0,4) == 0) {
    setValueType(InjectedFault::FlipBit);
    setValue(s.substr(5));
  }
  else if (s.compare(0,4,"All0",0,4) == 0) {
    setValueType(InjectedFault::AllValue);
    setValue(0);
  }
  else if (s.compare(0,4,"All1",0,4) == 0) {
    setValueType(InjectedFault::AllValue);
    setValue(1);
  }
  else {
    std::cout << "InjectedFault::parseWhat() - Error Unsupported type " << s << "\n";
    assert(0);
    return 1;
  }
  
  return 0;
}

void 
InjectedFault::increaseTiming(uint64_t cycles, uint64_t insts, uint64_t addr)
{
  switch (getTimingType())
    {
    case (InjectedFault::TickTiming):
      {
	setTiming(cycles);
	break;
      }
    case (InjectedFault::InstructionTiming):
      {
	  setTiming(insts);
	  break;
      }
    case (InjectedFault::VirtualAddrTiming):
      {
	/*Originally this changed to a different timing type, however, we will keep it at the same type to enable the precise injection of permanent faults (e.g. first value of a macroblock)
	 */


	/* Virtual addresses can not be used for non transient faults, that is because in the presence of branches we can not predefine an address for the next fault to occure (will the branch succeed or not).
	   For that reason we change the Timing Type either the Tick or Instruction can be used, one can add an option but currently instructions are choosen no thought was put onto that choice.
	 */
	//setTimingType(InjectedFault::InstructionTiming);
	//setTiming(insts);
	
	/*we also remove the fault from the fault queue as it was done for the other types too
	 */
	//getQueue()->remove(this);
	
	break;
      }
    default:
      {
	std::cout << "InjectedFault::increaseTiming() - getTimingType default type error\n";
	assert(0);
	break;
      }
    }
}



void
InjectedFault::dump() const
{
  if (DTRACE(FaultInjection)) {
    std::cout << "===InjectedFault::dump()===\n";
    std::cout << "\tWhere: " << getWhere() << "\n";
    std::cout << "\tWhen: " << getWhen() << "\n";
    std::cout << "\tWhat: " << getWhat() << "\n";
    std::cout << "\tthreadID: " << getThread() << "\n";
    std::cout << "\tfaultID: " << getFaultID() << "\n";
    std::cout << "\tfaultType: " << getFaultType() << "\n";
    std::cout << "\ttimingType: " << getTimingType() << "\n";
    std::cout << "\ttiming: " << getTiming() << "\n";
    std::cout << "\trelative: " << getRelative() << "\n";
    std::cout << "\tvalueType: " << getValueType() << "\n";
    std::cout << "\tvalue: " << getValue() << "\n";
    std::cout << "\toccurrence: " << getOccurrence() << "\n";
    std::cout << "~==InjectedFault::dump()===\n";
  }
}


InjectedFaultQueue::InjectedFaultQueue(const string &n)
  : objName(n), head(NULL), tail(NULL)
{
  //warning
  // causes segmentation fault
  //if (DTRACE(FaultInjection)) {
  //std::cout << "InjectedFaultQueue::InjectedFaultQueue()\n";
  //}
}


void
InjectedFaultQueue::insert(InjectedFault *f)
{
  f->setQueue(this); // when inserting a fault to a queue place a reference from it to the queue

  InjectedFault *p;

  p = head;
  
  if (empty()) {//queue is empty
    head = f;
    tail = f;
    return;
  }
  else {//queue is not empty
    while ((p!=NULL) && (p->getTiming() < f->getTiming())) {//travel queue elements to find the one before which our element will be inserted
      p = p->nxt;
    }
    
    if (p==NULL) {//element inserted at the end
      tail->nxt = f;
      f->prv = tail;
      tail = f;
      return;
    }
    else if (p==head) {//element inserted in the beginning
      head->prv = f;
      f->nxt = head;
      head = f;
      return;
    }
    else {//element inserted between two other elements
      p->prv->nxt = f;
      f->prv = p->prv;
      p->prv =f;
      f->nxt = p;
      return;
    }
  }

}

void
InjectedFaultQueue::remove(InjectedFault *f)
{
  InjectedFault *p;


  if ((head==NULL) & (tail==NULL)) {//queue is empty
    return;
  }

  p = head;
  while ((p!=NULL) && (p!=f)) {//search if event exists
    p = p->nxt;
  }

  if (p==NULL) {//event was not found
    std::cout << "InjectedFaultQueue:remove() -- Fault was not found on the queue\n";
    assert(0);
  }

  if (f->prv==NULL) {//fault to be removed is the first one
    head = f->nxt;
  }
  else {
    f->prv->nxt = f->nxt;
  }

  if (f->nxt==NULL) {//fault to be removed is the last one
    tail = f->prv;
  }
  else {
    f->nxt->prv = f->prv;
  }

  return;

}


InjectedFault *InjectedFaultQueue::scan(std::string s , ThreadEnabledFault &thisThread , Addr vaddr){

  InjectedFault *p;
  int i;
  int64_t exec_time = 0;
  uint64_t exec_instr = 0;
  
  char *dummy= (char*)(s.c_str());
  dummy = dummy + 10;
  std:: stringstream s1;
  int cpuindex = atoi(dummy);
  if(cpuindex > cores){
    std::cout << "InjectedFault: SCAN :: wrong number of cores!\n";
    assert(0);
  }
  
 
  p = head;
  while(p){
    i =  get_fi_counters( p , thisThread, s , &exec_time , &exec_instr );
/*    if (DTRACE(FaultInjection)) {
	  std::cout << "=============scan() " <<s<<"======\t"<<i<<"\t==========\n";
    }*/
    if(i){
/*	if (DTRACE(FaultInjection)) {
	  std::cout<<"============threadid :"<<thisThread.getThreaId()<<"============\n";
	  std::cout<<"============exec time :"<<exec_time<<"============\n";
	  std::cout<<"============exec instr :"<<exec_instr<<"===========\n";
	  std::cout << "*************************************************************\n";
	  std::cout<<"*****************:"<<p->getTiming()<<"*************************\n";
	  std::cout << "*************************************************************\n";  
	}
*/
 	switch( p->getTimingType() ){
	  case (InjectedFault ::TickTiming):
	  {
	    if(p->getRelative() && thisThread.getRelative() ){
	      
	      if(exec_time == p->getTiming()){
		p->setServicedAt(exec_time);
		return(p);
	      }
	    }
	    else if(!p->getRelative()){
	      if(p->getTiming() == exec_time){
		p->setServicedAt(exec_time);
		return p;
	      }
	    }
	  }
	  break;
	  case (InjectedFault ::InstructionTiming):
	  {
	    if(p->getRelative() && thisThread.getRelative()){
	      if(exec_instr == p->getTiming() ){
		p->setServicedAt(exec_instr);
		return(p);
	      }
	    }
	    else if(!p->getRelative()){
	      if(p->getTiming() == exec_instr){
		p->setServicedAt(exec_instr);
		return p;
	      }
	    }
	  }
	  break;
	  case (InjectedFault ::VirtualAddrTiming):
	  {
	    if(p->getRelative() && thisThread.getRelative() ){
	      if(vaddr == p->getTiming() + thisThread.getMagicInstVirtualAddr() ){
		p->setServicedAt(vaddr);
		return(p);
	      }
	    }
	    else if(!p->getRelative()){
	      if(p->getTiming() == vaddr){
		p->setServicedAt(vaddr);
		return p;
	      }
	    }
	  }
	  break;
	  default:
	    {
	      std::cout << "InjectedFaultQueue::scan() - getTimingType default type error\n";
	      assert(0);
	      break;
	    }
	
	
	}
      }
      p = p->nxt;
  }
  return(NULL);
  
}



/*
InjectedFault *
InjectedFaultQueue::scan(std::string s, int64_t t, uint64_t instCnt, Addr vaddr , ThreadEnabledFault &thisThread)
{
  InjectedFault *p;
 
  
  int64_t exec_time;
  uint64_t exec_instr;
  Addr exec_addr;
  thisThread.increaseExecutedTime(s); 
  p = head;
  thisThread.CalculateExecutedTime(s,&exec_time,&exec_instr,&exec_addr);
  if( DTRACE(FaultInjection) && exec_instr % 2000 == 0){
    dump();
  }



  while (p){
    if(!(p->getWhere().compare(s)) || !(p->getWhere().compare("all"))){
*	if(p->getFaultType() == p->RegisterInjectedFault || p->getFaultType() == p->PCInjectedFault ){
	    p->setCPU(reinterpret_cast<BaseCPU *>(p->find(s.c_str())));
	}
	else if(p->getFaultType() == p->GeneralFetchInjectedFault || p->getFaultType() == p->OpCodeInjectedFault || p->getFaultType() == p->RegisterDecodingInjectedFault || p->getFaultType() == p->ExecutionInjectedFault){
	    p->setCPU(reinterpret_cast<BaseO3CPU *>(p->find(s.c_str())));
	}

	switch( p->getTimingType() ){
	  case (InjectedFault ::TickTiming):
	  {
	    if(p->getRelative() && thisThread.getRelative() ){
	      if(exec_time == p->getTiming() + thisThread.getMagicInstTickCnt()){
		p->setServicedAt(t);
		return(p);
	      }
	    }
	    else{
	      if(p->getTiming() == exec_time){
		p->setServicedAt(t);
		return p;
	      }
	    }
	  }
	  break;
	  case (InjectedFault ::InstructionTiming):
	  {
	    if(p->getRelative() && thisThread.getRelative()){
	      if(exec_instr == p->getTiming() + thisThread.getMagicInstInstCnt()){
		p->setServicedAt(t);
		return(p);
	      }
	    }
	    else{
	      if(p->getTiming() == exec_instr){
		p->setServicedAt(t);
		return p;
	      }
	    }
	  }
	  break;
	  case (InjectedFault ::VirtualAddrTiming):
	  {
	    if(p->getRelative() && thisThread.getRelative() ){
	      if(exec_addr == p->getTiming() + thisThread.getMagicInstVirtualAddr()){
		p->setServicedAt(t);
		return(p);
	      }
	    }
	    else{
	      if(p->getTiming() == exec_addr){
		p->setServicedAt(t);
		return p;
	      }
	    }
	  }
	  break;
	  default:
	    {
	      std::cout << "InjectedFaultQueue::scan() - getTimingType default type error\n";
	      assert(0);
	      break;
	    }
	}
    }
    p = p->nxt;
  }
  return NULL;
}

*/

void
InjectedFaultQueue::dump() const
{
  InjectedFault *p=head;

  if (DTRACE(FaultInjection)) {
    std::cout << "=====InjectedFaultQueue::dump()=====\n";
    while (p) {
      p->dump();
      p = p->nxt;
    }
    std::cout << "~====InjectedFaultQueue::dump()=====\n";
  }
}
