/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include <string>
#include <bitset>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr *cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h
int CirGate :: _globalRef = 1;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
  stringstream ss;
  ss << getLineNo();
  stringstream ss2;
  ss2 << _varId;
  stringstream ss3;
  string tmp;
  string tmp2;
  size_t s = 0x80000000;

  if (_symbol != "")
     tmp = "= "+getTypeStr()+"("+ ss2.str() +")"
           + "\"" + _symbol + "\"" + ", line "+ ss.str();
  else 
     tmp = "= "+getTypeStr()+"("+ ss2.str() +")"
           +", line "+ ss.str();
  cout<<"=================================================="<<endl
      <<setw(49)<<left<<tmp<<"="<<endl;
  
  for (size_t i = 0; i < _fecGrp.size(); i++){
    if (_fecGrp[i]->_sim != _sim) ss3<<"!";
     ss3<<_fecGrp[i]->_varId<<" ";
  }
  tmp2 = "= FECs: " + ss3.str();
  cout<<setw(49)<<left<<tmp2<<"="<<endl;
  
  cout<<"= Value: ";
  for (int i = 0; i < 32; i++) {
     if (i % 4 == 0 && i != 0) cout<<"_";
     if (s & _sim) cout<<"1";
     else cout<<"0";
     s = s >> 1;
  }
  cout<<" ="<<endl;
  cout<<"=================================================="<<endl;

}
void
CirGate::reportFanin(int level) const
{
    assert (level >= 0);
    cout<<getTypeStr()<<" "<<_varId<<endl;

    dfsTraversalIn(level, 0, 0);
    globalRefPlus1();
}
int
CirGate::dfsTraversalIn(int level, int cnt, int lev) const
{
   assert (level >= 0);
   if (cnt == level) return lev;
   for (size_t i = 0; i < _faninList.size(); i++){
      if (!_faninList[i]->isGlobalRef() 
         || _faninList[i]->getTypeStr() == "PI") {
           cnt++;
           if (cnt > lev) lev++;
           if (_faninList[i]->_faninList.size() != 0 && cnt < level)
              _faninList[i]->setToGlobalRef();
           for (int j = 0; j < cnt; j++) cout<<"  ";
           if (_inv[i] == true) cout<<"!";
           cout<<_faninList[i]->getTypeStr()<<" "
               <<_faninList[i]->_varId<<endl;
           lev = _faninList[i]->dfsTraversalIn(level,cnt,lev);
           cnt--;
       }
       else if (_faninList[i]->getTypeStr() == "AIG"){
          cnt++;
          if (cnt > lev) lev++;
          if (cnt > level) return lev;
          for (int j = 0; j < cnt; j++) cout<<"  ";
          if (_inv[i] == true) cout<<"!";
          cout<<_faninList[i]->getTypeStr()<<" "
              <<_faninList[i]->_varId;
          if (cnt < level)cout<<" (*)";
          cout<<endl;
          cnt--;
       } 
   }
   return lev;  

}
void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   cout<<getTypeStr()<<" "<<_varId<<endl;
   dfsTraversalOut(level, 0, 0);
   globalRefPlus1();

}

int
CirGate::dfsTraversalOut(int level, int cnt, int lev) const
{
   assert (level >= 0);

   if (cnt == level) return lev;

   for (size_t i = 0; i < _fanoutList.size(); i++){
      if (!_fanoutList[i]->isGlobalRef()){ 
         cnt++;
         if (cnt > lev) lev++;
         for (int j = 0; j < cnt; j++) cout<<"  ";
         for (size_t j = 0; j < _fanoutList[i]->_faninList.size(); j++){
            if (_fanoutList[i]->_faninList[j]->_varId == _varId){
               if (_fanoutList[i]->_inv[j] == true){
                  cout<<"!";
                  break;
               }
            }
         }
         cout<<_fanoutList[i]->getTypeStr()<<" "
             <<_fanoutList[i]->_varId<<endl;
         if (_fanoutList[i]->_fanoutList.size() != 0 && cnt != lev)
            _fanoutList[i]->setToGlobalRef();
         lev = _fanoutList[i]->dfsTraversalOut(level,cnt,lev);
         cnt--;
      }
      else if (_fanoutList[i]->getTypeStr() == "AIG"){
         cnt++;
         if (cnt > lev) lev++; 
         if (cnt > level) return lev;
         for (int j = 0; j < cnt; j++) cout<<"  ";
         for (size_t j = 0; j < _fanoutList[i]->_faninList.size(); j++){
            if (_fanoutList[i]->_faninList[j]->_varId == _varId){
               if (_fanoutList[i]->_inv[j] == true){
                  cout<<"!";
                  break;
               }
            }
         }
         cout<<_fanoutList[i]->getTypeStr()<<" "
             <<_fanoutList[i]->_varId;
        
         if (cnt < lev) cout<<" (*)";
         cout<<endl;
         cnt--;
      }
   }
   return lev;   
  

}

