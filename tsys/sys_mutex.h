#pragma once
//void __tmp_mutex_l( int lock , int index );

#define MakeAutoVarName___2( nm , suff ) nm##suff 
#define MakeAutoVarName___1( nm , suff ) MakeAutoVarName___2( nm , suff ) 
#define MakeAutoVarName( nm  ) MakeAutoVarName___1(nm , __COUNTER__)

#define AUTOENTER(a) entrleave_scope_real MakeAutoVarName(f__syncscope)(a)
#define AUTOENTERG() entrleave_scope_real f__syncscope_gen


class entrleave_scope_base {
public:
  virtual void enter() = 0;
  virtual void leave() = 0;
};


template <typename t_loc>
class entrleave_scope: public entrleave_scope_base {
protected:
	t_loc * loc;
	virtual void enter() {		loc->enter();	}
	virtual void leave() {		loc->leave();	}
public:
	entrleave_scope(t_loc & aloc) {		loc=&aloc;		enter();	}
	~entrleave_scope() {		leave();	}
};


class entrleave_scope_real {
	// здесь параметр шаблоне не важен т.к. не влияет на размер
	// класса (там указатель на тип параметра шаблона)
	char buff[sizeof(entrleave_scope<entrleave_scope_base>)];
	template <typename t_loc>	void init(t_loc * aloc) {new (buff) entrleave_scope<t_loc>(*aloc);}
public:
	template <typename t_loc>	entrleave_scope_real(t_loc & aloc) {		init<t_loc>(&aloc);	}
	template <typename t_loc>	entrleave_scope_real(t_loc * aloc) {		init<t_loc>(aloc);	}
	entrleave_scope_real( int mutex_id );
	entrleave_scope_real(); // general data copy mutex
	~entrleave_scope_real() {		((entrleave_scope_base*)buff)->leave();	}
};


namespace crsys{
class sys_mutex_base{
protected:	
	void * data;
public:
	void enter();
	void leave();
	sys_mutex_base(){};
	~sys_mutex_base(){};
private:
	sys_mutex_base(const sys_mutex_base & s) {}; // копирование запрещено
	sys_mutex_base & operator = (const sys_mutex_base & s) { return *this;}; // копирование запрещено
};
struct sys_mutex: public sys_mutex_base{ 
	sys_mutex();
	~sys_mutex();
};
struct sys_mutex_c: public sys_mutex_base{ 
	sys_mutex_c();
	~sys_mutex_c();
};

//typedef sys_mutex_base sys_mutex;
//typedef sys_mutex_base<1> sys_mutex_c;

}; // namespace crsys
