#include <cassert>
#include "t_format.h"
#include "tb_basedtools.h"
#include "tb_c_tools.h"
#include "t_string.h"
#include "t_sstream.h"


namespace tbgeneral {
    namespace format_ctx_ns {

    template <typename TChar> void str_slice(rc_string<TChar>& s, const TChar* start, const TChar* end) {
        s.sdata = const_cast<TChar*>(start); s.fsize = end - start;
    }
    template <typename TChar> rc_string<TChar> str_slice(const TChar* start, const TChar* end) {
        rc_string<TChar> res; str_slice(res, start, end); return res;
    }

        int to_format_flags(int vchar) {
            switch (vchar) {
            case '-': return eff_LeftJustity;
            case '+': return eff_ForcedSignValue;
            case ' ': return eff_ForcedSignBlank;
            case '#': return eff_Lattice;
            case '0': return eff_ZeroLeftPad;
            }
            return 0x0;
        }
        int to_format_lenspec(int vchar) {
            switch (vchar) {
            case 'h': case 'l': case 'j': case 'z': case 't': case 'L': return vchar;
            default: return 0;
            }
            return 0x0;
        }
        enum {
            efsp_SignedInt = 'd', efsp_UnsignedInt = 'u', efsp_InsignOctal = 'o', efsp_HexUInt_lc = 'x', efsp_HexUInt_uc = 'X'
            , efsp_DecFloatPoint = 'f', efsp_ScinceFloat_lc = 'e', efsp_ScinceFloat_uc = 'E', efsp_GenFloat_lc = 'g', efsp_GenFloat_uc = 'G'
            , efsp_HexFloat_lc = 'a', efsp_HexFloat_uc = 'A'
            , efsp_Character = 'c', efsp_String = 's', efsp_StringW = 'S', efsp_Pointer = 'p', efsp_NoPrint = 'n', efsp_Nothing = '%'

        };

        size_t to_format_specify(int vchar) {
            static char map[128] = { 0 };
            static bool mapinit = false;
            static const char* list = "duoxXfeEgGaAcsSpn";
            if (!mapinit) {
                for (auto p = list; *p; p++) map[*p] = *p;
                map['F'] = 'f'; map['i'] = 'd';
                mapinit = true;
            };
            return (vchar < sizeof(map)) ? map[vchar] : 0;
        }
        //    eff_LeftJustity = 1, eff_ForcedSignValue = 2, eff_ForcedSignBlank = 4, eff_Lattice = 8, eff_ZeroLeftPad = 0x10
        //, eff_AddWidthParam = 0x20, eff_AddPrecParam = 0x40

        int format_parse_specifier(format_ctx<char>& ctx, const char* ds, const char* de) {
            auto dsb = ds;
            ctx.fspec = { 0 };
            auto& fspec = ctx.fspec; fspec.prec = -1;
            if (ds < de && ds[0] == '%') {
                str_slice(ctx.fmtstr, ds + 1, de);
                ctx.os[0] << '%';
                return 0;
            }
            //int flags=0;    int width=0;   int prec=-1; int dlen=0; int fspec=0;
            for (; ds < de; ds++) { auto f = to_format_flags(ds[0]); fspec.flags |= f; if (f == 0) break; }
            if (ds < de && ds[0] == '*') { fspec.flags |= eff_AddWidthParam; ds++; };
            if (!(fspec.flags & eff_AddWidthParam)) for (; (ds < de) && ds[0] >= '0' && ds[0] <= '9'; ds++)
                fspec.width = fspec.width * 10 + (ds[0] - '0');
            if (ds < de && ds[0] == '.') { fspec.prec = 0; ds++; };
            if (0 == fspec.prec && ds < de && ds[0] == '*') { fspec.flags |= eff_AddPrecParam; ds++; };
            if (0 == fspec.prec && !(fspec.flags & eff_AddPrecParam)) for (; (ds < de) && (ds[0] >= '0' && ds[0] <= '9'); ds++)
                fspec.prec = fspec.prec * 10 + (ds[0] - '0');
            if (ds < de) { fspec.length = to_format_lenspec(ds[0]); if (fspec.length) ds++; };
            if (ds < de && fspec.length == 'h' && ds[0] == 'h') { fspec.length = 'hh'; ds++; }
            if (ds < de && fspec.length == 'l' && ds[0] == 'l') { fspec.length = 'll'; ds++; }
            if (ds < de) { fspec.specifer = static_cast<int>( to_format_specify(ds[0]) ); ds++; }
            str_slice(ctx.fmtstr, ds, de);
            ctx.currfmtspec = str_slice(dsb - 1, ds);
            return fspec.specifer;
        }

        int format_parse(format_ctx<char>& ctx, bool fin) {
            auto ds = ctx.fmtstr.cbegin(), de = ctx.fmtstr.cend(), dsb = ds;
            if (!fin)
                for (; ds < de; ds++) {
                    if (*ds == '%') {
                        ctx.os[0].write(dsb, ds - dsb);
                        auto fspec = format_parse_specifier(ctx, ds + 1, de);
                        if (fspec != 0) return 0;
                        dsb = ds = ctx.fmtstr.cbegin() - 1;
                    }
                }
            else { ds = de; }
            ctx.os[0].write(dsb, ds - dsb);
            return 0;
        }

        template<typename DType> int format_out_data_std_t(format_ctx<char>& ctx, DType d, const char* repl_len) {
            if (ctx.fspec.specifer == efsp_String || ctx.fspec.specifer == efsp_StringW) {
                assert(false);
                *ctx.os << d;
            }
            char buff[128]; char fmtbuff[128];
            auto fs = ctx.currfmtspec.to_buff(fmtbuff, sizeof(fmtbuff));
            if (ctx.fspec.length == 0 && repl_len) {
                auto endfs = fs.size() - 1;
                char fspecfs = fs[endfs];
                fs.replace(endfs, 1, repl_len);
                fs.push_back(fspecfs);
            }
            auto cntc = snprintf(buff, sizeof(buff), fs.c_str(), d);
            ctx.os->write(buff, cntc);
            return 0;
        }

        int format_out_data_float(format_ctx<char>& ctx, double d) { return format_out_data_std_t(ctx, d, 0); };
        int format_out_data_int(format_ctx<char>& ctx, int32_t d) { return format_out_data_std_t(ctx, d, 0); };
        int format_out_data_int(format_ctx<char>& ctx, int64_t d) { return format_out_data_std_t(ctx, d, "ll"); };

        template<typename DChar, typename SChar> int format_out_data_string_t(format_ctx<DChar>& ctx, const SChar* s, size_t cnt) {
            int pad = 0; DChar fillc = ' ';
            if (ctx.fspec.width != 0) {
                //TODO:!for utf8 need calculate cnt
                if (ctx.fspec.width > static_cast<int>(cnt)) {
                    pad = ctx.fspec.width - static_cast<int>(cnt);
                    pad = ctx.fspec.flags & eff_LeftJustity ? -pad : pad;
                }
            };
            if (pad > 0) nchar2stream(*ctx.os, fillc, pad);
            push2stream(*ctx.os, s, cnt);
            if (pad < 0) nchar2stream(*ctx.os, fillc, -pad);
            return 0;
        }

        int format_out_data_string(format_ctx<char>& ctx, const char* s, size_t cnt) { return format_out_data_string_t(ctx, s, cnt); }
        int format_out_data_string(format_ctx<char>& ctx, const wchar_t* s, size_t cnt) { return format_out_data_string_t(ctx, s, cnt); };
        int format_out_data_string(format_ctx<wchar_t>& ctx, const char* s, size_t cnt) { return format_out_data_string_t(ctx, s, cnt); }
        int format_out_data_string(format_ctx<wchar_t>& ctx, const wchar_t* s, size_t cnt) { return format_out_data_string_t(ctx, s, cnt); };

    };
}; // namespace tbgeneral::format_ctx_ns
// https://www.cplusplus.com/reference/cstdio/printf/

namespace tbgeneral {
    const char* vbformat(stringA& res, va_params_t& vaparams) {
        if (!vaparams.vars_present) { res << vaparams.format; return res.c_str(); }

        char buff[4 * 1024];
        auto res_oldsz = res.size();
        int r = tb_vsnprintf(buff, sizeof(buff), vaparams.format, vaparams.vars);
        if (r <= int(sizeof(buff))) {
            res.append(buff, r);
        }
        else {
            res.resize(res_oldsz + r);
            r = tb_vsnprintf(&res[res_oldsz], r + 1, vaparams.format, vaparams.vars);
        }
        return res.c_str();
    };

    const char* bformat(stringA& res, const char* format, ...) {
        va_Init(vaparams, format);
        return vbformat(res, vaparams);
    };

    stringA oldformat(const char* format, ...)
    {
        stringA buff;
        va_Init(vaparams, format);
        vbformat(buff, vaparams);
        return buff;
    };
    stringA  vformat(va_params_t& vaparams) {
        stringA buff;
        vbformat(buff, vaparams);
        return buff;
    };

    stringA getSpaceAlignedString(int level) {
        enum { MAXSPSTR = 50 };
        static darray<stringA> perAlignedStr;
        if (perAlignedStr.size() == 0) {
            darray<stringA> bb; bb.resize(MAXSPSTR);
            auto per = " "; stringA d;
            for (int i = 0; i < MAXSPSTR; i++) { bb[i] = d; d << per; }
            perAlignedStr = bb;
        }
        auto sz = static_cast<int>( perAlignedStr.size() );
        if (level < sz) { return perAlignedStr[level]; }
        stringA result;
        for (; level >= sz; level -= sz - 1)
            result << perAlignedStr[sz - 1];
        result << perAlignedStr[level];
        return result;
    }


    // „лены limit_rc_stringbuf

    template<> void limit_rc_stringbuf<char>::flush() {
            if (!buff.size()) return;
            flush_dest(buff.cbegin(), buff.size());
            buff.reset_size();
    };
    template<> void limit_rc_stringbuf<char>::write_proc(const char_type* s, size_t n) {
            auto capa = buff.capacity();
            if (capa<2) { 
                flush_dest( s , n*sizeof(char_type) ); return; 
            }
            while (n > 0) {
                auto nn = std::min(capa - buff.size(), n);
                buff.append(s, nn);
                s += nn; n -= nn;
                if (buff.size() == capa)
                    flush();
            };
    }
    template<> std::streamsize limit_rc_stringbuf<char>::xsputn(const char_type* s, std::streamsize n) {
            write_proc(s, static_cast<size_t>(n));
            return n;
    };
    template<> limit_rc_stringbuf<char>::int_type limit_rc_stringbuf<char>::overflow(int_type c ) {
        if (traits_type::eq_int_type(traits_type::eof(), c)) return traits_type::not_eof(c);
        char_type cc = traits_type::to_char_type(c);
        write_proc(&cc, 1);
        return c;
    };


}; //namespace tbgeneral {