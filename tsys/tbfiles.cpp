#define _CROSS_SYS_COD
#include <cstdio>
#include <fstream>

#include "tsys/cross_sys.h"
#include "tsys/sys_mutex.h"
#include "gdata/tb_algo.h"
#include "gdata/tb_env.h"
#include "tsys_helper.h"

#include "tsys/tbfiles.h"
#include "tsys/tb_process.h"
#include "gdata/t_format.h"
#include "parse/t_path.h"

#include "time.h"

warning_MSVC(disable , 4996)

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-result"
#endif

inline uint64 DWORDS264( uint64 dwl , uint64 dwh ) { return dwl | (dwh<<32); }

//using stringA = tbgeneral::stringA;

//------------------------------


#ifdef _WINDOWS
#define NOMINMAX
#define LEAN_AND_MEAN
#include "windows.h"
#include <io.h>
#include <direct.h>
#elif defined(_LINUX)

#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>


#endif



namespace tbgeneral {
//using stringA= tbgeneral::stringA;  
//using stringW = tbgeneral::stringW;
//using param_stringA = tbgeneral::param_stringA;
using cdatetime = crsys::cdatetime;

stringW buffWfromAstr(const param_stringA& p, void* b, size_t bsize) {
	stringW r;
	r.assign_storage(b, bsize);
	r.assign(p.param);
	return r;
	//tbgeneral::convert(r, p.param);
}

}

namespace tbgeneral{ namespace filesystem {

#ifdef WIN32
	enum { TypeOf_filetime = cdatetime::eformat_win_FILETIME };
#else 
	enum { TypeOf_filetime = cdatetime::eformat_time_t };
#endif

static cdatetime filetime2cdatetime(uint64 v ){
  return cdatetime::convert( v , cdatetime::enum_timeformat( TypeOf_filetime ) ).data + cdatetime::localshift().data;
};




esyserror fileexist(const tbgeneral::param_stringA& fnm){
	r_fileinfo inf;
	esyserror err=file_getinfo( fnm , inf );
	return ( err ? err : ( inf.attr.b.etype ==efat_FILE ? esyse_OK : esyse_FILENOTEXIST));
};
esyserror direxist(const tbgeneral::param_stringA& fnm){
 r_fileinfo inf;
 esyserror err=file_getinfo( fnm , inf );
 return ( err ? err : ( inf.attr.b.etype ==efat_DIR ? esyse_OK : esyse_FILENOTEXIST));
};


void reopen_std_handles(const param_stringA& _sin, const param_stringA& _sout, const param_stringA& _serr){
	char buff[3][64];
	auto sin = _sin.param.tocstr(buff[0],sizeof(buff[0]));
	auto sout = _sout.param.tocstr(buff[1], sizeof(buff[1]));
	auto serr = _serr.param.tocstr(buff[2], sizeof(buff[2]));
	const char* sio[] = { sin.c_str() , sout.c_str() , serr.c_str() };
	if (sin.empty() ) sio[0] =NULL_FILE;
	if (sout.empty()) sio[1] = NULL_FILE;
	if (serr.empty()) sio[2] = sio[1];
	freopen(sio[0],"rt" , stdin); setbuf( stdin , 0 );
	freopen(sio[1],"w+b" , stdout); setbuf( stdout , 0 );
	freopen(sio[2],"w+b" , stderr); setbuf( stderr , 0 );
}





#ifdef _WINDOWS



time64_t FILETIME2t64( const FILETIME & f ){
	return DWORDS264( f.dwLowDateTime , f.dwHighDateTime );
}

void initattr( r_fileattributes & a , uint flags ){
	ZEROMEM(a);
	a.b.etype= ( flags & FILE_ATTRIBUTE_DIRECTORY ? efat_DIR : efat_FILE );
};

esyserror WinErrorToErr( uint err){
	switch (err) {
	case ERROR_FILE_NOT_FOUND : return esyse_FILENOTEXIST;
	case ERROR_INVALID_DRIVE:
	case ERROR_PATH_NOT_FOUND : return esyse_PATHNOTEXIST;
	case ERROR_ACCESS_DENIED: return esyse_ACCESSDENIED;
	case ERROR_NOT_ENOUGH_MEMORY: return esyse_NOT_ENOUTH_MEM;
	case ERROR_INVALID_PARAMETER: return esyse_INVALIDPARAM;
	default: return esyse_GENERALFAILURE; 
	};
}

void fillattr(r_fileinfo & inf , const WIN32_FILE_ATTRIBUTE_DATA & fad) {
	using namespace crsys; 
 inf.exist = true;
 initattr(inf.attr,  fad.dwFileAttributes );
 //inf.attr.b.etype= (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? efat_DIR : efat_FILE );
 //TODO: attributes
 inf.size = DWORDS264( fad.nFileSizeLow , fad.nFileSizeHigh );
 inf.crtime = filetime2cdatetime( FILETIME2t64(fad.ftCreationTime) );
 inf.atime  = filetime2cdatetime( FILETIME2t64(fad.ftLastAccessTime));
 inf.mtime  = filetime2cdatetime(FILETIME2t64(fad.ftLastWriteTime ));
 inf.ctime  = inf.mtime;
};



esyserror file_getinfo(const param_stringA& p, r_fileinfo & inf){
	wchar_t buff[256];
	auto wfn = buffWfromAstr(p,buff);
	//auto wfn = stringW(_fnm);
 WIN32_FILE_ATTRIBUTE_DATA fad; 
 if (!GetFileAttributesExW( wfn.c_str() ,  GetFileExInfoStandard , &fad)) 
	{ inf=r_fileinfo(); 
      return WinErrorToErr( GetLastError() ); 
	}
 fillattr( inf , fad);	
 return esyse_OK;
};


stringA get_programm_path(){
	//TOOD: utf8
	wchar buff[4*1024];
	if (!GetModuleFileNameW( 0 ,  buff , sizeof(buff) ))
		RAISE("error at get_programm_path");
	return  buff ;
};

esyserror setCurrentDir(const param_stringA& cd ){
	wchar_t buff[256];
	auto ws = buffWfromAstr(cd,buff);
		//stringW(cd);
	if (!SetCurrentDirectoryW( ws.c_str() )) 
		   return WinErrorToErr( GetLastError() ); 
	return esyse_OK;
};

stringA getCurrentDir() {
	uint rlen= GetCurrentDirectoryW( 0, 0  ); if (!rlen) return 0;
	stringW r; r.resize(rlen-1);
	GetCurrentDirectoryW(static_cast<DWORD>(r.size()+1) , r.data());
	return r;
};





#elif defined(_LINUX)


struct cdef_pathinitialize{
	static cdef_pathinitialize init;
	tbgeneral::stringA defprgpath;
	cdef_pathinitialize() {defprgpath=get_programm_path();};
};
cdef_pathinitialize cdef_pathinitialize::init;

esyserror linuxerrconvert( int err){
	switch (err) {
	case ENOENT: return esyse_FILENOTEXIST; //   Компонент полного имени файла file_name не существует или полное имя является пустой строкой. 
	case ENOTDIR:return esyse_PATHNOTEXIST; //    Компонент пути не является каталогом.	
	case ELOOP:  return esyse_LINKERROR; //    При поиске файла встретилось слишком много символьных ссылок. 
	case EFAULT: return esyse_INVALIDPARAM; //    Некорректный адрес. 
	case EACCES: return esyse_ACCESSDENIED;
	case ENOMEM: return esyse_NOT_ENOUTH_MEM;
	case ENAMETOOLONG:return esyse_INVALIDPARAM;
	default: return esyse_GENERALFAILURE; 
	};
};

stringA get_programm_path(){
	static stringA res;
	if (!res.size()) {
		AUTOENTERG();
		char buff[2048];
		int sz = readlink("/proc/self/exe", buff, sizeof(buff));
		res.assign( buff , sz );
	};	

	return res;
};
void initattr( r_fileattributes & a , uint flags ){
	ZEROMEM(a);
	a.b.etype =	(flags & S_IFREG ? efat_FILE : 
						(flags & S_IFDIR ? efat_DIR: 
						(flags & S_IFLNK? efat_LINK : efat_OTHER )) );
};

esyserror file_getinfo(const param_stringA& fnm , r_fileinfo & inf){
	char sbuff[256]; 
	auto fnm0 = fnm.param.tocstr(sbuff);
	struct stat buf;
	int res= stat( fnm0.c_str() , &buf); 
	if (res) { inf=r_fileinfo(); return linuxerrconvert(res); };
	initattr( inf.attr , buf.st_mode );
	inf.size = buf.st_size;	
 
	inf.crtime = 0;
	inf.atime  = filetime2cdatetime(buf.st_atime );
	inf.mtime  = filetime2cdatetime(buf.st_mtime);
	inf.ctime  = filetime2cdatetime(buf.st_ctime);
	inf.crtime = min<double>(inf.atime.data ,inf.ctime.data);

	return esyse_OK;
};

stringA getCurrentDir() {
// char *get_current_dir_name(void)
	return get_current_dir_name();
};


#endif

c_directory::entrylist c_directory::getentrys( int cnt ){
	bool full=cnt==-1; 
	c_directory::entrylist res; res.reserve(cnt>0?cnt:100);
	for (;cnt>0 || full;cnt--) {
		c_directory::c_entry e;
		if (!this->getentry( e )) break;
		if (e.fn == "." || e.fn == "..") continue;
		res.push_back( e );
	};
	return res;
};

#ifdef _WINDOWS

struct c_directory_win{
	enum { eInvalidHandle = -1 };
	intptr_t handle;
	uint num;
	struct _wfinddata_t first; 
	c_directory_win(const stringA & fullnm){
		num=0;
		stringW fnm ( fullnm );
		handle= _wfindfirst( fnm.c_str() , &first );
	};
	bool isopen() { return handle!=eInvalidHandle; }
	~c_directory_win() { 
		if (isopen()) 
			{ _findclose( handle); handle= eInvalidHandle; };
	};
	static c_directory_win* open( const stringA & fullnm ) {
		auto r = new c_directory_win(fullnm);
		if (!r->isopen()) { delete r; r=0; };
		return r;
	}
};

bool c_directory::open(const stringA& fullnm){
	stringA fnm = fullnm; 
	dir = fullnm; fnm<<"/*";
	//c_directory_win * r= new c_directory_win(fnm);
//	if (!r->handle) { delete r; r=0;}
	//if (!r->isopen()) { delete r; r=0;}
	//this->h = (void*)r;
	this->h = c_directory_win::open(fnm);
	return h!=0;
};
c_directory::~c_directory(){
	if (this->h) { 
		delete (c_directory_win*)h; this->h=0; 
	};
};
bool c_directory::getentry(c_entry & e){
	auto wh= (c_directory_win *) h;
	if (!wh) return false;
	struct _wfinddata_t data; 
	if (wh->num==0) { data = wh->first; } else 
	{ if (_wfindnext( wh->handle , &data )<0) return false; };
	initattr( e.type , data.attrib );
	e.fn =  data.name ; 
	wh->num++;
	return true;
};

#elif defined(_LINUX)
bool c_directory::open(const stringA& fullnm){
	char buff[256]; 
	auto name0 = fullnm.tocstr(buff);
	DIR * d = opendir( name0.c_str() );
	h = (void*) d;
	dir = fullnm; 
	return h!=0;
};
c_directory::~c_directory(){
	if (h) { closedir((DIR*)h); h=0; }
};
bool c_directory::getentry(c_entry & e){
	return false;
	if (!h) return false;
	struct dirent * de = readdir((DIR *) h);
	if (!de) return false;
	//TODO:
	//initattr( e.type , de->type );
	//e.fn = de->name; 
	return true;
};
#endif


bool getDirItems( const stringA & dnm , c_directory::entrylist * res , int cnt ) 
	{ c_directory d(dnm); *res = d.getentrys(cnt);  
		return d.isopen(); 
	};




stringA readstrfromfile( FILE * f ){
	stringA res;
	char buff[4*1024];ZEROMEM(buff);
	fseek(f,0,0);size_t szr;
	while (0!=(szr = fread( buff,1,sizeof(buff), f)) ) 
		res.append( buff , szr );
	return res;
}; // ---------------------

stringW readstrfromfileW( FILE * f ){
	stringW res;
	wchar_t buff[4*1024];ZEROMEM(buff);
	fseek(f,0,0);size_t szr;
	while (0!=(szr = fread( buff,1,sizeof(buff), f ))) {
		szr /= sizeof(wchar_t);
		wchar_t * s=buff; 
		if ( *( (uint16*)s) == 0xFEFF) {
			s++; szr--;}
		res.append( s ,  szr );
	};
	return res;
}; // ---------------------

struct file_rec_t {
	FILE* fp;
	file_rec_t(FILE* fp) { this->fp = fp; }
	~file_rec_t() { if (fp) fclose(fp); }
};

int64_t cmp_content_files(const stringA& lfnm, const stringA& rfnm) {
	file_rec_t lf = openFile(lfnm, "rb"); if (!lf.fp) return -1;
	file_rec_t rf = openFile(rfnm, "rb"); if (!lf.fp) return -2;
	enum { buff_sz = 16 * 1024 };
	char buff_l[buff_sz], buff_r[buff_sz];
	int64_t offs = 1;
	while (1) {
		auto sz_l = fread(buff_l, 1, sizeof(buff_l), lf.fp);
		auto sz_r = fread(buff_r, 1, sizeof(buff_r), rf.fp);
		if (sz_l != sz_r) return offs;
		if (sz_l == 0) return 0;
		if (memcmp(buff_l, buff_r, sz_l)) return offs;
		offs += sz_l;
	};
}


stringA readstrfromfile(const param_stringA& fn ){
	file_rec_t f = openFile( fn , "rb" );
	if (!f.fp) return 0;
	stringA rr= readstrfromfile(f.fp);
	return rr;
};

stringW readstrfromfileW(const param_stringA& fn){
	file_rec_t f = openFile(fn, "rb");
	if (!f.fp) return 0;
	stringW rr= readstrfromfileW(f.fp);
	return rr;
};





#ifdef _WINDOWS
int stream_open(std::ofstream& s, const param_stringA& fname, std::ios_base::openmode m) {
	wchar_t buff[256];
	auto wfname = buffWfromAstr(fname, buff);
	//s.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	s.open( wfname.c_str(), m);
	return s.is_open() ? 0 : 1;
}

int stream_open(std::fstream& s, const param_stringA& fname, std::ios_base::openmode m) {
	wchar_t buff[256];
	auto wfname = buffWfromAstr(fname, buff);
	//s.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	s.open(wfname.c_str(), m);
	return s.is_open() ? 0 : 1;
}

FILE* openFile(const param_stringA& fn , const param_stringA& mod) {
	if ( fn.param.empty()) return 0;
	wchar_t buff[256];  wchar_t buffmode[64];
	auto wfn = buffWfromAstr(fn, buff);
	auto wmod = buffWfromAstr(mod, buffmode);
	return  _wfopen( wfn.c_str() , wmod.c_str() );
}
#elif defined(_LINUX)
int stream_open(std::ofstream& s, const param_stringA& fname, std::ios_base::openmode m) {
	char buff[256];
	auto fname0 = fname.param.tocstr(buff) ;
	//s.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	s.open( fname0.c_str(), m);
	return s.is_open() ? 0 : 1;
}

int stream_open(std::fstream& s, const param_stringA& fname, std::ios_base::openmode m) {
	char buff[256];
	auto fname0 = fname.param.tocstr(buff) ;
	//s.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	s.open(fname0.c_str(), m);
	return s.is_open() ? 0 : 1;
}
FILE* openFile(const param_stringA& fn , const param_stringA& mod ) {
	char buff[256];  char buffmode[64];
	if (fn.param.empty() ) return 0;
	auto fn0 = fn.param.tocstr( buff);
	auto mod0 = mod.param.tocstr( buffmode );
	return  fopen( fn0.c_str() , mod0.c_str() );
}
#endif

int writedata2file( FILE* f, const void* d , size_t sz ){
	if (!f) return 0;
	fwrite( d , 1 , sz , f );
	//stringA rr= readstrfromfile(f);
	fclose(f);
	return 1;
};

int writedata2file(const param_stringA& fn, const void* d , size_t sz ){
	//auto zfn = fn.param.tocstr(buff );
	FILE* f =openFile( fn , "wb" );
	if (!f) { return 0; }
	int r= writedata2file( f , d , sz );
	fclose(f);
	return r;
};


int copyfile_d(const param_stringA& dst , const param_stringA& src ){
	int result=0;
	file_rec_t fsrc =openFile( src , "rb" );
	file_rec_t fdst =openFile( dst , "wb" );
	if (!fsrc.fp || !fdst.fp) return 1; 
	//TODO: optimize
	stringA buff = readstrfromfile( fsrc.fp );
	writedata2file( fdst.fp , buff.data() , buff.size() );
	return result;
};

/*
stringA get_programm_path(){
	return crsys::get_programm_path();
};
*/
stringA get_programm_dir(){
	return get_file_dir(get_programm_path());
};
stringA get_programm_dir_a(){
	//using namespace crsys;
	static stringA pdir=0;
	if (pdir.size()) { return pdir;}
	stringA nv=get_programm_dir();
	AUTOENTERG();
	 pdir=nv; 
	return pdir;
};

stringA get_programm_name(){
	return get_file_name(get_programm_path());
};

inline size_t strlen_s(const stringA & d) { return d.size(); }

template<class TArray> stringA findfile_t( const TArray & dirlist, uint cntdirs , const stringA & filename){
	for (uint i=0;i<cntdirs;i++) {
		auto sep = strlen_s(dirlist[i]) != 0 && filename.size() != 0 ? "/" : "";
		auto f= format( "%s%s%s" , dirlist[i], sep, filename );
		if (f.empty()) continue;
		f= expand_environment_string( f);
		if (0==fileexist( f.c_str() )) return f;
	}
	return 0;
}

stringA findfile( const char ** dirlist, uint cntdirs , const stringA & filename){
	return findfile_t(dirlist , cntdirs , filename);
};

stringA findfile( const darray<stringA> & dirlist, const stringA & filename){
	strlen_s(0);
	strlen_s(dirlist[0]);
	return findfile_t(dirlist , static_cast<uint>(dirlist.size()) , filename);
}

int foreach_files_i(const stringA & dirname , bool recurse , r_foreach_file & rfctx ){
	//crsys::direxist()
	if (0==fileexist( dirname )) {
		return rfctx.forfile( dirname ); 
	}
	c_directory d;
	if (!d.open( dirname )) { return 11; }
	c_directory::c_entry e;
	int err;
	while(1) {
		if (!d.getentry( e )) break;
		//efat_FILE=1,efat_DIR
		auto fullname= dirname + "/" + e.fn;
		switch ( e.type.b.etype ) {
		case efat_FILE :  
			//compress_file( fullname );
			if (0!=(err=rfctx.forfile( fullname ))) return err;
			break;
		case efat_DIR :  
			if (e.fn =="." || e.fn=="..") continue;
			if ( 0!=(err= foreach_files_i(fullname,recurse, rfctx) ))  return err;
			break;
		};
	};
	return 0;
};

//#include <functional>

int foreach_files(const stringA& basedirname, std::function<int(const stringA & fn, bool isdir)> ffun, int options ) {
	//crsys::direxist()
	std::function<int(const stringA& reldirname)> frnd;
	frnd = [&](const stringA& reldirname) {
		auto dir = join2path(basedirname , reldirname);
		c_directory d;
		if (!d.open(dir)) { return 11; }
		c_directory::c_entry e;
		int err;
		while (1) {
			if (!d.getentry(e)) break;
			//efat_FILE=1,efat_DIR
			auto relfilename = join2path( reldirname , e.fn );
			auto fullname = join2path(basedirname, relfilename );
			auto fnfor_fun = options & efReadDir_RelFileName ? relfilename : fullname;
			switch (e.type.b.etype) {
			case efat_FILE:
				//compress_file( fullname );
				if (0 != (err = ffun(fnfor_fun, false))) return err;
				break;
			case efat_DIR:
				if (e.fn == "." || e.fn == "..") continue;
				if (options & efReadDir_IncDirs) 
					if (0 != (err = ffun(fnfor_fun,true))) return err;
				if (0 != (err = frnd(relfilename)))  return err;
				break;
			};
		};
		return 0;
	};
	frnd("");

	return 0;
};

darray<stringA> readDirectory(const stringA& dirname, int options ){
	darray<stringA> res;
	auto fn = [&](const stringA& fn, bool isdir) { 
		res.push_back(fn); return 0;};
	foreach_files( dirname , fn , options);
	return res;
};

darray<stringA> get_all_files(const stringA & dirname , bool recurse ){
	struct rff : r_foreach_file {
		darray<stringA> resa;
		int forfile(const stringA & fullname) override { resa.push_back(fullname); return 0; };
	};
	rff ctx;
	foreach_files_i( dirname , recurse , ctx);
	return ctx.resa;
}

void DeleteFilesDir(const param_stringA& path ){
	//using c_directory = c_directory;
	c_directory d(path.param); c_directory::c_entry e;
	c_directory::entrylist l; //uint prevsz=0;
	stringA spath; spath<<path.param<<'/';
	while ( d.getentry(e) ) {
		if (e.fn=="." || e.fn=="..") continue;
		auto fp = spath+e.fn;
		delete_file( fp );
	};
};
void DeleteTempFilesDir(const param_stringA& path )
	{ DeleteFilesDir(path); };



int erase_dir(const stringA& path, int flags) {
	c_directory::entrylist list;
	if (0 != direxist( path ) ) return 0;
	auto r = getDirItems(path, &list, 100);
	if (!r) return -1;
	int err;
	for (auto& e : list) {
		auto sfull = path + "/" + e.fn;
		if (e.type.b.etype == efat_DIR) {
			err = erase_dir( sfull , flags );
			if (err) 
				 return err; 
			err= remove_epmty_direcory( sfull );
			if ((err && flags&1) || (flags & 0x10)) 
				c_printf("error (%d) on delete dir %s \n", err, sfull);
		}
		else {
			err = delete_file(sfull);
			if ((err && flags & 1) || (flags & 0x10)) 
				c_printf("error (%d) ondelete file %s \n", err, sfull );
		}
		if (err) 
			return err;
	}
	return 0;

};


int copy_dir(const stringA& dest, const stringA& src, int flags) {
	c_directory::entrylist list;
	auto r= getDirItems( src , &list , 100 );
	if (!r) {
		c_printf("error read dir %s\n", src );
		return -1;
	}
	int err;
	for (auto& e : list) {
		auto sfull = src + "/" + e.fn;
		auto dfull = dest + "/" + e.fn;
		if (e.type.b.etype == efat_DIR) {
			MakeDir(dfull);
			err=copy_dir(dfull, sfull , flags );
		} else {
			err=copy_file(dfull, sfull);
			if (flags & 2) c_printf("copy file %s -> %s\n", sfull, dfull);
		}
		if (err) return err;
	}
	return 0;
}



#ifdef _WINDOWS

int remove_epmty_direcory(const stringA& w) {
	auto path = stringW(w);
	if (!RemoveDirectoryW(path.c_str()))
		return GetLastError();
	return 0;
}

int copy_file( const stringA & _dest , const stringA& _src ) {

	auto dest = stringW(_dest); 
	auto src = stringW(_src);
	auto br = CopyFileExW(src.c_str(), dest.c_str(), 0, 0, 0, 0);
	if (!br) return GetLastError();
	return 0;
}

int delete_file( const stringA & fn ) {
	stringW ws(fn);
	auto res = DeleteFileW(ws.c_str());
	return  res ? 0 : GetLastError();
}

FILE* makeTempFile_e(const param_stringA& fntemplate, stringA & outfn ){
	static int localcounter=0;
	int locc = crsys::_InterlockedIncrement(&localcounter);
    stringA tmpd= getenv("tmp") , tmpf;
	r_split_path sp = split_path( fntemplate.param );
	if ( sp.ddir.size()==0 ) RAISE("makeTempFile_e bad parametr fntemplate");
	tmpd<<'/'<<sp.ddir;
	if (direxist(tmpd)) if (!MakeDir( tmpd )) RAISE("makeTempFile_e unable to create dir");;
	tmpf= format("%s/%s_%d_%x_u_%d.%s", tmpd , sp.fname , crsys::getpid(), crsys::getTickCount(), locc , sp.ext ) ;
	FILE * fp = openFile( tmpf , "w+b" );
	outfn = tmpf;
	return fp;
};

FILE* makeTempFile(const param_stringA& fntemplate , stringA & outfn ){
	r_split_path sp = split_path( fntemplate.param );
	stringA fn;
	fn= format("%s_%x_u_.%s", sp.fname , crsys::getTickCount() , sp.ext ) ;
	char * dd= _tempnam( "", fn.c_str() );
	outfn = dd; free(dd);
	FILE* fp;
	fp=openFile( outfn , "w+bD" );
	return fp;
}

static int win_makedir(const wchar * dir) {
	if (CreateDirectoryW( dir , 0 )) return 0;
	auto e = GetLastError();
	if (e == ERROR_ALREADY_EXISTS ) return 0;
	return e;
}

bool MakeDir(const param_stringA & dir){
	wchar_t buff[256];
	auto ws = buffWfromAstr(dir,buff);
	//stringW ws( dir );
	auto ds= ws.data(), f = ds + ws.size();
	if (ds==f) return false;
	if (0==win_makedir(ds)) return true;
	bool isnetpath=isSlash(ds[0])||isSlash(ds[1]);
    int counter=0; 
	*f='/';f++;
	for( auto pc=ds;pc<f;pc++) {
		if (!isSlash(*pc)) continue;
		if (pc==ds||isSlash(pc[-1])) continue; 
		counter++;
		if (counter<3 && isnetpath) continue;
		*pc=0; 
		if (0!=win_makedir(ds)) return false;
		*pc='/';
	}
	return true;
};

static int deleteDirW( const stringW & path ){
	//rmdir("");
	//rm();
	throw 1;
}



#endif

#ifdef _LINUX
int delete_file( const stringA & fn ) {
	//TODO:
	remove( fn.c_str() );
	return 0;
}

FILE* makeTempFile( const char * fntemplate ,  stringA & outfn ){
	//FILE * fp=fopen( fntemplate , "w+bD" );
	//unlink( fntemplate );
	FILE * fp=tmpfile();
	outfn.resize(0);
	if (fp) outfn = format("/proc/%d/fd/%d",getpid(),fileno(fp) );
	{	char buff[2048];
		//int sz = 
			readlink(outfn.c_str(), buff, sizeof(buff));
		//printf("make temp file org='%s' link='%s'\n" ,buff , outfn.c_str());
	}
	return fp;
};

FILE* makeTempFile_e( const char * fntemplate , stringA & outfn ){
    return 	makeTempFile(fntemplate , outfn);
};
bool MakeDir(const param_stringA& dir){
	char buff[256];
	auto dir0 = dir.param.tocstr(buff);
	return 0==mkdir( dir0.c_str() , uint(-1));
};
int remove_epmty_direcory(const stringA& dir) {
	char buff[256];
	auto dir0 = dir.tocstr(buff);
	int res = rmdir(dir0.c_str());
	return  res;
}
int copy_file(const stringA& _dest, const stringA& _src) {
	char buff_s[256]; char buff_d[256];
	auto src = _src.tocstr(buff_s);
	auto dst = _dest.tocstr(buff_s);
	std::error_code e;
	std::filesystem::copy(src.c_str(), dst.c_str(), e);
	return e.value();
}

#endif

}}; // namespace tbgeneral::filesystem  



