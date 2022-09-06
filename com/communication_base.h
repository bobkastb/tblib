#pragma once

#include "gdata/t_string.h"
#include "gdata/t_sstruct.h"
#include <string>


namespace tbgeneral {
	struct cManagerChannels;	
	struct cChannelInstanceBase;
	struct cChannel;

	struct cChannelInstanceBase{
		enum { INFINITE=uint(-1)};	
		enum { stReadReadyMSK=0xF0 };	
		stringW chname;
		uint chID;
		uint chtp;
		uint lasterror;
		virtual uint ReadData( void * buff , uint dsz , uint* err=0);
		virtual uint WriteData( void * buff , uint dsz, uint* err=0 );
		virtual uint WaitData( uint sz=1 , uint ms=INFINITE );
		virtual int GetStatus(){ return 0; };
		~cChannelInstanceBase();
		friend struct cChannel;
		friend struct cManagerChannels;
		protected:
		virtual void StartRead(){}; 
	};
	struct cChannel{
		cChannelInstanceBase *  operator ->() const { return fch.fdata;};
		
		uint ReadData( void * buff , uint dsz , uint* err=0) { return fch->ReadData(buff , dsz , err ); };
		uint WriteData( void * buff , uint dsz , uint* err=0) { return fch->WriteData(buff , dsz , err ); };
		uint WaitData( uint sz=1 , uint ms=cChannelInstanceBase::INFINITE ) { return fch->WaitData(sz, ms ); };
		bool Connected();
		bool Exist(){ return fch.fdata != 0; };
		
		protected:
		rc_base_struct<cChannelInstanceBase> fch;
		friend struct cManagerChannels;
	};
	struct cServerOfChannels{
		
		virtual bool Accept( cChannel & newch , uint ms );
		virtual stringW GetServerName(){ return fname;}; 
		void Close();
		protected:
		stringW fname;
		
	};

	struct cManagerChannels{
		enum enumChanelType{ echtp_MultiInstance = 0x10000 , echtp_Text = 0x20000 };
		cChannel newChanel();
		cServerOfChannels newServerChanel(const wchar_t *chname , enumChanelType chtp=echtp_MultiInstance );
		cChannel ConnectToChanel(const wchar_t *chname=0);
		void Initialize( uint CountConqThreads );	
		void CloseChanel( cChannel & ch );
		int WaitChannelsRead(cChannel * cha , uint chcnt , uint ms); 
		private:
		void * fdata;	
	};


};