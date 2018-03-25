#include <iostream>
#include "main.h"
using namespace std;

void parseerror(int errcode){
    static string errstr[]={
            "NUM_EXPECTED",     //0
            "SYM_EXPECTED",     //1
            "ADDR_EXPECTED",    //2
            "SYM_TOO_LONG",     //3
            "TOO_MANY_DEF_IN_MODULE",   //4
            "TOO_MANY_USE_IN_MODULE",   //5
            "TOO_MANY_INSTR",           //6
    };
    cout<<"Parse Error line "<<linenum<<" offset "<<lineoffset<<": "<<errstr[errcode]<<endl;
}

void errorlist(int errrule,string symbol){
    string errmsg;
    switch(errrule){
        case 2: errmsg=" Error: This variable is multiple times defined; first value used";break;
        case 3: errmsg=" Error: "+symbol+" is not defined; zero used";break;
        case 6: errmsg=" Error: External address exceeds length of uselist; treated as immediate";break;
        case 8: errmsg=" Error: Absolute address exceeds machine size; zero used";break;
        case 9: errmsg=" Error: Relative address exceeds module size; zero used";break;
        case 10: errmsg=" Error: Illegal immediate value; treated as 9999";break;
        case 11: errmsg=" Error: Illegal opcode; treated as 9999";break;
        default: errmsg="Wrong Error Code";break;
    }
    cout<<errmsg;

}



void warninglist(int warnrule,int module,string symbol, int modulesize=0,int symbolpos=0){
    switch(warnrule){
        case 4: cout<<"Warning: Module "<<module<<": "<<symbol<<" was defined but never used"<<endl;break;
        case 5: cout<<"Warning: Module "<<module<<": "<<symbol<<" too big "<<symbolpos<<" (max="<<modulesize<< ") assume zero relative"<<endl;break;
        case 7: cout<<"Warning: Module "<<module<<": "<<symbol<<" appeared in the uselist but was not actually used"<<endl;break;
    }

}