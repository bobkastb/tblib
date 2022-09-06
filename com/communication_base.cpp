#include "communication_base.h"
#include <vector>
#include <list>
#include "tsys/sys_mutex.h"

#define __INTERNAL__COMMUNICATION__TBGENERAL
#include "hide_communication_base.h"


namespace tbgeneral{


struct cMainCommunicationManager{
	static cMainCommunicationManager main;	
	cVectorAllocator<cChannelInstanceBase*> ListOfChInstance; 
	crsys::sys_mutex m_Instance;

	cEventAllocator<cCommAutoEvent>  fEvents;
	cEventAllocator<cCommEvQue>  fQues;

	cCommAutoEvent * AllocEvent() { return fEvents.AllocR(); };
	void FreeEvent(cCommAutoEvent * ev) { fEvents.FreeR( ev ); };
	void AddInstance(cChannelInstanceBase* inst){ AUTOENTER(m_Instance); inst->chID = ListOfChInstance.Alloc( inst );	};
	void Remove(cChannelInstanceBase* inst){ AUTOENTER(m_Instance); 	ListOfChInstance.Free( inst->chID ); };

	void FreeR(cCommAutoEvent * ev) { if (ev) fEvents.FreeR( ev ); };
	void waitevent( cCommEvent_ptr ev , uint ms ); //TODO:
};

template <class Data_t> struct cCommObjectRef{
	Data_t * data;	
	void Reset( ){ 
		Data_t * fdata=data; 
		data=0; 
		cMainCommunicationManager::main.FreeR(fdata); 
	};
	cCommObjectRef() { data=0;}
	~cCommObjectRef() { Reset(); }
	};

struct cChannelInstance_InternalBase: public cChannelInstanceBase{
	crsys::sys_mutex m_Channel;
	std::vector<cCommEvent_ptr> AssociatedEvents;
	//QueueUserAPC
	//CreateIOCompletionPort - лучшее решение
	virtual void OnCreate();
	virtual void OnDestroy();
	
	void add_AssocEvent( const cCommEvent_ptr & Ev ) ;
	void rem_AssocEvent( const cCommEvent_ptr & Ev ) ;
	void Signal_AssocEvent( uint signaldata ) ;
	
	virtual void StartRead();

};

	void cChannelInstance_InternalBase::add_AssocEvent( const cCommEvent_ptr & Ev ) { AUTOENTER(m_Channel); 
		for (uint i=0;i<AssociatedEvents.size();i++) if (AssociatedEvents[i]==Ev) return;
		AssociatedEvents.push_back(Ev);
	};
	void cChannelInstance_InternalBase::rem_AssocEvent( const cCommEvent_ptr & Ev ) { AUTOENTER(m_Channel); 
		for (uint i=0;i<AssociatedEvents.size();i++) if (AssociatedEvents[i]==Ev) {
			AssociatedEvents.erase( AssociatedEvents.begin()+i ); return;
		};
	};
	void cChannelInstance_InternalBase::Signal_AssocEvent( uint signaldata ) { AUTOENTER(m_Channel); 
		for (uint i=0;i<AssociatedEvents.size();i++) AssociatedEvents[i]->Signal( cCommSignalRec( signaldata , chID) );
	};

struct cLocalMemChannel: public cChannelInstance_InternalBase{
	crsys::sys_mutex m_Buff;
	std::vector<byte> buff;
	
};
cMainCommunicationManager cMainCommunicationManager::main;

//cChannelInstance_InternalBase* ChIB( cChannel & ch) { return (cChannelInstance_InternalBase*)( *ch ); } 

int cManagerChannels::WaitChannelsRead(cChannel * cha , uint chcnt , uint ms){
	cMainCommunicationManager & main = cMainCommunicationManager::main;
	//TODO: cheak ready to read 
	cCommAutoEvent * fev = main.fEvents.AllocR(); cChannelInstance_InternalBase* chi;
	for (uint i=0;i<chcnt;i++) {
		chi=((cChannelInstance_InternalBase*)cha[i].fch.data()); 
		if (chi) { chi->add_AssocEvent( fev ); chi->StartRead(); };
	};
	main.waitevent( fev , ms );
    int result=0;
	for (uint i=0;i<chcnt;i++) { 
		chi=((cChannelInstance_InternalBase*)cha[i].fch.data()); 
		chi->rem_AssocEvent( fev );
		//if ( chi->isReadyToRead() ) result=1<<i;
		//if ( sr.ownerdata == chi->chID ) result=1<<i;
	};
	main.fEvents.FreeR(fev);
	return result;
}; 



}; //namespace tbgeneral