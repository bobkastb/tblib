#pragma once
#include <functional>
#include "tb_basetypes.h"
#include "iostream"

#include "tsys/systime.h"
#include "gdata/t_string.h"


//namespace filesystem = tbgeneral;

namespace tbgeneral{ namespace filesystem{

#ifdef _WIN32
#define NULL_FILE "nul"
#else
#define NULL_FILE "/dev/null"
#endif


enum esyserror{
	esyse_OK = 0,
	esyse_GENERALFAILURE,
	esyse_INVALIDPARAM,
	esyse_NOT_ENOUTH_MEM,

	esyse_FILENOTEXIST = 0x10000,
	esyse_PATHNOTEXIST ,
	esyse_LINKERROR,

	esyse_ACCESSDENIED = 0x20000,
};

enum efileattribute{
	efat_FILE=1,efat_DIR, efat_LINK, efat_OTHER
};

union r_fileattributes{
	struct rb_fileattributes{ 
		uint etype:3; //efat_FILE=1,efat_DIR, efat_LINK,
	};
	rb_fileattributes b;
	uint dw;
};

//TODO: Нужно переделать cdatetime на msunixtime_t
struct r_fileinfo{
	using cdatetime = crsys::cdatetime;
	bool exist;
	r_fileattributes attr;
    uint64			size;  // размер файла в байтах
    cdatetime		atime;    // время последнего доступа
    cdatetime       mtime;    // время последней модификации
    cdatetime       ctime;    // время создания
    cdatetime       crtime;   //TODO: минимальное между ctime и atime. Удалить?
	r_fileinfo() { RESETMEM(*this); }
};

struct FILEauto{
	FILE* d;
	FILEauto() { d=0; }
	FILEauto(FILE* _d) { d=_d; }
	~FILEauto() { if (d) fclose(d); }
	};


struct c_directory{
	struct c_entry{  
		stringA fn;
		r_fileattributes type;
	};
	typedef darray<c_entry> entrylist;
	void * h;
	stringA dir;
	c_directory(const stringA & fullnm) { h = 0; open(fullnm); };
	c_directory() {h=0;};
	//void reopen();
	bool isopen() { return h!=0; }
	bool open(const stringA& fullnm);
	bool getentry(c_entry & e);
	entrylist getentrys( int cnt=-1 );
	~c_directory();
};


struct r_foreach_file { virtual int forfile(const stringA & fullname)=0; };
template <class TOwner> struct v_foreach_file : r_foreach_file { 
	typedef int (TOwner::*t_function)(const stringA & fullname);
	TOwner * own; t_function ffunc;
	v_foreach_file( TOwner * _own, t_function _ffunc) { own=_own;  ffunc=_ffunc; }
	int forfile(const stringA & fullname) override{ return (own->*ffunc)(fullname);}; 
	int foreach_files(const stringA & dirname , bool recurse ){
		return foreach_files_i(dirname , recurse , *this );
	};
};


int foreach_files_i(const stringA & dirname , bool recurse , r_foreach_file & rfctx );
darray<stringA> get_all_files(const stringA& dirname, bool recurse);

int foreach_files(const stringA& basedirname, std::function<int(const stringA& fn, bool isdir)> ffun, int options);
enum { efReadDir_Recurse=1 , efReadDir_RelFileName=2 , efReadDir_IncDirs = 4 };
darray<stringA> readDirectory(const stringA& dirname,  int options= efReadDir_Recurse );
bool getDirItems(const stringA& dnm , c_directory::entrylist * res , int cnt=-1 );



esyserror file_getinfo( const param_stringA & fnm , r_fileinfo & inf);
esyserror fileexist(const param_stringA& fnm);
esyserror direxist(const param_stringA& fnm);
stringA getCurrentDir();
esyserror setCurrentDir(const param_stringA& cd );
stringA get_programm_path();
stringA get_programm_path();
stringA get_programm_dir();
stringA get_programm_dir_a();
stringA get_programm_name();

void reopen_std_handles( const param_stringA& sin , const param_stringA& sout , const param_stringA& serr );






bool MakeDir(const param_stringA & dir);
FILE* makeTempFile(const param_stringA& fntemplate , stringA & outfn );
FILE* makeTempFile_e(const param_stringA& fntemplate , stringA & outfn );
void DeleteTempFilesDir(const param_stringA& path );
void DeleteFilesDir(const param_stringA& path );
int delete_file( const stringA & path );
int erase_dir( const stringA & path , int flags=0xFF );
int remove_epmty_direcory(const stringA& path);

int copy_file(const stringA& dest, const stringA& src);
int copy_dir(const stringA& dest, const stringA& src , int flags); // 1- recurse
int copyfile_d(const param_stringA& dst, const param_stringA& src);


int stream_open(std::ofstream& s, const param_stringA& fname, std::ios_base::openmode m);
int stream_open(std::fstream& s, const param_stringA& fname, std::ios_base::openmode m);
FILE* openFile(const param_stringA& fn , const param_stringA& mode);
int writedata2file( FILE* f, const void* d , size_t sz );
int writedata2file(const param_stringA& fn, const void* d, size_t sz);
stringA readstrfromfile( FILE * f );
stringW readstrfromfileW( FILE * f );
stringA readstrfromfile(const param_stringA& fn  );
stringW readstrfromfileW(const param_stringA& fn  );
int64_t cmp_content_files(const stringA& lf, const stringA& rf);

template <class Data_t> int writestr2file(const param_stringA& fn, const Data_t & s ){
		return writedata2file( fn , s.data() , s.size()*sizeof(typename Data_t::value_type) );
};


stringA findfile( const darray<stringA> & dirlist, const stringA & filename);
stringA findfile( const char ** dirlist, uint cntdirs , const stringA & filename);

//stringA filename_to_utf8( const stringA & fn);
//stringA utf8_to_filename( const stringA & fn);



}}; // namespace tbgeneral::filesystem{

namespace filesystem = tbgeneral::filesystem;
namespace rfilesystem = tbgeneral::filesystem;

