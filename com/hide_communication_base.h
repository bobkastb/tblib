#pragma once

#ifdef __INTERNAL__COMMUNICATION__TBGENERAL
#include <queue>

namespace tbgeneral{


//*********************
template <class Data_t> struct cVectorAllocator{
	struct cRec{Data_t data; int next;};
	int firstfree;
	std::vector<cRec> datav;
	int Alloc( const Data_t & data ) {  int res; cRec rec; rec.data=data; rec.next=-2; 	
		if (firstfree<0) { res=(int)datav.size(); datav.push_back(rec); }
		else { res=firstfree; firstfree=datav[res].next; datav[res]=rec; }
		return res;
	};
	bool exist(uint num) { return datav[num].next==-2; }
	void Free( uint num ) { if (num>datav.size()) return; 
		if (datav[num].next!=-2) return;
		cRec rec; rec.data=0; rec.next = firstfree;
		datav[num] = rec;
		firstfree = num;
	};
	cVectorAllocator() {firstfree=-1;}
};

template <class Data_t> struct cEventAllocator{
	typedef Data_t * Data_ptr;
	crsys::sys_mutex m_Res;
	cVectorAllocator<Data_ptr> ListOfResource;
	std::vector<Data_ptr> FreeResource;

	Data_ptr AllocR(){ AUTOENTER(m_Res); Data_ptr res=FreeResource.back(); 
			if (!res) { res=new Data_t(); ListOfResource.Alloc(res); }
			else { FreeResource.pop_back(); }
			res->OnAllocUse();
			return res;
		};	
	void FreeR(Data_ptr ev){
		AUTOENTER(m_Res);
		FreeResource.push_back(ev); //ev->used=false;
		ev->OnFreeUse();
	};
	
	void SignalToAllUsed( uint sigdata , uint ownerdata ){ AUTOENTER(m_Res);
		for (uint i=0;i<ListOfResource.datav.size();i++) if (ListOfResource.exist(i)) {
				Data_ptr dp=	ListOfResource.datav[i].data;
				if (dp->IsBusy()) 
					dp->Signal(sigdata , ownerdata);
		};
	};
	
};
//*********************



struct cCommSignalRec{
	uint sigdata ; uint ownerdata;
	cCommSignalRec(){ ownerdata=0;sigdata=0;};
	cCommSignalRec(uint _sig  , uint _own  ){ ownerdata=_own;sigdata=_sig;};
};
struct cCommExSignalRec: public cCommSignalRec{
	bool parentdef, issignal;	
	cCommExSignalRec(){ parentdef=issignal=false; };
};

template <class Data_t> struct cSyncQueue {
	std::deque<Data_t> que;
	crsys::sys_mutex m_Que;

	void push(const Data_t & d){ AUTOENTER(*this); que.push_back( d ); }; 
	bool pop(Data_t & d) { AUTOENTER(*this);  if (empty()) return false;  d= *que.front(); que.pop_front();  return true; }; 
	bool empty() { return que.size()==0;};
	void reset() { que.resize(0); };
	void enter() { m_Que.enter(); };
	void leave() { m_Que.leave(); };
		
	};


struct cCommEvent{
	bool used;
	void * handle;
	bool IsBusy() { return (this) && this->used;}
	virtual void Signal( const cCommSignalRec & sr ){}; 
	virtual void Reset(){};
	virtual void OnFreeUse(){ used=false; };
	virtual void OnAllocUse(){ used=true; };
	virtual void AllocSysResource(){};	
	virtual void FreeSysResource();	// CloseHandle
	virtual bool getevent(cCommExSignalRec & sr);

	cCommEvent( ){ AllocSysResource(); };	
	~cCommEvent(){ FreeSysResource(); };	
};
typedef cCommEvent * cCommEvent_ptr;

struct cCommAutoEvent: public cCommEvent{
	virtual void AllocSysResource();	
	virtual void Signal( const cCommSignalRec & sr ); 
};
struct cCommResetEvent: public cCommEvent{
	virtual void AllocSysResource();	
	virtual void Signal( const cCommSignalRec & sr ); 
	virtual void Reset();
};
struct cCommIOCompPort: public cCommEvent{
	virtual void AllocSysResource();	
	virtual void Signal( const cCommSignalRec & sr ); 
};
struct cCommEvQue: public cCommResetEvent{
	cSyncQueue<cCommSignalRec> que;
	virtual void AllocSysResource();	
	virtual void OnAllocUse(){ cCommEvent::OnAllocUse(); que.reset(); };
	virtual void Signal( const cCommSignalRec & sr ) { AUTOENTER(que); que.push( sr ); cCommResetEvent::Signal(sr); }; 
		
};



}; // namespace tbgeneral
#endif 