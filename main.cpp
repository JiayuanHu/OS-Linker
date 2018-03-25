#include <iostream>
#include <fstream>
#include <map>
#include "errorevent.h"
#include <regex>
#include <cstring>
#include <cstdlib>

using namespace std;

#define MACHINE_SIZE 512

int linenum=0;
unsigned long lineoffset;
int acc=0;

class symbol{
public:
    symbol(string a,int b,int c){
        identifier=a;
        addr=b;
        module=c;
    }
    void setMultidef(){
        multidefined=true;
    }
    string getid(){return identifier;}
    int getaddr(){return addr;}
    int getmodule(){return module;}
    bool getused(){return used;}
    bool getmultidef(){return multidefined;}
    void setused(){used=true;}
    void resetaddr(int i){addr=i;}


private:
    string identifier;
    int addr;
    int module;
    bool used=false;
    bool multidefined=false;

};

map<string,symbol*> symbolmap;
string* symboltable=new string[256];  //at least 256 symbols, start from 0
int symnum=0;

class use{
public:
    use(string a){symbol=a;}
    void setused(){ used=true; }
    int getused(){return used;}
    string getname(){return symbol;}
private:
    string symbol;
    bool used=false;
};

class module{

public:
    module(){};
    void setdefbegin(int i) {defbegin=i;}
    void setdefend(int i) {defend=i;}
    void setsize(int i){size=i;}
    void setoffset(int i){offset=i;}
    int getsize(){return size;}
    int getdefbegin(){return defbegin;}
    int getdefend(){return defend;}
    int getoffset(){return offset;}
private:
    int size;
    int offset;
    int defbegin;
    int defend;

};



module* modtable[2048];     //may exceeds, start from 1
int modnum=0;

char* ReadToken(istream &input){
    static bool newline=true;
    static char linebuffer[2048];   //may exceeds
    char* tok;
    static int size;
    while(1){
        if(newline){
            string buffer;
            if(!getline(input,buffer)) return NULL;   //eof ends with '\n'
            memset(linebuffer,'\0', sizeof(char)*2048);
            strcpy(linebuffer,buffer.c_str());
            size=strlen(linebuffer);
            tok=strtok(linebuffer," \t\n");
            newline=false;
            linenum++;
        } else {
          tok=strtok(NULL," \t\n");
        }

        if(tok) {lineoffset=tok-linebuffer+1;return tok;}
        else {
            lineoffset=size+1;   //end of line '\n' offset
            newline=true;
        }
    }
}


bool checknum(char* tok){
    while(tok[0]!='\0'){       //safe??
        if(tok[0]<'0'||tok[0]>'9') return false;
        else tok++;  //{lineoffset++;}
    }
    return true;
}

int ReadNumber(istream &input,bool alloweof=false){
    char* tok;
    tok=ReadToken(input);
    if(!tok && alloweof) return -1;   //read defnum, can be end of file
    if(!tok || !checknum(tok)) {parseerror(0);exit(-1);}
    return stoi(tok);
}

bool Readinstr(istream &input,int &opcode,int &operand){
    char* tok, *toktmp;
    tok=ReadToken(input);

    toktmp=tok;

    //upto 4-digit ---rule 10/11
    int count=0;
    while(toktmp[0]!='\0') {
        count++;
        toktmp++;
    }

    if(count>4) {
        opcode=9;
        operand=999;
        return false;
    }
    else{
        int tmp=stoi(tok);
        opcode=tmp/1000;
        operand=tmp%1000;
        return true;
    }

}

string ReadSymbol(istream &input){
    char* tok;
    tok=ReadToken(input);
    if(!tok) {parseerror(1);exit(-1);}
    string token(tok);
    if(!regex_match(token,regex("[A-Za-z][a-zA-Z0-9]*"))){ parseerror(1);exit(-1); }
    if(token.length()>16) (parseerror(3),exit(-1));   //exact limits (a)
   // lineoffset+=token.length();
    return token;
}

char ReadAddr(istream &input){
    char* tok;
    tok=ReadToken(input);
    if(!tok){parseerror(2);exit(-1);}
    string token(tok);
    regex addr("^(A|E|I|R)");
    if(!regex_match(token,addr)){parseerror(2);exit(-1);}
   // lineoffset+=1;
    return tok[0];
}

void checkDefaddr(){
    module* cur=modtable[modnum];
    int modsize=cur->getsize();
    int st=cur->getdefbegin();
    int end=cur->getdefend();
    int offset=cur->getoffset();
    for(int i=st;i<end;i++) {
        map<string, symbol *>::iterator tmp = symbolmap.find(symboltable[i]);
        if (tmp == symbolmap.end()) {
            cout << "error";
            exit(-1);
        }
        int addr=tmp->second->getaddr();
        if(addr-offset>modsize-1){
            warninglist(5,modnum,tmp->first,modsize-1,addr-offset);
            tmp->second->resetaddr(offset);
        }

    }

}
void ReadDeflist(istream &input,bool &endoffile){

    //read def number
    int defnum=ReadNumber(input,true);
    if(defnum==-1) {endoffile=true;return;}

    //check range ---exact limits b(1)
    if(defnum>16) {parseerror(4);exit(-1);}

    //access def & store
    for(int i=0;i<defnum;i++){
        string id=ReadSymbol(input);
        int addr=ReadNumber(input)+acc;

        //check multidefine --- rule 2
        map<string,symbol*>::iterator it=symbolmap.find(id);
        if(it!=symbolmap.end()) {it->second->setMultidef();}
        else {
            //add to symbolmap & symboltable
            symbol* newsym = new symbol(id, addr, modnum);
            symboltable[symnum++]=id;
            symbolmap.insert(std::pair<string, symbol *>(id, newsym));
        }
    }

}

void ReadDeflist2(istream &input,bool &endoffile){

    //read def number
    int defnum=ReadNumber(input,true);
    if(defnum==-1) {endoffile=true;return;}

    //access def
    for(int i=0;i<defnum;i++){
        ReadSymbol(input);
        ReadNumber(input);
    }

}

void ReadUselist(istream &input){

    //read uselist num
    int usenum=ReadNumber(input);

    //check range---exact limits b(2)
    if(usenum>16) {parseerror(5);exit(-1);}

    //access uselist & store
    for(int i=0;i<usenum;i++){
        ReadSymbol(input);
    }
}

void ReadUselist2(istream &input,use** list,int &size){

    //read uselist num
    int usenum=ReadNumber(input);

    //access uselist & store
    for(int i=0;i<usenum;i++){
        string sym=ReadSymbol(input);
        use* tmp=new use(sym);
        list[size++]=tmp;
    }
}

void ReadInstrlist(istream &input){

    //read instr num
    int instrnum=ReadNumber(input);

    //check range
    acc+=instrnum;
    if(acc>MACHINE_SIZE) {parseerror(6);exit(-1);}

    //add info to module
    modtable[modnum]->setsize(instrnum);

    for(int i=0;i<instrnum;i++){
        ReadAddr(input);
        ReadNumber(input);
    }

}

int mem=0;
string memtoString(){
  //static int mem=0; //don't know why this no work
    string addup=to_string(mem/100)+to_string((mem%100)/10)+to_string(mem%10);
    mem=mem+1;
    return addup;
}

string instrtoString(int opcode,int oprd){
    return to_string(opcode)+to_string(oprd/100)+to_string((oprd%100)/10)+to_string(oprd%10);
}
void ReadInstrlist2(istream &input,use** uselist,int usesize){

    //read instr num
    int instrnum=ReadNumber(input);


    for(int i=0;i<instrnum;i++){
        cout<<memtoString()<<": ";
        char type=ReadAddr(input);
        int opcode,operand;
        bool legal=Readinstr(input,opcode,operand);
        switch(type){
            case 'A': {
                if(!legal) {cout<<instrtoString(opcode,operand);errorlist(11);}
                else{
                    if (operand > MACHINE_SIZE) {
                        cout << instrtoString(opcode, 0);
                        errorlist(8);   //rule 8
                    }
                    else cout<<instrtoString(opcode,operand);
                }

                break;
            }
            case 'R':{
                if(!legal) {cout<<instrtoString(opcode,operand);errorlist(11);}  //rule 11
                else{
                    int modsize=modtable[modnum]->getsize();
                    if(operand>=modsize) {cout<<instrtoString(opcode,0+acc);errorlist(9);}  //rule 9
                    else cout<<instrtoString(opcode,operand+acc);
                }
                break;
            }
            case 'I':{
                cout<<instrtoString(opcode,operand);
                if(!legal) errorlist(10);  //rule 10
                break;
            }
            case 'E':{
                if(!legal) {cout<<instrtoString(opcode,operand);errorlist(11);break;}  //rule 11
                if(operand>usesize-1) {cout<<instrtoString(opcode,operand);errorlist(6);break;} //rule 6

                use* tmp=uselist[operand];
                tmp->setused();
                string name=tmp->getname();
                map<string,symbol*>::iterator it=symbolmap.find(name);
                if(it!=symbolmap.end()){
                    symbol* sym=it->second;
                    sym->setused();
                    cout<<instrtoString(opcode,sym->getaddr());
                }
                else{
                    cout<<instrtoString(opcode,0); //rule 3
                    errorlist(3,name);
                }
                break;
            }
            default: cout<<"Error"<<endl;break;
        }
        cout<<endl;
    }

    acc+=instrnum;
}



int main(int argc,char** argv) {

    if(argc!=2) {cout<<"Usage: "<<argv[0]<<" <filename>"<<endl;exit(-1);}

    //input file
    ifstream inputfile (argv[1]);
    if(!inputfile.is_open()) cout<<"Error: can't load input file\n";


//////////////PASS ONE//////////////////////////////////////////////////////////////////////////////////////////////////

    while(!inputfile.eof()) {
        modnum++;
        module * mod=new module();
        modtable[modnum]=mod;
        mod->setdefbegin(symnum);
        mod->setoffset(acc);

        //Deflist
        bool endoffile=false;
        ReadDeflist(inputfile,endoffile);
        if(endoffile) {
            delete(mod);
            modtable[modnum]=NULL;
            modnum--;
            break;
        }
        mod->setdefend(symnum);

        //Uselist
        ReadUselist(inputfile);

        //Instructions
        ReadInstrlist(inputfile);

        //rule 5
        checkDefaddr();
    }

    //print symbol table
    cout << "Symbol Table" << endl;
    for (int i = 0; i < symnum; i++) {
        string key = symboltable[i];
        map<string, symbol *>::iterator it = symbolmap.find(key);
        symbol *sym = it->second;
        cout << sym->getid() << "=" << sym->getaddr();
        if (sym->getmultidef()) errorlist(2);
        cout << endl;
    }
    cout<<endl;

//////////////END OF PASS ONE///////////////////////////////////////////////////////////////////////////////////////////




    //reset global numbers
    linenum=0;
    lineoffset=0;
    acc=0;
    modnum=0;
    inputfile.clear();
    inputfile.seekg(0,ios::beg);




//////////////PASS TWO//////////////////////////////////////////////////////////////////////////////////////////////////

    //print
    cout<<"Memory Map"<<endl;

    while(!inputfile.eof()){
        modnum++;

        //Deflist
        bool endoffile=false;
        ReadDeflist2(inputfile,endoffile);
        if(endoffile) {modnum--;break;}

        //Uselist
        use* uselist[16];  //at most 16 use
        int usesize=0;
        ReadUselist2(inputfile,uselist,usesize);

        //Instructions
        ReadInstrlist2(inputfile,uselist,usesize);

        //check uselist --- rule 7
        for(int i=0;i<usesize;i++){
            use* tmp=uselist[i];
            if(!tmp->getused()) warninglist(7,modnum,tmp->getname());
        }

    }
    cout<<endl;


    //defined but not used --- rule 4
    for(int i=0;i<symnum;i++){
        string key = symboltable[i];
        map<string, symbol *>::iterator it = symbolmap.find(key);
        symbol *sym = it->second;
        if(!sym->getused()) warninglist(4,sym->getmodule(),sym->getid());
    }
    cout<<endl;


//////////////END OF PASS TWO///////////////////////////////////////////////////////////////////////////////////////////


    //close file
    inputfile.close();

    return 0;

}




