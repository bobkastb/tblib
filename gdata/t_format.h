#pragma once
#include "t_sstream.h"

namespace tbgeneral {
    namespace format_ctx_ns {
        enum {
            eff_LeftJustity = 1, eff_ForcedSignValue = 2, eff_ForcedSignBlank = 4, eff_Lattice = 8, eff_ZeroLeftPad = 0x10
            , eff_AddWidthParam = 0x20, eff_AddPrecParam = 0x40
        };
        struct format_ctx_specify {
            int flags, width, prec, length;
            int specifer;
        };


        template<typename TChar> struct format_ctx {
        private:
            basic_orcstringstream<TChar> ss;
        public:
            format_ctx_specify fspec;
            rc_string<TChar> fmtstr, currfmtspec;
            std::basic_ostream<TChar>* os;
            bool isFinshed;
            int cntAddParams;
            //format_ctx()
            format_ctx(const rc_string<TChar>& fmt, std::basic_ostream<TChar>* _os) { init(fmt, _os); }
            format_ctx(const rc_string<TChar>& fmt, void* localbuff, size_t buffsz) {
                ss.setstart_localbuff(localbuff, buffsz);
                init(fmt, &ss);
            }
            rc_string<TChar> str() { return ss.str(); };
            const rc_string<TChar>& nosecstr() { return ss.nosecstr(); };
        private:
            void init(const rc_string<TChar>& fmt, std::basic_ostream<TChar>* _os) {
                os = _os;
                fspec = { 0 };
                cntAddParams = 0;
                fmtstr = fmt;
                isFinshed = false;
            }
        };

        int format_parse(format_ctx<char>& ctx, bool fin);
        int format_out_data_int(format_ctx<char>& ctx, int32_t d);
        int format_out_data_int(format_ctx<char>& ctx, int64_t d);
        int format_out_data_float(format_ctx<char>& ctx, double d);
        int format_out_data_string(format_ctx<char>& ctx, const char* s, size_t cnt);
        int format_out_data_string(format_ctx<char>& ctx, const wchar_t* s, size_t cnt);
        int format_out_data_string(format_ctx<wchar_t>& ctx, const char* s, size_t cnt);
        int format_out_data_string(format_ctx<wchar_t>& ctx, const wchar_t* s, size_t cnt);


        template<typename TChar> int format_out_data(format_ctx<TChar>& ctx, int8_t d) { return format_out_data_int(ctx, d); }
        template<typename TChar> int format_out_data(format_ctx<TChar>& ctx, uint8_t d) { return format_out_data_int(ctx, d); }
        template<typename TChar> int format_out_data(format_ctx<TChar>& ctx, int16_t d) { return format_out_data_int(ctx, d); }
        template<typename TChar> int format_out_data(format_ctx<TChar>& ctx, uint16_t d) { return format_out_data_int(ctx, d); }
        template<typename TChar> int format_out_data(format_ctx<TChar>& ctx, int32_t d) { return format_out_data_int(ctx, d); }
        template<typename TChar> int format_out_data(format_ctx<TChar>& ctx, uint32_t d) { return format_out_data_int(ctx, static_cast<int32_t>(d)); }
        template<typename TChar> int format_out_data(format_ctx<TChar>& ctx, int64_t d) { return format_out_data_int(ctx, d); }
        template<typename TChar> int format_out_data(format_ctx<TChar>& ctx, uint64_t d) { return format_out_data_int(ctx, static_cast<int64_t>(d) ); }
        template<typename TChar> int format_out_data(format_ctx<TChar>& ctx, float d) { return format_out_data_float(ctx, d); }
        template<typename TChar> int format_out_data(format_ctx<TChar>& ctx, double d) { return format_out_data_float(ctx, d); }
        template<typename TChar, typename SChar> int format_out_data(format_ctx<TChar>& ctx, const SChar* d) { return format_out_data_string(ctx, d, strlen_s(d)); }
        template<typename TChar, typename SChar> int format_out_data(format_ctx<TChar>& ctx, const rc_string<SChar>& d) { return format_out_data_string(ctx, d.cbegin(), d.size()); }
        template<typename TChar, typename SChar> int format_out_data(format_ctx<TChar>& ctx, const std::basic_string<SChar>& d) { return format_out_data_string(ctx, d.cbegin(), d.size()); }


        template<typename TChar, typename Dtype> int format_out_data(format_ctx<TChar>& ctx, const Dtype& d) {
            *ctx.os << d; return 0;
        };

        template <typename TChar, typename... Rest> int formatRec(format_ctx<TChar>& ctx) { return format_parse(ctx, true); }
        template <typename TChar, typename First, typename... Rest> int formatRec(format_ctx<TChar>& ctx, const First& first, const Rest&... rest) {
            if (ctx.cntAddParams == 0) {
                format_parse(ctx, false);
                if (ctx.fspec.specifer == 0) { return -1; } //invald format
            }
            if (ctx.fspec.flags & eff_AddWidthParam) {
                ctx.cntAddParams = 1; //TODO: ctx.fspec.width = first;
            }
            else if (ctx.fspec.flags & eff_AddPrecParam) {
                ctx.cntAddParams = 1; //TODO:ctx.fspec.prec = first;
            }
            else {
                format_out_data(ctx, first);
                ctx.cntAddParams = 0;
            }
            return formatRec(ctx, rest...);
        }
    }; // format_ctx_ns 

    template <typename TChar, typename... Rest> std::basic_ostream<TChar>& format_to(std::basic_ostream<TChar>& ress, const rc_string<TChar>& fmt, const Rest&... rest) {
        format_ctx_ns::format_ctx<TChar> ctx(fmt, &ress);
        format_ctx_ns::formatRec(ctx, rest...);
        return ress;
    }
    template <typename TChar, typename... Rest> std::basic_ostream<TChar>& format_to(std::basic_ostream<TChar>& ress, const TChar* fmt, const Rest&... rest) {
        format_ctx_ns::format_ctx<TChar> ctx(fmt, &ress);
        format_ctx_ns::formatRec(ctx, rest...);
        return ress;
    }


    template <typename TChar, typename... Rest> void format_to(rc_string<TChar>& res, const rc_string<TChar>& fmt, const Rest&... rest) {
        char Buffer[4 * 1024];
        format_ctx_ns::format_ctx<TChar> ctx(fmt, Buffer, sizeof(Buffer));
        format_ctx_ns::formatRec(ctx, rest...);
        auto& nss = ctx.nosecstr();
        if (nss.size() > res.capacity() && res.size()==0 )
            res = ctx.str();
        else
            res.append(nss);
    }
    template <typename TChar, typename... Rest> void format_to(rc_string<TChar>& res, const TChar* fmt, const Rest&... rest) {
        return format_to(res, str_aslocal(fmt), rest...);
    }
    template <typename TChar, typename... Rest> void c_printf(const TChar* fmt, const Rest&... rest) {
        format_to(std::cout, str_aslocal(fmt), rest...);
    }


    template <typename TChar, typename... Rest> rc_string<TChar> format(const rc_string<TChar>& fmt, const Rest&... rest) {
        rc_string<TChar> res;
        format_to(res, fmt, rest...);
        return res;
    }

    template <typename TChar, typename... Rest> rc_string<TChar> format(const TChar* fmt, const Rest&... rest) {
        return format(str_aslocal(fmt), rest...);
    }


    rc_string<char> oldformat(const char* frmt, ...);
    const char* bformat(rc_string<char>& res, const char* frmt, ...);
    rc_string<char> vformat(va_params_t& vaparams);
    const char* vbformat(rc_string<char>& res, va_params_t& vaparams);

    stringA getSpaceAlignedString(int level);

    template <typename TChar, typename... Rest> void c_printf(const param_stringA& fmt, const Rest&... rest) {
        return format_to( std::cout , fmt.param, rest...);
    }

};

