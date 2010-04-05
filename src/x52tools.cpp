#include <stdio.h>
#include <stdarg.h>
#include "x52tools.h"
#include <iostream>
#include <fstream>

using namespace std;
using std::map;
using std::string;

x52tools_t::x52tools_t(void) : verbose(false)
{
}

void x52tools_t::debug_out(int type, const char* msg, ...)
{
    if ((type == info) && (!verbose)) return;
    va_list ap;
    va_start(ap, msg);
    switch (type)
    {
    case info:
        fprintf(stderr, "\033[0;32m[x52control]\033[0m: ");
        break;
    case warn:
        fprintf(stderr, "\033[0;34m[x52control]\033[0m: ");
        break;
    case err:
    default:
        fprintf(stderr, "\033[0;31m[x52control]\033[0m: ");
        break;
    }
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    fflush(stderr);
}

void x52tools_t::debug_out(const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    fprintf(stderr, "[x52control]: ");
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    fflush(stderr);
}

void x52tools_t::read_config(void) {
    debug_out(info,"reading config");
    string line;
    char _buff[1024];
    int line_count=0;
    map<string,string> config;
    ifstream cfg("Resources/plugins/x52control/x52control.ini");
    if ( cfg.good() ) {
        while ( !cfg.eof()) {
            line_count++;
            cfg.getline(_buff, 1024);
            line=_buff;
            //ignore remarks
            int found=line.find("#",0);
            if (found >= 0) line.resize(found);
            if (line.length() > 0 ){
                //remove blanks
                found=line.find(" ",0);
                while ( found >= 0 ) {
                    line.replace(found,1,"");
                    found=line.find(" ",0);
                }
                found=line.find("=");
                if (found >= 0) {
                    string key=line.substr(0,found);
                    string val=line.substr(found+1);
                    config[key]=val;
                } else {
                    debug_out(err, "configuration file error: garbage in line %d",line_count);
                }
            }
        }
        cfg.close();
    } else {
        debug_out(err,"can't read config file");
    }
    x52control_config=config;
}
