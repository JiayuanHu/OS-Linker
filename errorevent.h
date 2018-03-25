//
// Created by Jiayuan Hu on 2/8/18.
//

//#ifndef LINKER_ERROREVENT_H
//#define LINKER_ERROREVENT_H

//#endif //LINKER_ERROREVENT_H

#include <string>

void parseerror(int errcode);

void errorlist(int errrule,std::string symbol="");

void warninglist(int warnrule,int module,std::string symbol, int modulesize=0,int symbolpos=0);