#pragma once

#include "gdata/t_sstruct.h"
#include "gdata/tb_numbers.h"

#include "gdata/tb_env.h"
#include "gdata/t_error.h"


namespace tbgeneral {

	struct conf_struct_t;
	using conf_struct_ref = rc_base_struct<conf_struct_t>;



	struct conf_variant_types {
		enum eVarType { ejtNone, ejtBool, ejtInt, ejtFloat, ejtStr, ejtObject, ejtArray };
		using str_t = stringA;
		using key_t = stringA;
		static stringA getvartype_name(eVarType tp);
		struct print_option_t {
			enum { eNOPRINT = 1 };
			int values_per_line; 
			int mask;
			print_option_t() { values_per_line = 0; mask = 0; }
			print_option_t(int vpl, int m = 0) { values_per_line = vpl; mask = m; }
		};
		enum eVarOpt { ejtNoPrint = print_option_t::eNOPRINT };


	};

	struct conf_variant : public conf_variant_types {
		byte type;
		byte opt;
		union var_type {
			str_t str;
			conf_struct_ref node;
			int64 i;
			bool b;
			double f;
			var_type() { };
			~var_type() {};
		};
		var_type d;

		void clear();
		str_t toString()const;

		int64 toInt() const;
		double toFloat() const;
		str_t toStr() const;
		bool toBool() const;
		stringW toStrW() const;
		conf_struct_ref toNode() const;

		str_t & refStr(); 


		void convert(int8_t& v)const { v = (int8_t)toInt(); }
		void convert(uint8_t& v)const { v = (uint8_t)toInt(); }
		void convert(int16_t& v)const { v = (int16_t)toInt(); }
		void convert(uint16_t& v)const { v = (uint16_t)toInt(); }
		void convert(int& v)const { v = (int)toInt(); }
		void convert(uint& v)const { v = (uint)toInt(); }
		void convert(float& v)const { v = float(toFloat()); }
		void convert(double& v)const { v = double(toFloat()); }
		void convert(bool& v) const { v = toBool(); };
		void convert(str_t& v)const { v = toStr(); };
		void convert(stringW& v)const { v = toStrW(); };
		void convert(conf_struct_ref& v)const { v = toNode(); };
		void convert(conf_variant& v)const { v.assign(*this); };

		void assign(const conf_variant& cv);

		template <class TRet> TRet get() const { TRet rv;  convert(rv); return rv; }

		conf_variant& operator =(const conf_variant& cv) { assign(cv); return *this; };

		conf_variant(const conf_variant& cv) { init(ejtNone); assign(cv); }
		conf_variant(int64_t v) { init(ejtInt); d.i = v; }
		conf_variant(uint64_t v) { init(ejtInt); d.i = v; }
		conf_variant(int v) { init(ejtInt); d.i = v; }
		conf_variant(uint v) { init(ejtInt); d.i = (int)v; }
		conf_variant(const char* v) { init(ejtStr); d.str = v; }
		conf_variant(const str_t& v) { init(ejtStr); d.str = v; }
		conf_variant(const conf_struct_ref& v);
		conf_variant(bool v) { init(ejtBool); d.b = v; }
		conf_variant(float v) { init(ejtFloat); d.f = v; }
		conf_variant(double v) { init(ejtFloat); d.f = float(v); }
		conf_variant();
		~conf_variant() { clear(); };
		conf_variant copy() const { return conf_variant(*this); };

		void init(byte tp);

		bool isObjectType() const { return type == ejtObject || type == ejtArray; }
	};



struct conf_struct_t : public conf_variant_types {
	
	struct r_property { key_t key;  conf_variant data; };

	struct iterator {
	private:
		const conf_struct_t* nod;
		size_t index;
	public:
		iterator(const conf_struct_t* _nod, size_t _index) { nod = _nod; index = _index; };
		bool operator < (const iterator& r) { return index < r.index; }
		bool operator == (const iterator& r) { return index == r.index; }
		bool operator != (const iterator& r) { return index != r.index; }
		iterator& operator ++ () { index++; return *this; }
		const iterator& operator * () const { return *this; } //TODO:!!
		const key_t& Key() const;
		const conf_variant& Value() const;
		const size_t Index() const { return index; };
	};
	// Получение итераторов 
	const iterator cbegin() const { return begin(); };
	const iterator cend() const { return end(); };
	const iterator begin() const;
	const iterator end() const;
	iterator begin();
	iterator end();


	print_option_t* print_option;
	eVarType stype;
	darray<conf_variant> a_data;
	darray<r_property> named_props; // elementtype not defined


protected:
	//const r_property* find_prop(const key_t& key) const;
	const conf_variant& get_ref(size_t index) const;
	const conf_variant& get_ref(const key_t& key) const;

public:
	conf_variant getvar(size_t index) const;
	conf_variant getvar(key_t key)const;

	// Получить количество элементов в узле (размер карты или массива)

	// ------------ Получение данных 
	// Карта. Получить указатель на данные по ключу. Если данных нет - вернет 0
	const conf_variant* get_ptr(size_t index) const;
	const conf_variant* get_ptr(const key_t& key) const;
	conf_variant* get_ptr(size_t index);
	conf_variant* get_ptr(const key_t& key);
	bool exists(const key_t& key) const {		return get_ptr(key) != 0; 	}
	const int find_index(const key_t& key) const;

	// Массив. Получить значение нужного типа по индексу
	template <class TRet > TRet get(size_t index) const { return get_ref(index).get<TRet>(); }
	// Карта. Получить значение нужного типа по ключу
	template <class TRet> TRet get(const key_t key) const { auto y = get_ptr(key);  return y ? y->get<TRet>() : TRet(); }
	// Карта. Получить значение нужного типа по ключу, в случае неудачи будет значение по умолчанию
	template <class TRet> TRet getdef(const key_t key, const TRet& def) const { auto y = get_ptr(key);   return y ? y->get<TRet>() : def; }
	// Карта.Получить значение нужного типа по ключу, через ссылку
	template <class TRet> bool get(const key_t key, TRet& v) const { auto y = get_ptr(key); if (y) v = y->get<TRet>();  return y != 0; }

	// Получить универсальное значение по ключу с путём. Путь - строка ключей разделенных символом /
	conf_variant * get_path_ptr(const key_t& path);
	conf_variant get_path(const key_t& path) const;
	// Получить значение совместимого типа по ключу с путём. Путь - строка ключей разделенных символом /
	template <class TRet> TRet get_path(const key_t key) const { auto y = get_path(key);  	return y.get<TRet>(); }
	int set_path(const key_t& path, conf_variant);
	//template <class TRet> int set_path(const key_t path, TRet v ) const {  set_path(key, conf_variant(v)); }

	// Получить значение совместимого типа по ключу. Если данных нет или данные несовпадающего типа - генетрируется исключение
	const conf_variant* get_demand(const key_t& key, eVarType stype) const;
	template <class TRet> TRet demand(const key_t key) {
		auto type = conf_variant(TRet()).type;
		auto v = get_demand(key , conf_variant::eVarType( type) );
		if (!v) throw "Invalid key";
		return v->get<TRet>();
	}



	// Добавить значение
	size_t push_back( const conf_variant& v);
	size_t push_back(const key_t& key , const conf_variant& v);
	// Установить значение
	void set(const key_t& key, const conf_variant& v);
	void set(size_t index, const conf_variant& v);

	void resetfield(const key_t& key) { set(key, conf_variant()); };

	template <class TMap> void set_map(const key_t key, TMap& m) {
		auto nn = newnode(ejtObject); nn->assign_map(m); 	set(key, nn);
	}
	template <class TArray> void set_array(const key_t key, const TArray& m) {
		auto nn = newnode(ejtArray); nn->assign_array(m); set(key, nn);
	}
	// Установить в карту значение совместимого типа по ключу
	//template <class TRet> size_t set(const key_t key, const TRet& v) { return set( key , conf_variant(v)); }
	//template <class TRet> size_t set(size_t index, const TRet& v) { return set(index, conf_variant(v)); }
	// Добавить в карту значение совместимого типа, по ключу (ключ может совпадать с уже существующими)

	// Карта. Получить узел по ключу, через ссылку
	conf_struct_ref getnode(const key_t key) const { return getdef<conf_struct_ref>(key, conf_struct_ref()); }
	// Массив. Добавить новый узел 
	conf_struct_ref add_node(eVarType jt = ejtObject) { auto r = newnode(jt); push_back(r); return r; }
	// Карта. Добавить новый узел . Игнорировать совпадение ключа
	conf_struct_ref add_node(const key_t key, eVarType jt = ejtObject) { auto r = newnode(jt); push_back(key, r); return r; }
	// Карта. Добавить новый узел или обновить существующий  (с ключом)
	conf_struct_ref set_node(const key_t key, eVarType jt = ejtObject) { auto r = newnode(jt); set(key, r); return r; }
	// Карта. Получить существующий или добавить новый узел  (с ключом)
	conf_struct_ref demand_node(const key_t key, eVarType jt = ejtObject) { auto r = getnode(key); return !r.empty() ? r : add_node(key, jt); }




	//--------- Получить массивы и карты
	template <class TRet> darray<TRet> get_array(const key_t key) {
		auto jn = get<conf_struct_ref>(key);
		return !jn.empty() ? jn->toArray<TRet>() : darray<TRet>();
	}
	template <class TRet> bool get_array(const key_t key, darray<TRet>& res) {
		auto jn = get<conf_struct_ref>(key);
		if (!jn.empty()) res = jn->toArray<TRet>();
		return !jn.empty();
	}
	template <class TMap> int get_map(const key_t key, TMap& m) {
		auto jn = get<conf_struct_ref>(key);
		return !jn.empty() ? jn->toMap(m) : 0;
	}


	//--------- Сохранить в узел массивы и карты
	template <class TMap> void toMap(TMap& m) {
		for (uint i = 0; i < named_props.size(); i++) {
			auto p = &named_props[i];
			p->data.convert(m[p->key]);
		};
		return;
	}
	template <class TRet> void toArray(darray<TRet>& res) {
		res.resize(a_data.size());
		for (uint i = 0; i < a_data.size(); i++) res[i] = a_data[i].get<TRet>();
	}
	template <class TRet> darray<TRet> toArray() { darray<TRet> res; toArray(res); return res; }

	template <class TMap> void assign_map(TMap& m) {
		settype(ejtObject);
		for(auto & p : m) 
			push_back(p.first, p.second);
	}
	template <class TArray> void assign_array(const TArray& m) {
		settype(ejtArray);
		a_data.resize(0); a_data.reserve(m.size());
		for(auto &p : m) a_data.push_back( p );
	}

	// создать новый узел заданного типа
	static conf_struct_ref newnode(eVarType jt = ejtObject);
	// создать копию узла
	conf_struct_ref copy() const;
	// Скопировать в себя заданный узел 
	void copy(const conf_struct_t * from);

	conf_struct_t(eVarType _stype) { stype = _stype; print_option=0; }
	conf_struct_t() { 		stype = ejtNone; print_option=0; }

	void erase(); // очистка внутренностей! size -> 0
	void erase(const key_t& _key);

	const size_t size() const { return a_data.size() ? a_data.size() : named_props.size(); };
	bool isObject() const { return stype == ejtObject; }
	bool isArray() const { return  stype == ejtArray; }
	void settype(eVarType _stype) { erase(); stype = _stype; }

}; // struct conf_struct_t

struct conf_parser_defs_based : public conf_variant_types {
	//using json_print_option = print_option_t;
	struct print_options {
		smap<print_option_t> shortopts;
	};

	enum {
		jeEVMustBeString = 1001,
		jeInvalidClosureEV,
		jeUnknownEnvVars,
		jeFileNotRead
	};
};

struct conf_parser_ctx_based : public conf_parser_defs_based {


	t_error_info error;
	conf_struct_ref root;
	stringA data; // 
	stringA datafilename; // последний загруженный файл
	LocalEnv locenv;

	void new_root();
	conf_struct_ref get(const stringA& path) const;
	int expand_all_env_vars();
	void clear();

	virtual int parsedata(const stringA& data)=0;
	virtual int parsefile(const stringA& filename);
	virtual int parse_DataOrFile(const stringA& filename);
	virtual bool FilenameIsData(const stringA& fn)=0;


};


}; // namespace tbgeneral {


