#include <cctype>
#include <cstring>

#include <cstring>
#include <memory>

#include "tw_parser.h"

#include <map>
#include <vector>
#include <string>

namespace taowin {
    namespace parser {
        enum class TK {
            error = 0, tag, attr, assign, value, text, close, close1, close2, eof, next,
        };

        static char* p = nullptr;

        static inline void _skip() {
            while(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
                p++;
        }

        enum class SC {
            init, tag,
        };

        static SC sc = SC::init;
        static TK tk = TK::error;
        static const char* tt;  // token text
        static char nc; // next char to be used, if not zero

        static void next() {
            if(nc) {
                *p = nc;
                nc = 0;
            }
            if(sc == SC::init) {
                _skip();
                if(*p == '<') {
                    p++;
                    if(*p == '/') {
                        p++;
                        tt = p;
                        while(::isalnum(*p))
                            p++;
                        if(*p == '>') {
                            *p = '\0';
                            p++;
                            sc = SC::init;
                            tk = TK::close2;
                            return;
                        }
                    } else if(::isalpha(p[0])) {
                        tt = p;
                        while(::isalnum(*p))
                            p++;
                        if(*p != '\0') {
                            nc = *p;
                            *p = '\0';
                        }
                        sc = SC::tag;
                        tk = TK::tag;
                        return;
                    }
                } else {
                    tt = p;
                    while(*p && *p != '<' && *p != '>')
                        p++;
                    if(*p != '\0') {
                        nc = *p;
                        *p = '\0';
                    }
                    tk = *tt ? TK::text
                        : *p ? TK::error
                        : TK::eof;
                    return;
                }
            } else if(sc == SC::tag) {
                _skip();
                if(::isalpha(*p)) {
                    tt = p;
                    while(::isalnum(*p))
                        p++;
                    if(*p != '\0') {
                        nc = *p;
                        *p = '\0';
                    }
                    tk = TK::attr;
                    return;
                } else if(*p == '=') {
                    *p = '\0';
                    p++;
                    tk = TK::assign;
                    return;
                } else if(*p == '\'' || *p == '\"') {
                    const char c = *p++;
                    tt = p;
                    while(*p && *p != c)
                        p++;
                    if(*p == c) {
                        *p = '\0';
                        p++;
                        tk = TK::value;
                        return;
                    }
                } else if(*p == '/') {
                    *p = '\0';
                    p++;
                    if(*p == '>') {
                        p++;
                        sc = SC::init;
                        tk = TK::close1;
                        return;
                    }
                } else if(*p == '>') {
                    p++;
                    sc = SC::init;
                    tk = TK::close;
                    return;
                }
            }

            if(!*p) {
                tk = TK::eof;
                return;
            }

            tk = TK::error;
            return;
        }

        inline bool is(const char* t, const char* a) {
            return ::strcmp(t, a) == 0;
        }


        static void parse(char* xml, PARSER_OBJECT* parent) {
            p = xml;
            for(;;) {
                next();
                if(tk == TK::tag) {
                    const char* tag_name = tt;
                    PARSER_OBJECT* obj = new PARSER_OBJECT;
                    obj->tag = tag_name;
                    parent->append_child(obj);
                    next();
                    while(tk == TK::attr) {
                        //std::cout << " " << tt;
                        const char* attr = tt;
                        next();
                        if(tk == TK::assign) {
                            //std::cout << "=\"";
                            next();
                            if(tk == TK::value) {
                                obj->set_attr(attr, tt);
                                next();
                                continue;
                            }

                            throw "value expected after assignment.";
                        }
                        obj->set_attr(attr, "");
                    }

                    if(tk == TK::close) {
                        //std::cout << "\n";
                        for(;;) {
                            parse(p, obj);
                            if(tk == TK::next)
                                continue;

                            else if(tk == TK::close2) {
                                if(::strncmp(tag_name, tt, ::strlen(tt)) != 0)
                                    throw "mismatched opening tag and closing tag.";

                                tk = TK::next;
                                //std::cout << tt;
                                //::cout << "\n";
                                return;
                            }

                            if(tk == TK::eof) throw "premature eof, expecting closing tag.";
                            else throw "expecting closing tag.";
                        }
                    } else if(tk == TK::close1) {
                        tk = TK::next;
                        //std::cout << "\n";
                        return;
                    }

                    throw "unexpected following token for open tag.";
                } else if(tk == TK::text) {
                    tk = TK::next;
                    //std::cout << "<!CDATA[" << tt << "]]\n";
                    return;
                } else if(tk == TK::close2) {
                    return;
                } else if(tk == TK::eof) {
                    return;
                }

                throw "unexpected token while calling parse.";
            }
        }

        PARSER_OBJECT* parse(char* xml) {
            PARSER_OBJECT po;

            try {
                parse(xml, &po);
            }
            catch(const char*) {

            }

            return po.first_child();
        }

        PARSER_OBJECT* parse(const char* xml) {
            int len = (int)::strlen(xml) + 1;
            std::unique_ptr<char> x(new char[len]);
            ::memcpy(x.get(), xml, len);
            return parse(x.get());
        }

    }
}
