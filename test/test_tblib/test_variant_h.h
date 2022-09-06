#pragma once

#include "conf/json_parser.h"

namespace tbgeneral { namespace test_ns {


conf_struct_ref _list( const darray<conf_variant> & l );
conf_struct_ref _obj( const darray<conf_struct_t::r_property> & l );
int cmpvalue(test_state& err , int index, const conf_variant & n ,  const conf_variant& eta  );
int cmpnode(test_state& err , int index, const conf_struct_ref& n , const conf_struct_ref& eta  );

using f_foreach_conv_variant = std::function<bool(const conf_struct_ref& own,const stringA& k, const conf_variant& v)>;
void foreach_confv(const conf_struct_ref& n, f_foreach_conv_variant forv);

}};