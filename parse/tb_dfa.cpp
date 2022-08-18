#include <vector>
#include <algorithm>

#include "gdata/t_format.h"
#include "tb_dfa.h"
//#include "gdata/t_tools.h"

//#include "tb_vector_cs.h"





namespace tbgeneral{

	template <class Vec> 
	int preinit_addch_for_mask(Vec & vec , int m){ if (!m) return 0;
		enum {MB=1<<(sizeof(typename Vec::element_t)*8)};
		cBitSetC<MB> bs;
		for (int i=0;i<MB;i++) { 
			typename Vec::element_t ii= i & m; 
			if (ii && !bs.test(ii)) 
				{ vec.push_back(ii); bs.set(ii); } 
		}
		return 0;
	} 
	int c_DFA_NativeCode_Base::newmaskforstate(cEState & st, char_t nm){
		if (st.startTransferIndex!=BAD_TINDEX) {  
			enum {MB=256};
			cVectorC<char_t,MB> chs;  
			char_t m = nm & ~st.pre_mask; 
			preinit_addch_for_mask( chs , m );
			unpacked_transfers_t & tt=unp_transfers[ st.startTransferIndex ];
			auto oldc= tt.size();
			for (size_t i=0;i<oldc;i++) { unp_transfer upt=tt[i]; char_t v=upt.c;
				for (int k=0;k<chs.sz;k++) { upt.c=v|chs.data[k]; tt.push_back(upt); }
			};
		};
		st.pre_mask = nm;
		setstate(st);
		return 0;
	}; 
	template<class VectorT , class char_t >	
	int fillchar_ttlist( char_t v , int m, VectorT & vec ){
		enum {MB=1<<(sizeof(VectorT::element_t)*8)};
		cVectorC<char_t,MB> chs;  
		preinit_addch_for_mask( chs , m );
		vec.sz=0;
		vec.push_back(v);
		for (int i=0;i<chs.sz;i++) vec.push_back( v|chs.data[i]);
		return 0;
	}
	template<class VectorT >	
	int fillchars_ttlist( VectorT & vec , int m ){ if (!m) return 0;
		enum {MB=1<<(sizeof(typename VectorT::element_t)*8)};
		cVectorC<typename VectorT::value_type,MB> chs;  
		preinit_addch_for_mask( chs , m );
		int szin=vec.size();
		for (int k=0;k<szin;k++) { 
			for (int i=0;i<chs.sz;i++) vec.push_back( vec[k] |chs.data[i]);
		};
		return 0;
	}

	//template<class Vec> bool findinvector(Vec )	value_type


	template<class Set, class VecTT> void setfrom_TL( Set& set, const VecTT & tl ){
		set.clear(); for (uint i=0;i<tl.size();i++) { set.set(tl[i].c); };
	};
	template<class Set, class VecTT> void getchars_fromNS_TT( Set& set, int state, const VecTT & tl  ){
		set.clear(); for (uint i=0;i<tl.size();i++) if (state==tl[i].st) { set.set(tl[i].c); };
	};
	template<class VecC, class VecTT> void Filtred_TT( VecC & dst, const VecTT & tl , int state ){
		dst.clear(); for (uint i=0;i<tl.size();i++) if (state==tl[i].st) { dst.push_back(tl[i].c); };
	};
	template<class Vec , class VecTT , class Set > int statesTTConvolution( Vec & r, const VecTT & tt , const Set* allowedc ){
		for (uint i=0;i<tt.size();i++) {
			if ((allowedc)&&(!allowedc->test(tt[i].c))) continue;
			int sv=tt[i].st; if (0>vectorFindValue(r,sv)) r.push_back(sv);
		};
		return r.size();
	}
	template<class Set , class VecTT > int TTRedirectT( VecTT & tt , Set& cset , int newstate ){
		for (uint i=0;i<tt.size();i++) {	if (cset.test(tt[i].c)) tt[i].st = newstate; };
		return 0;
	}


	void c_DFA_NativeCode_Base::makezerostate(){
		if (this->startstate == BADSTATE || this->states.size()==0) {
			addnewstate();
			this->startstate = addnewstate().id;
		};
	};
	c_DFA_NativeCode_Base::cEState c_DFA_NativeCode_Base::addnewstate(){
		cEState st; st.pre_mask=0x0; st.Rule = BAD_RULE; st.startTransferIndex = BAD_TINDEX;
		st.id = static_cast<istate_t>( states.push_back( st ) );
		return st;
	};

	void c_DFA_NativeCode_Base::setFinalState( cEState & st , c_addinctx & ctx){
		if ( (st.isfinal() && st.Rule != ctx.Ud) || (st.startTransferIndex != BAD_TINDEX) ) doerr(119);
		st.Rule = ctx.Ud; st.pre_mask=0;	setstate( st );
		ctx.final = st.id;
	};

	c_DFA_NativeCode_Base::unpacked_transfers_t & c_DFA_NativeCode_Base::GetMakeTT( cState & cs ){
		if (cs.startTransferIndex==BAD_TINDEX) 
			cs.startTransferIndex = static_cast<int>( unp_transfers.push_back( unpacked_transfers_t() ));
		return unp_transfers[ cs.startTransferIndex ];  
	};
	c_DFA_NativeCode_Base::unpacked_transfers_t c_DFA_NativeCode_Base::GetTT( istate_t ist ){
		return (states[ist].startTransferIndex == BAD_TINDEX )? unpacked_transfers_t() : unp_transfers[ states[ist].startTransferIndex ];
	};


	int c_DFA_NativeCode_Base::makecopystatechain( istate_t from ){
		cEState cs= getstate( from ); if (cs.isfinal()) return cs.id;
		if (cs.startTransferIndex==BAD_TINDEX) return cs.id;
		cEState ns= addnewstate() ;
		*(cState*) &ns = *(cState*) &cs;
		unpacked_transfers_t upt = unp_transfers[ cs.startTransferIndex ]; upt.makeunique();
		ns.startTransferIndex = static_cast<int>( unp_transfers.push_back( upt ) );
		setstate(ns);

		cVectorC<istate_t,MB> astates , nstates; 
		statesTTConvolution(astates , upt , (cBitSetC<MB>*) 0 );
		nstates.resize( astates.size() );
		for (uint i=0;i<astates.size();i++) { istate_t fst=astates[i];
			nstates[i] = makecopystatechain( fst );
			if (nstates[i]!=fst) 
				for (uint k=0;k<upt.size();k++) if (upt[k].st==fst) upt[k].st= nstates[i];
		};
		return ns.id;
			
	};

	int c_DFA_NativeCode_Base::add_In_word(c_addinctx & ctx , c_addindata nd){
		cVectorC<char_t,MB> chlst;  cBitSetC<MB> mR, mRS;  
		c_inputword::c_symbol sym;
		if (!ctx.iw.getsymbol( nd.mindex ,sym , chlst)) doerr(118); 
		nd.mindex++;
		char_t m=sym.mask; 
		cEState cs= getstate( nd.curr ) ;  if (cs.isfinal()) doerr(119);
		bool nextfinal = ctx.iw.isend(nd.mindex); ;

		char_t nm=m | cs.pre_mask;
		if (cs.pre_mask != nm) { newmaskforstate(cs, nm); };// увеличение уже существующего множества переходов 
		if (m != nm) { 	fillchars_ttlist(  chlst , nm & ~m  ); } // составить список символов

		
		unpacked_transfers_t upt= GetMakeTT(cs);
		mR.FromVector( chlst ); 
		//{
		cBitSetC<MB> mS; setfrom_TL( mS , upt ); mRS= mR & mS; mR= mR-mS; 
		//};
		if (!mRS.empty()) {
			if (nextfinal) doerr(125);
			cVectorC<istate_t,MB> astates; statesTTConvolution(astates , upt , &mRS);
			for (uint i=0;i<astates.size();i++) { istate_t nsi=astates[i];
				cBitSetC<MB> mNS,mNSR; getchars_fromNS_TT( mNS , nsi , upt );
				mNSR = mNS & mRS;
				if (mNSR.empty()) continue;
				if (getstate(nsi).isfinal()) doerr(120);
				if (mNSR == mNS) { // используется старое состояние
					nd.curr = nsi;
				} else { // образуется новое состояние
					nd.curr=makecopystatechain(nsi);
					TTRedirectT( upt, mNSR , nd.curr );
				};
				add_In_word(ctx,nd); 
			};
		};
		if (!mR.empty()) {
			unpacked_transfers_t & upt= GetMakeTT(cs);
			cEState ns; if (nextfinal && ctx.final!=BADSTATE ) ns=getstate(ctx.final); else ns= addnewstate() ;
			unp_transfer tt; tt.st = ns.id; 
			for (int i=0;i<chlst.sz;i++) if (!mRS.test( chlst.data[i] )) {
				tt.c =chlst.data[i];  upt.push_back( tt );
			};
			cs.pre_mask=nm;
			setstate( cs );
			nd.curr = ns.id; 
			if (nextfinal ) { setFinalState( ns , ctx ); }
			else add_In_word(ctx,nd); 
		};
		return 0;
	};

	int c_DFA_NativeCode_Base::add_In_word( const c_inputword & iw ){
		if (ispacked()) return doerr(101);
		makezerostate();
		c_addinctx ctx ; c_addindata nd;
		ctx.iw = iw;
		nd.curr = startstate; //nd.mask= mask;  nd.val=val; nd.cnt=cnt; 
		nd.mindex=0;
		ctx.final = BADSTATE; ctx.start=nd; ctx.Ud = iw.Ud;
		return add_In_word( ctx , nd ); 

	};

	int c_DFA_NativeCode_Base::add_In_word( char_t * mask, char_t * val , irule_t Ud ,  int cnt ) { // длина не ограничена
		return add_In_word(c_inputword( mask, val, Ud , cnt));
		/*
		if (ispacked()) return doerr(101);
		makezerostate();
		c_addinctx ctx ; c_addindata nd;
		nd.curr = startstate; nd.mask= mask;  nd.val=val; nd.cnt=cnt; nd.mindex=0;
		ctx.final = BADSTATE; ctx.start=nd; ctx.Ud = Ud;
		return add_In_word( ctx , nd ); 
		*/
	}; 

	//--------------------------------------------
	template <class Vec1 , class Vec2 > int _do_packedstates(const Vec1 & unp, Vec2 & pk, int fromst , int packofs ) {
		int szu= static_cast<int>(unp.size());
		int ofsp= packofs - unp[0].c , mxpck= ofsp + unp[szu-1].c;
		if ((int)pk.size()<=mxpck) pk.resize(mxpck+1);
		for (uint i=0;i<unp.size();i++) { 
			typename Vec1::value_type unpr=unp[i];
			int ofs=ofsp+unpr.c;
			if (pk[ofs].from!=c_DFA_NativeCode_Base::BADSTATE) { throw "DKA paked: unavailable place"; };
			pk[ofs].from=fromst; pk[ofs].to=unpr.st;
		};
		return ofsp;
	};
	bool operator < ( const c_DFA_NativeCode_Base::unp_transfer & l , const c_DFA_NativeCode_Base::unp_transfer & r ){ 		return l.c < r.c; };
	void c_DFA_NativeCode_Base::pack(){
		//<cTransfer> transfers
		int lfree=0;
		struct l_starr{ istate_t sti; uint cntTT; 
		bool operator < (l_starr r) const { return cntTT<r.cntTT; };  
		};
		darray<l_starr> lstst;
		for (uint i=0;i<states.size();i++) { l_starr s;s.sti=i;s.cntTT= static_cast<uint>( GetTT(i).size()); lstst.push_back(s); }
		std::sort( &lstst[0] , &lstst[0] + lstst.size() ); 

		for (uint _ist=0;_ist<states.size();_ist++) { uint ist=lstst[states.size()-1-_ist].sti; cState & cs=states[ist];	unpacked_transfers_t upt = GetTT(ist);	
			if (upt.size()==0) { cs.startTransferIndex =BAD_TINDEX; continue; };
			std::sort( &upt[0] , &upt[0] + upt.size() ); 
			if (transfers.size()==0) { cs.startTransferIndex = _do_packedstates(upt ,transfers, ist , 0 );   continue; }
			int foundofs=-1 , first=0;
			for (uint i=lfree;i<transfers.size();i++) if (transfers[i].from==BADSTATE) {   
				if (!first) { lfree=i; first=1; };
				int ofs=i - upt[0].c , iicnt= static_cast<int>(transfers.size()) ; uint k;
				for (k=0;k<upt.size();k++) { int ii=ofs+upt[k].c;
					if ((ii<iicnt)&&(transfers[ii].from!=BADSTATE)) 
							break; }
				if (k==upt.size()) { foundofs=i; break; };
			};
			if (!first) { lfree= static_cast<int>(transfers.size()); first=1; };
			if (foundofs==-1) foundofs= static_cast<int>(transfers.size());
			cs.startTransferIndex = _do_packedstates(upt ,transfers, ist , foundofs );
		};
		status |= ePacked;
		unp_transfers.clear();
	};


	int c_DFA_NativeCode_Base::findUnPack( const c_DataPtr<char_t> & _buff , irule_t & frule ) 
	{   istate_t si=this->startstate;
		//int ttsize=(int)transfers.size();
		c_DataPtr<char_t> buff= _buff;
		for (int i=0;i<=buff.cnt;i++) {
			cState cs= states[si]; //cState & cs= states[si];
			if (cs.isfinal()) { frule=cs.Rule; return i; };
			if (i==buff.cnt) break;
			char_t v=buff.p[i] & cs.pre_mask;
			unpacked_transfers_t tt= GetTT( si );
			bool fndc=false; for (size_t i=0;i<tt.size();i++) if (tt[i].c == v) {
				si = tt[i].st;fndc=true;
			};
			if (!fndc) return 0;
		};
		return 0;
	};
		
	


	int c_DFA_NativeCode_Base::find( const c_DataPtr<char_t> &  _buff , irule_t & frule ) // return size of cmd, for bad - return 0 
	{   istate_t si=this->startstate;
		int ttsize=(int)transfers.size();
		c_DataPtr<char_t> buff= _buff;
		for (int i=0;i<=buff.cnt;i++) {
			cState cs= states[si]; //cState & cs= states[si];
			if (cs.isfinal()) { frule=cs.Rule; return i; };
			if (i==buff.cnt) break;
			char_t v=buff.p[i] & cs.pre_mask;
			int ii= cs.startTransferIndex +v;  
			if ((ii<0)||(ii>=ttsize)|| (transfers[ii].from!=si) ) return 0;
			si = transfers[ii].to;
		};
		return 0;
	};


/*

	int c_DFA_NativeCode_Base::find( const c_DataPtr<char_t> &  _buff , irule_t & frule ) // return size of cmd, for bad - return 0 
	{   istate_t sii=this->startstate;
		//int ttsize=(int)transfers.size();
		c_DataPtr<char_t> buff= _buff;
		cState * states_ptr = states.fdata;    //8
		cTransfer* transf_ptr=transfers.fdata; //4
		void* buffend= buff.p + buff.cnt;
		int result;
		// cs. Rule>=BAD_RULE_HI
		//__asm mov eax, [this]
		//__asm push edi __asm push esi __asm push ebx 
		__asm mov esi,buff.p  __asm mov ebx,states_ptr __asm mov edi,transf_ptr __asm movzx edx,sii
		loop1:
		__asm {
				mov eax,dword ptr [ebx+edx*8+4]  // rule(2) + mask(1)
				mov ecx,dword ptr [ebx+edx*8+0]  // startoffset
				cmp ax,0
				jge exit1	
				cmp esi,buffend // MEM: if (i==buff.cnt) break;  ** можно менять на константу... и вааще - все базы...
				jae	exit1
				bswap eax
				lodsb
				and al,ah //& cs.pre_mask
				movzx eax,al
				add eax,ecx // cs.startTransferIndex +buff.p[i] & cs.pre_mask;
				js exit2
				cmp eax, 0x89a // ttsize //MEM:
				jae exit2
				mov ecx,dword ptr [edi+eax*4+0]
				movzx edx,cx
				shr ecx,16
				cmp ax,cx
				je loop1
		};
		exit2: __asm mov eax,0
		exit1:
		//__asm pop ebx __asm pop esi __asm pop edi 
		__asm mov result,eax
		return result;
	};

*/
	int c_DFA_NativeCode_Base::find4( uint buff , irule_t & frule ) // return size of cmd, for bad - return 0 , speed find for 4 bytes
	{   istate_t si=this->startstate;
		int ttsize=(int)transfers.size();
		const cTransfer * tt=transfers.data();
		for (int i=0;i<=4;i++,buff>>=8) {
			cState cs= states[si]; //cState & cs= states[si];
			if (cs.isfinal()) { frule=cs.Rule; return i; };
			if (i==4) break;
			char_t v=(buff & 0xFF) & cs.pre_mask;
			int ii= cs.startTransferIndex +v;  
			if ((ii<0)||(ii>=ttsize)|| (tt[ii].from!=si) ) return 0;
			si = tt[ii].to;
		};
		return 0;
	};


	c_DFA_NativeCode_Base::cStats c_DFA_NativeCode_Base::getstats(){
		cStats r; ZEROMEM(r);
		r.states = static_cast<int>(states.size()-1); r.transitions_unuse=0;
		for (uint i=0;i<transfers.size();i++) if (transfers[i].from==BADSTATE) r.transitions_unuse++;
		r.transitions = static_cast<int>(transfers.size()) - r.transitions_unuse;
		return r;
	};

	//--------------------------------------------
	struct cGenerationCtx{
		enum { BADSTATE=c_DFA_NativeCode_Base::BADSTATE , MB=c_DFA_NativeCode_Base::MB , HEX= c_DFA_NativeCode_Base::cOptionGeneration::HEX };
		typedef c_DFA_NativeCode_Base::istate_t istate_t;
		typedef c_DFA_NativeCode_Base::cEState cEState;
		typedef c_DFA_NativeCode_Base::unp_transfer unp_transfer;
		typedef c_DFA_NativeCode_Base::char_t char_t;
		
		c_DFA_NativeCode_Base * self;
		c_DFA_NativeCode_Base::cResultGeneration res;
		c_DFA_NativeCode_Base::cOptionGeneration opt;
		std::vector<bool> statemasks;
		stringA buff;

		int get_TTlst( unp_transfer* tl , uint sz , const cEState & cs ){
			if (self->ispacked()) {
				uint cnt=0;
				darray<c_DFA_NativeCode_Base::cTransfer> transfers= self->transfers;
				int hTI= cs.startTransferIndex + self->charsPower();
				int lTI= cs.startTransferIndex; 
				if (lTI<0) lTI=0; 
				if (hTI>=(int)transfers.size()) hTI= static_cast<int>(transfers.size());
				for (int i=lTI;i<hTI;i++) if (transfers[i].from==cs.id) { 
					//c_DFA_NativeCode_Base::char_t ch = i-cs.startTransferIndex;
					unp_transfer nt; nt.c = i-cs.startTransferIndex; nt.st = transfers[i].to;
					if (cnt>=sz) self->doerr(55);
					tl[cnt]= nt; cnt++;
				};
				return cnt;
			}else {
				c_DFA_NativeCode_Base::unpacked_transfers_t upt=self->unp_transfers[cs.startTransferIndex];
				if (sz<upt.size()) self->doerr(55);
				for (uint i=0;i<upt.size();i++) { tl[i]= upt[i]; };
				return static_cast<int>(upt.size());
			};
		};

		void gen( istate_t ist , int depth  ){
			if (ist==BADSTATE) return;
			cEState cs= self->getstate(ist);
			if (cs.isfinal()) { res.cntwords++; 
				format_to( res.txt , "%s r%d " , buff , cs.Rule );
				if (opt.urinf) opt.urinf->PutRuleInfo( *self , cs.Rule ,res.txt );
				format_to( res.txt , "%c" , opt.separator);
				return; }
			if (cs.startTransferIndex==c_DFA_NativeCode_Base::BAD_TINDEX) { return ;}
			if (statemasks[ist]) { res.loops++; return; }
			statemasks[ist]=true;
			cVectorC<unp_transfer,MB> ttbuff; ttbuff.sz =get_TTlst( ttbuff.data , ARRAYLEN(ttbuff) , cs );
			auto binsz=buff.size(); 
			cVectorC<char_t,MB> chlst; 
			cVectorC<istate_t,MB> astates; statesTTConvolution(astates , ttbuff , (cBitSetC<MB>*) 0 );
			
			for (uint si=0;si<astates.size();si++) { 
				buff.resize(binsz);
				Filtred_TT( chlst , ttbuff , astates[si] ); 
				if (opt.opt!=HEX) 
					fillchars_ttlist(  chlst , ~cs.pre_mask  );
				if (chlst.size()>1) {
					buff<<"[";
					for (uint k=0;k<chlst.size();k++) { format_to(buff , opt.charformat , chlst[k]);  };
					buff<<"]";
				}
				else format_to( buff , opt.charformat , chlst[0]  );
				if (cs.pre_mask!=0xFF && (opt.opt==HEX)) {  
					format_to(buff, " &%02x ",cs.pre_mask);
				};
				gen( astates[si] , depth+1 );  
			};
			statemasks[ist]=false;
		};
		void gen(  ){ 
			if (self->startstate==BADSTATE) return;
			statemasks.resize( self->states.size()); 
			gen( self->startstate , 0);
		};
	}; // cGenerationCtx
	c_DFA_NativeCode_Base::cResultGeneration c_DFA_NativeCode_Base::Generation( cOptionGeneration opg ) {
		cGenerationCtx ctx; ctx.self=this; ctx.opt= opg;
		ctx.gen();
		return ctx.res;
	};

}; // namespace tbgeneral{

//static tbgeneral::c_DFA_NativeCode_Base nnn;

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#endif

using namespace tbgeneral;
void test_DFA_NativeCode_Base(){
	byte masks[]={0xFE,0xFF,0xFF,0x00};
	uint64 masksf=0xFFFFFF ;
	const char *chartst1[]={
			"abc",
			"bcd" 
			,"cda"
	};  
	typedef c_DFA_NativeCode_Base::irule_t  irule_t;
	c_DFA_NativeCode_Base a1;
	c_DFA_NativeCode_Base::cResultGeneration gr[10];
	c_DFA_NativeCode_Base::cStats stats[10];
	int res[10]; int16 rules[10];
	for (size_t i=0;i<ARRAYLEN(chartst1);i++) {
		a1.add_In_word( masks, (byte*)chartst1[i] , (irule_t) i+1 , static_cast<int>(strlen(chartst1[i])) );
	};
	a1.add_In_word( (byte*)&masksf, (byte*)"agd" , 50 , 3 );
	a1.add_In_word( (byte*)&masksf, (byte*)"dxy" , 51 , 3 );
	gr[0]= a1.Generation( a1.GenASCI() );

	a1.pack();
	stats[0]=a1.getstats();
	gr[1]= a1.Generation( a1.GenASCI() );

	for (size_t i=0;i<ARRAYLEN(chartst1);i++) {
		res[i]=a1.find( chartst1[i] , rules[i] );
	};
	res[0]=a1.find( "xxy" , rules[0] );
	int i=0;

};