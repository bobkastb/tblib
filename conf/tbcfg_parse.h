#pragma once

#include "gdata/tb_env.h"
#include "gdata/t_error.h"
#include "conf_variant.h"

namespace tbgeneral {

//hocon file format  https://github.com/lightbend/config/blob/master/HOCON.md
struct hocon_parse_ctx : conf_parser_ctx_based {
	int parsedata(const stringA& data)  override;
	bool FilenameIsData(const stringA& fn) override;
private:
	void fixup_option(print_options* opts, bool doset);
};


struct ini_parse_ctx : conf_parser_ctx_based {
	int parsedata(const stringA& data)  override;
	bool FilenameIsData(const stringA& fn) override;
};


}; // namespace tbgeneral {