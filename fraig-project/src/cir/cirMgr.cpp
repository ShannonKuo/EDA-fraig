/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;


static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

CirGate*
CirMgr::getGate(unsigned gid) const {
   if (gid >= _maxVar + _poSize + 1) return 0;
   return _TOTAL[gid];
}
/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   int litId;
   size_t cnt = 1;
   string tokens[6];
   string token;
       
   ifstream _file;
   _file.open(fileName.c_str(), ifstream::in);
   if (_file == 0) {
      cout<<"Cannot open design \""<<fileName.c_str()<<"\"!!"<<endl;
      return 0;
   }
   while (_file.getline(buf,1024,'\n')){
      string line = string(buf); 
      if (lineNo == 0){
         size_t n = myStrGetTok(line, token);
         int id = 0;
         //error        
         for (size_t i = 0; i < line.size(); i++){
            if (line[i] == 32 && line[i+1] ==32){
               colNo = i + 1;
               parseError(EXTRA_SPACE);
               return 0; 
            }
         }
         while(token.size()){
            tokens[id] = token;
            n = myStrGetTok(line, token, n);
            id++;
         };
         //bug colNo;
         for (size_t i = 0; i < 6; i++){
            if (tokens[i] == ""){
               errMsg = "number of AIGs";
               parseError(MISSING_NUM);
               return 0;
            }
         }
         if (tokens[0] != "aag") {
            errMsg = tokens[0];
            parseError(ILLEGAL_IDENTIFIER); 
            return 0;
         }
         _maxVar = atoi(tokens[1].c_str());
         _piSize = atoi(tokens[2].c_str());
         _poSize = atoi(tokens[4].c_str());
         _aigSize = atoi(tokens[5].c_str());
         _TOTAL = new CirGate*[_maxVar + _poSize + 1];
         for(size_t i = 0; i < _maxVar + _poSize + 1; i++)
            _TOTAL[i] = 0;
      }
      else if (lineNo <= _piSize){
         myStr2Int(line, litId);
         CirPiGate* piGate = new CirPiGate;
         piGate->_varId = (size_t)(litId/2);
         piGate->_lineNo = lineNo+1;
         _PI.push_back(piGate);
      } 
      else if (lineNo <= (_piSize + _poSize)){
         myStr2Int(line, litId);
         CirPoGate* poGate = new CirPoGate;
         poGate->_varId = _maxVar + cnt;
         poGate->_faninId.push_back(litId);
         poGate->_lineNo = lineNo+1;
         _PO.push_back(poGate);
         cnt++;
      }
      else if (lineNo <= (_piSize + _poSize + _aigSize)){
         int id = 0;
         size_t n = myStrGetTok(line, token);
         while(token.size()){
            tokens[id] = token;
            n = myStrGetTok(line, token, n);
            id++;
         };
         myStr2Int(tokens[0], litId);
         CirAigGate* aigGate = new CirAigGate;
         aigGate->_varId = (size_t)(litId/2);
         aigGate->_lineNo = lineNo+1;
         myStr2Int(tokens[1], litId);
         aigGate->_faninId.push_back(litId);
         myStr2Int(tokens[2], litId);
         aigGate->_faninId.push_back(litId);
         _AIG.push_back(aigGate);
      }
      else {
         if (line[0] == 'c') break;
         int id = 0;
         size_t n = myStrGetTok(line, token);
         while(token.size()){
            tokens[id] = token;
            n = myStrGetTok(line, token, n);
            id++;
         };
         if (line[0] == 'i'){
            tokens[0].erase(tokens[0].begin());
            _PI[atoi(tokens[0].c_str())]->_symbol = tokens[1];
         }
         else if (line[0] == 'o'){
             tokens[0].erase(tokens[0].begin());
            _PO[atoi(tokens[0].c_str())]->_symbol = tokens[1];
         }
      }
      for (int i = 0; i < 1024; i++)
         buf[i] = 0;
      ++lineNo;
   }
   setTotalList();
   setFaninList(_AIG);
   setFaninList(_PO);
   setConst(0);
   setFanoutList(_AIG);
   setFanoutList(_PO);
   setFanoutList(_FL);
   setFlGate(_AIG);
   setFlGate(_PI);
   sort(_flId.begin(), _flId.end());

   for (size_t i = 0; i < _PO.size(); i++)   
      dfsTraversal(_PO[i]);
   if (_PO.size() != 0) _PO[0]->globalRefPlus1(); 
   lineNo = 0;
   return true;

}



/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   cout<<endl
       <<"Circuit Statistics"<<endl
       <<"=================="<<endl
       <<"  PI   "<<setw(9)<<right<<_piSize<<endl
       <<"  PO   "<<setw(9)<<right<<_poSize<<endl
       <<"  AIG  "<<setw(9)<<right<<_AIG.size()<<endl
       <<"------------------"<<endl
       <<"  Total"<<setw(9)<<right
       <<(_piSize + _poSize + _AIG.size())<<endl;
}

void
CirMgr::printNetlist() const
{
   cout<<endl;
   int cnt = 0;
   static size_t id = 0;
   for (size_t i = 0; i < _NET.size(); i++){
      if (_NET[i]->getTypeStr() == "PI"){
         cout<<"["<<cnt<<"] "<<"PI  "<<_NET[i]->_varId;
         if (_NET[i]->_symbol != "") cout<<" ("<<_NET[i]->_symbol<<")";
         cout<<endl;
      }
      else if (_NET[i]->getTypeStr() == "PO"){
         cout<<"["<<cnt<<"] "<<"PO  "<<_NET[i]->_varId<<" ";
         if (_NET[i]->_faninList[0]->getTypeStr() =="UNDEF") cout<<"*";
         if (_NET[i]->_inv[0] ==1) cout<<"!";
         cout<<_NET[i]->_faninList[0]->_varId;
         if (_NET[i]->_symbol != "") cout<<" ("<<_NET[i]->_symbol<<")";
         cout<<endl;
      }
      else if (_NET[i]->getTypeStr() == "AIG"){
         cout<<"["<<cnt<<"] "<<"AIG "<<_NET[i]->_varId<<" ";
         if (_NET[i]->_faninList[0]->getTypeStr() =="UNDEF") cout<<"*";
         if (_NET[i]->_inv[0] == 1) cout<<"!";
         cout<<_NET[i]->_faninList[0]->_varId<<" ";
         if (_NET[i]->_faninList[1]->getTypeStr() =="UNDEF") cout<<"*";
         if (_NET[i]->_inv[1] == 1) cout<<"!";
         cout<<_NET[i]->_faninList[1]->_varId<<endl;
         id++;
      }
      else if (_NET[i]->getTypeStr() == "CONST")
         cout<<"["<<cnt<<"] "<<"CONST"<<_NET[i]->_varId<<endl;
      else if (_NET[i]->getTypeStr() == "FLOATING FANIN") 
         cout<<"["<<cnt<<"] "<<"FL   "<<_NET[i]->_varId<<endl;
      if (_NET[i]->getTypeStr() != "UNDEF") cnt++;
   }
}

void 
CirMgr::dfsTraversal(CirGate* gate)  
{
   for (size_t i = 0; i < gate->_faninList.size(); i++){ 
      if( !gate->_faninList[i]->isGlobalRef()){
         gate->_faninList[i]->setToGlobalRef();
         dfsTraversal(gate->_faninList[i]);
      }
   }
   gate->_dfs = 1;
   _NET.push_back(gate);
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit: ";
   for (size_t i = 0; i < _piSize; i++){
      cout<<(_PI[i]->_varId);
      if (i != _piSize - 1) cout<<" ";
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit: ";
   for (size_t i = 0; i < _poSize; i++){
      cout<<_PO[i]->_varId;
      if (i != _poSize - 1) cout<<" ";
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   vector <size_t> fl;
   if (_undefined != 0){
      cout << "Gates with floating fanin(s): ";
      for (size_t i = 0; i < _flId.size(); i++)
         if (getGate(_flId[i])->getTypeStr() == "UNDEF"){
            
            for (size_t j = 0; j < getGate(_flId[i])->_fanoutList.size(); j++){
               for (size_t k = 0; k < fl.size(); k++){
                  if (getGate(_flId[i])->_fanoutList[j]->_varId == fl[k])
                     goto NOTUSED;
                  fl.push_back(getGate(_flId[i])->_fanoutList[j]->_varId);
                }
                if (fl.size() == 0)
                   fl.push_back(getGate(_flId[i])->_fanoutList[j]->_varId);
            }
            for (size_t j = 0; j < getGate(_flId[i])->_fanoutList.size(); j++){
               cout<<getGate(_flId[i])->_fanoutList[j]->_varId;
               if (j != getGate(_flId[i])->_fanoutList.size() - 1) cout<<" ";
            }
         }
      cout<<endl;
   }
NOTUSED:   
   if (_notUsed != 0){
      cout <<"Gates defined but not used  : ";
      for (size_t i = 0; i < _flId.size(); i++){
         if (getGate(_flId[i])->getTypeStr() != "UNDEF"){
            cout<<_flId[i];
            if (i != _flId.size() - 1) cout<<" ";
         }
      }

      cout<<endl;
   }

}

void
CirMgr::printFECPairs()const
{
   for (size_t j = 0; j < fecGrps.size(); j++){
     cout<<"["<<j<<"] ";
     for (size_t k = 0; k < fecGrps[j].size(); k++){
        if (k != 0 && getGate(fecGrps[j][k])->_sim != getGate(fecGrps[j][0])->_sim)  
           cout<<"!";
        cout<<fecGrps[j][k];
        if (k != fecGrps[j].size() - 1) cout<<" ";
     }
     cout<<endl;
   }

}

void
CirMgr::writeAag(ostream& outfile) const
{
   size_t _aigdSize = 0;
   for (size_t i = 0; i < _AIG.size(); i++)
      if (_AIG[i]->_dfs == 1) _aigdSize++;
   outfile<<"aag "<<_maxVar<<" "<<_piSize<<" 0 "
       <<_poSize<<" "<<_aigdSize<<endl;
   for (size_t i = 0; i < _PI.size(); i++)
      outfile<<_PI[i]->_varId * 2<<endl;
   for (size_t i = 0; i < _PO.size(); i++){
      if (_PO[i]->_inv[0] == 1) outfile<<(_PO[i]->_faninList[0]->_varId * 2 + 1)<<endl;
      else outfile<<(_PO[i]->_faninList[0]->_varId * 2)<<endl;
   }
   for (size_t i = 0; i < _NET.size(); i++){
      if (_NET[i]->_dfs == 1 && _NET[i]->getTypeStr() == "AIG"){
         outfile<<_NET[i]->_varId * 2<<" ";
         if (_NET[i]->_inv[0] == 1)
            outfile<<_NET[i]->_faninList[0]->_varId * 2 + 1<<" ";
         else 
            outfile<<_NET[i]->_faninList[0]->_varId * 2 <<" ";
         if (_NET[i]->_inv[1] == 1)
            outfile<<_NET[i]->_faninList[1]->_varId * 2 + 1<<endl;
         else
            outfile<<_NET[i]->_faninList[1]->_varId * 2<<endl;
      }
   }
   for (size_t i = 0; i < _PI.size(); i++){
      if (_PI[i]->_symbol != "") 
         outfile<<"i"<<i<<" "<<_PI[i]->_symbol<<endl;
   }
   for (size_t i = 0; i < _PO.size(); i++){
      if (_PO[i]->_symbol != "") 
         outfile<<"o"<<i<<" "<<_PO[i]->_symbol<<endl;
   }


}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{  
   size_t _aigdSize = 0;
   for (size_t i = 0; i < _AIG.size(); i++)
      if (_AIG[i]->_dfs == 1) _aigdSize++;
   outfile<<"aag "<<_maxVar<<" "<<_piSize<<" 0 "
       <<_poSize<<" "<<_aigdSize<<endl;
   for (size_t i = 0; i < _PI.size(); i++)
      for (size_t j = 0; j < g->_faninList.size(); j++)
         if (_PI[i] == g->_faninList[j])
            outfile<<_PI[i]->_varId * 2<<endl;
   for (size_t i = 0; i < _PO.size(); i++)
      for (size_t j = 0; j < g->_fanoutList.size(); j++)
         if (_PO[i] == g->_fanoutList[j])
            outfile<<_PO[i]->_faninList[0]->_varId<<endl;
   
   outfile<<g->_varId * 2<<" ";
   if (g->_inv[0] == 1)
      outfile<<g->_faninList[0]->_varId * 2 + 1<<" ";
   else 
      outfile<<g->_faninList[0]->_varId * 2 <<" ";
   if (g->_inv[1] == 1)
      outfile<<g->_faninList[1]->_varId * 2 + 1<<endl;
   else
      outfile<<g->_faninList[1]->_varId * 2<<endl;
      
   
   for (size_t i = 0; i < _PI.size(); i++){
      for (size_t j = 0; j < g->_faninList.size(); j++)
         if (_PI[i] == g->_faninList[j]){
            if (_PI[i]->_symbol != "") 
               outfile<<"i"<<i<<" "<<_PI[i]->_symbol<<endl;
         }
   }
   for (size_t i = 0; i < _PO.size(); i++){
      for (size_t j = 0; j < g->_fanoutList.size(); j++)
         if (_PO[i] == g->_fanoutList[j]){
            if (_PO[i]->_symbol != "") 
               outfile<<"o"<<i<<" "<<_PO[i]->_symbol<<endl;
         }
   }



}

void 
CirMgr::setFaninList(vector<CirGate* > gate){
   for (size_t i = 0; i < gate.size(); i++){
      for (size_t j = 0; j < gate[i]->getFaninIdSize(); j++){
         if (getGate((gate[i]->getFaninId(j))/2)){
            if (gate[i]->getFaninId(j) % 2 != 0)
               gate[i]->_inv[j] = true;
            gate[i]->_faninList.push_back(getGate(gate[i]->getFaninId(j)/2));
            
         }
         //constant 0 1 
         else if (gate[i]->getFaninId(j) <= 1){
            if (gate[i]->getFaninId(j) == 1){
               gate[i]->_inv[j] = true;
               setConst(gate[i]);
            }
            else setConst(gate[i]);
         }
         //undefined
         else {
            CirFlGate* fl = new CirFlGate;
            fl -> _varId = (gate[i]->getFaninId(j))/2;
            if (gate[i]->getFaninId(j) % 2 != 0) 
               gate[i]->_inv[j] = true;
            fl->_type = "UNDEF";
            _undefined ++;
            gate[i]->_faninList.push_back(fl);
            _flId.push_back(fl->_varId);
            _FL.push_back(fl);
            _TOTAL[fl->_varId] = fl;
         }
      }
   }
}

void 
CirMgr::setFanoutList(vector <CirGate* > gate)
{
   for (size_t i = 0; i < gate.size(); i++){
      for (size_t j = 0; j < gate[i]->_faninList.size(); j++){
         gate[i]->_faninList[j]->_fanoutList.push_back(gate[i]);
         if (gate[i]->_faninList.size() == 2 
           &&gate[i]->_faninList[0]->_varId == gate[i]->_faninList[1]->_varId
           &&gate[i]->_inv[0] == gate[i]->_inv[1])  
            break;
      }
   }
}
void 
CirMgr::setFlGate(vector <CirGate* > gate)
{
   for(size_t i = 0; i < gate.size(); i++){
      if (gate[i]->_fanoutList.size() == 0){   
         CirFlGate* fl = new CirFlGate;
         fl = (CirFlGate*) gate[i];
         _flId.push_back(fl->_varId);
         _FL.push_back(fl);
         _TOTAL[fl->_varId] = fl;
         _notUsed ++;
      }
   }
}
       
void
CirMgr::setConst(CirGate* gate)
{
   CirConsGate* constant = new CirConsGate;
   if (_CONST.size() == 0){
      constant->_varId = 0;
      constant->_lineNo = 0;
      constant->_type = "CONST";
      _CONST.push_back(constant);
      _TOTAL[0] = constant; 
    }
    if (gate != 0)
       gate->_faninList.push_back(constant);

}

void
CirMgr::setTotalList()
{
   for (size_t i = 0; i < _PI.size(); i++)
      _TOTAL[_PI[i]->_varId] = _PI[i];
   for (size_t i = 0; i < _PO.size(); i++)
      _TOTAL[_PO[i]->_varId] = _PO[i];
   for (size_t i = 0; i < _AIG.size(); i++)
      _TOTAL[_AIG[i]->_varId] = _AIG[i];

}

