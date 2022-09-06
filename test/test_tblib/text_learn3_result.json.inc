namespace ns_aclearn4{
static const char *  ac_setting=u8R"const(
{
"learn":{"path": "./data/text/en_ru/lt1.txt" },
"outfile":"./data/freqs/testac_result.json",
"ac_params":{"start_alpha":1,"alpha_has_terminal_char":1,"minimum_frequency":1, "rate_endchar":0.0005 },
"alphabets":[
	{ 	"id":1,
		"lang":"en", 
		"symbols":8, 
		"alphabet":" ,.abcde",
		"ngramm":2}
	]
}
)const";	

// )const" u8R"const(

static const char *  ac_learn_data=u8R"const(
	abcde abcde abcde abcde abcde abcde
	abcde.abcde.abcde.abcde.abcde.abcde
	aaaaa,aaaaa,aaaaa,aaaaa,aaaaa,aaaaa,
	)const";

static const char *  ac_learn_result=u8R"const({
  "source":{
      "totalchars":114,
      "badchars":8,
      "totalfiles":0,
      "errored_files":0,
      "count_symbols":9,
      "srcdir":"./data/text/en_ru/lt1.txt"    },
  "ac_params":{
      "start_alpha":1,
      "alpha_has_terminal_char":1,
      "minimum_frequency":1    },
  "alphabets":[
      {
          "id":1,
          "name":"en",
          "ngramm":2,
          "Single":false,
          "symbols":8,
          "cnt_alpha":1,
          "alphabet":" ,.abcde",
          "transition_0":{
              "summrate":106,
              "alpha":[0.000061],
              "chars":{
                  " ":0.047134, ",":0.056536, ".":0.047134, "a":0.396117, "b":0.113133, "c":0.113133, "d":0.113133, "e":0.113133
                },
              "terminal":0.000488            },
          "transitions":{
              " ":{
                  "summrate":5,
                  "alpha":[0.000061],
                  "chars":{
                      " ":0.000061, ",":0.000061, ".":0.000061, "a":0.999024, "b":0.000061, "c":0.000061, "d":0.000061, "e":0.000061
                    },
                  "terminal":0.000488                },
              ",":{
                  "summrate":5,
                  "alpha":[0.000061],
                  "chars":{
                      " ":0.000061, ",":0.000061, ".":0.000061, "a":0.999024, "b":0.000061, "c":0.000061, "d":0.000061, "e":0.000061
                    },
                  "terminal":0.000488                },
              ".":{
                  "summrate":5,
                  "alpha":[0.000061],
                  "chars":{
                      " ":0.000061, ",":0.000061, ".":0.000061, "a":0.999024, "b":0.000061, "c":0.000061, "d":0.000061, "e":0.000061
                    },
                  "terminal":0.000488                },
              "a":{
                  "summrate":42,
                  "alpha":[0.000061],
                  "chars":{
                      " ":0.000061, ",":0.142726, ".":0.000061, "a":0.570967, "b":0.285453, "c":0.000061, "d":0.000061, "e":0.000061
                    },
                  "terminal":0.000488                },
              "b":{
                  "summrate":12,
                  "alpha":[0.000061],
                  "chars":{
                      " ":0.000061, ",":0.000061, ".":0.000061, "a":0.000061, "b":0.000061, "c":0.999024, "d":0.000061, "e":0.000061
                    },
                  "terminal":0.000488                },
              "c":{
                  "summrate":12,
                  "alpha":[0.000061],
                  "chars":{
                      " ":0.000061, ",":0.000061, ".":0.000061, "a":0.000061, "b":0.000061, "c":0.000061, "d":0.999024, "e":0.000061
                    },
                  "terminal":0.000488                },
              "d":{
                  "summrate":12,
                  "alpha":[0.000061],
                  "chars":{
                      " ":0.000061, ",":0.000061, ".":0.000061, "a":0.000061, "b":0.000061, "c":0.000061, "d":0.000061, "e":0.999024
                    },
                  "terminal":0.000488                },
              "e":{
                  "summrate":12,
                  "alpha":[0.000061],
                  "chars":{
                      " ":0.416306, ",":0.000061, ".":0.416306, "a":0.166534, "b":0.000061, "c":0.000061, "d":0.000061, "e":0.000061
                    },
                  "terminal":0.000488                }            }        }
    ]})const";

} // namespace ns_aclearn4{

namespace ns_aclearn3{
static const char *  ac_setting=u8R"const(
{
"learn":{"path": "./data/text/en_ru/lt1.txt" },
"outfile":"./data/freqs/testac_result.json",
"ac_params":{"start_alpha":1,"alpha_has_terminal_char":1,"minimum_frequency":1, "rate_endchar":0.0005 },
"alphabets":[
	{ 	"id":1,
		"lang":"en", 
		"symbols":8, 
		"alphabet":" ,.abcde",
		"ngramm":2}
	,{ 	"id":2,
		"lang":"Цифры",
		"symbols":14, "1/14" : 0.07142,
		"alphabet":" .:-0123456789"}
	]
}
)const";	

// )const" u8R"const(
static const char *  ac_learn_data=u8R"const(
	abcde abcde abcde abcde abcde abcde
	0123456789.0123456789.0123456789.0123456789.
	abcde.abcde.abcde.abcde.abcde.abcde
	aaaaa,aaaaa,aaaaa,aaaaa,aaaaa,aaaaa,
	)const";

static const char *  ac_learn_result=u8R"const({
  "source":{
      "totalchars":160,
      "badchars":10,
      "totalfiles":0,
      "errored_files":0,
      "count_symbols":21,
      "srcdir":"./data/text/en_ru/lt1.txt"    },
  "ac_params":{
      "start_alpha":1,
      "alpha_has_terminal_char":1,
      "minimum_frequency":1    },
  "alphabets":[
      {
          "id":1,
          "name":"en",
          "ngramm":2,
          "Single":false,
          "symbols":8,
          "cnt_alpha":2,
          "alphabet":" ,.abcde",
          "transition_0":{
              "summrate":107,
              "alpha":[0.000061,0.009340],
              "chars":{
                  " ":0.046700, ",":0.056041, ".":0.046700, "a":0.392345, "b":0.112081, "c":0.112081, "d":0.112081, "e":0.112081
                },
              "terminal":0.000488            },
          "transitions":{
              " ":{
                  "summrate":5,
                  "alpha":[0.000061,0.000061],
                  "chars":{
                      " ":0.000061, ",":0.000061, ".":0.000061, "a":0.998963, "b":0.000061, "c":0.000061, "d":0.000061, "e":0.000061
                    },
                  "terminal":0.000488                },
              ",":{
                  "summrate":5,
                  "alpha":[0.000061,0.000061],
                  "chars":{
                      " ":0.000061, ",":0.000061, ".":0.000061, "a":0.998963, "b":0.000061, "c":0.000061, "d":0.000061, "e":0.000061
                    },
                  "terminal":0.000488                },
              ".":{
                  "summrate":5,
                  "alpha":[0.000061,0.000061],
                  "chars":{
                      " ":0.000061, ",":0.000061, ".":0.000061, "a":0.998963, "b":0.000061, "c":0.000061, "d":0.000061, "e":0.000061
                    },
                  "terminal":0.000488                },
              "a":{
                  "summrate":42,
                  "alpha":[0.000061,0.000061],
                  "chars":{
                      " ":0.000061, ",":0.142718, ".":0.000061, "a":0.570932, "b":0.285435, "c":0.000061, "d":0.000061, "e":0.000061
                    },
                  "terminal":0.000488                },
              "b":{
                  "summrate":12,
                  "alpha":[0.000061,0.000061],
                  "chars":{
                      " ":0.000061, ",":0.000061, ".":0.000061, "a":0.000061, "b":0.000061, "c":0.998963, "d":0.000061, "e":0.000061
                    },
                  "terminal":0.000488                },
              "c":{
                  "summrate":12,
                  "alpha":[0.000061,0.000061],
                  "chars":{
                      " ":0.000061, ",":0.000061, ".":0.000061, "a":0.000061, "b":0.000061, "c":0.000061, "d":0.998963, "e":0.000061
                    },
                  "terminal":0.000488                },
              "d":{
                  "summrate":12,
                  "alpha":[0.000061,0.000061],
                  "chars":{
                      " ":0.000061, ",":0.000061, ".":0.000061, "a":0.000061, "b":0.000061, "c":0.000061, "d":0.000061, "e":0.998963
                    },
                  "terminal":0.000488                },
              "e":{
                  "summrate":12,
                  "alpha":[0.000061,0.083242],
                  "chars":{
                      " ":0.416331, ",":0.000061, ".":0.416331, "a":0.083242, "b":0.000061, "c":0.000061, "d":0.000061, "e":0.000061
                    },
                  "terminal":0.000488                }            }        },
      {
          "id":2,
          "name":"Цифры",
          "ngramm":1,
          "Single":false,
          "symbols":14,
          "cnt_alpha":2,
          "alphabet":" .:-0123456789",
          "transition_0":{
              "summrate":45,
              "alpha":[0.022161,0.000061],
              "chars":{
                  " ":0.000061, ".":0.088828, ":":0.000061, "-":0.000061, "0":0.088828, "1":0.088828, "2":0.088828, "3":0.088828, "4":0.088828, "5":0.088828, "6":0.088828, "7":0.088828, "8":0.088828, "9":0.088828
                },
              "terminal":0.000488            }        }
    ]})const";
	
};//namespace ns_aclearn3{