/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
   for (size_t i = 0; i < _maxVar + _poSize + 1; i++){
      if (_TOTAL[i] != 0){
         if (_TOTAL[i] -> _dfs == 0){
            if (_TOTAL[i]->getTypeStr() != "CONST" && _TOTAL[i]->getTypeStr() != "PI")
               _TOTAL[i]->_del = 1;
         }
      }
   }
   for (size_t i = 0; i < _maxVar + _poSize + 1; i++){
      if (_TOTAL[i] != 0){
         for (size_t j = 0; j < _TOTAL[i]->_faninList.size(); j++)
            if (_TOTAL[i]->_faninList[j]->_del == 0){
               for (size_t k = 0; k < _TOTAL[i]->_faninList[j]->_fanoutList.size(); k++)
                   if (_TOTAL[i]->_faninList[j]->_fanoutList[k] == _TOTAL[i]){
                      _TOTAL[i]->_faninList[j]->
                      _fanoutList.erase(_TOTAL[i]->_faninList[j]->_fanoutList.begin() + k);
                      k--;
                   }
            }
      }
   }
   for (size_t i = 0; i < _maxVar + _poSize + 1; i++){
      if (_TOTAL[i] != 0 && _TOTAL[i]->_del == 1){
         cout<<"Sweeping: "<<_TOTAL[i]->getTypeStr()<<"("<<i<<") removed..."<<endl;
         _TOTAL[i] = 0;
      }
   }

     
   updateSweep();
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
   size_t aigSize = _AIG.size();
   vector <size_t> del;
   bool change = 1;
   
   if (_aigSize == 0) return;
   while(change){
      change = 0;
      del.clear();
      for (size_t i = 0; i < _NET.size(); i++){
         if (_NET[i] != 0 && _NET[i]->_del != 1 && _NET[i]->getTypeStr() == "AIG"){
            for (size_t j = 0; j < _NET[i]->_faninList.size(); j++){
               if (_NET[i]->_faninList[j]->getTypeStr() == "CONST"){ 
                  //One of the fanin is 0;
                  if (_NET[i]->_inv[j] == 0 && _NET[i]->_del != 1) 
                     faninIsZero(_NET[i], j);
                  
                  //One of the fanin is 1;
                  if (_NET[i]->_inv[j] == 1 && _NET[i]->_del != 1){
                     if (j == 0) 
                        optSetUpd(_NET[i], _NET[i]->_faninList[1], 1, 1);
                     else optSetUpd(_NET[i], _NET[i]->_faninList[0], 0, 1);
                  }
                  del.push_back(i);
                  _NET[i]->_del = 1;
                  break;
               }
            }
         
            if (_NET[i]->_faninList[0]->_varId == _NET[i]->_faninList[1]->_varId
              &&_NET[i]->_del != 1){
               //faninIsSame
               if (_NET[i]->_inv[0] == _NET[i]->_inv[1])
                  optSetUpd(_NET[i], _NET[i]->_faninList[0], 0, 1);
               //faninInv
               else {
                  if (_NET[i]->_inv[0] == 0) faninInv(_NET[i],0);
                  else faninInv(_NET[i],1);
               }
               del.push_back(i);
               _NET[i]->_del = 1;
            } 
         }
      }
      change = updateOpt(del, change);
      
   }
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
/*
void
CirMgr::dfsSweep(CirGate* gate)
{
   bool doNotDel = 0;
   for (size_t i = 0; i < gate->_faninList.size(); i++){ 
      size_t _fanoutListSize = gate->_faninList[i]->_fanoutList.size();
      for (size_t j = 0; j < _fanoutListSize; j++){
         if (gate->_faninList[i]->_del == 0){
            if (gate->_faninList[i]->_fanoutList[j]->_del == 0)
               doNotDel = 1;
            else 
               gate->_faninList[i]->
               _fanoutList.erase(gate->_faninList[i]->_fanoutList.begin()+j);
         }
      }   
      if (!doNotDel && gate->_faninList[i]->_del == 0
         &&gate->_faninList[i]->getTypeStr() != "CONST"){
         if (gate->_faninList[i]->getTypeStr() == "PI"){
            _FL.push_back(gate->_faninList[i]);
            _flId.push_back(gate->_faninList[i]->_varId);
            _notUsed++;
         }
         else {
            gate->_faninList[i]->_del = 1;
            doNotDel = 0;
            dfsSweep(gate->_faninList[i]);
         }     
      }
   }
}
*/
void
CirMgr::updateSweep()
{
   int _FLSize = _FL.size();
   int _AIGSize = _AIG.size();
   if (_FLSize != 0){
      for (int i = _FLSize - 1; i >= 0; i--){
         if (_FL[i]->_del == 1){
            if (_FL[i]->getTypeStr() == "UNDEF") _undefined--;
            else _notUsed--;
            _FL.erase(_FL.begin()+i);
            _flId.erase(_flId.begin()+i);
         }
      }
   }
   if (_AIGSize != 0){
      for (int i = _AIGSize - 1; i >= 0; i--)
         if (_AIG[i]->_del == 1)   _AIG.erase(_AIG.begin()+i);
   }
   _NET.clear();
   _notUsed = 0;

   for (size_t i = 0; i < _maxVar + _poSize + 1; i++)
      if (_TOTAL[i] != 0) _TOTAL[i]->_dfs = 0;
   for (size_t i = 0; i < _PO.size(); i++)
      dfsTraversal(_PO[i]);
   _PO[0]->globalRefPlus1();
   setFlGate(_AIG);
   setFlGate(_PI);
}


void 
CirMgr::optSetUpd(CirGate* gate, CirGate* fanin, size_t in, bool upd)
{
   for (size_t i = 0; i < gate->_fanoutList.size(); i++){
      for (size_t j = 0; j < gate->_fanoutList[i]->_faninList.size(); j++){
         if (gate->_fanoutList[i]->_faninList[j] == gate){
            gate->_fanoutList[i]->_faninList[j] = fanin;
            if (gate->_fanoutList[i]->_inv[j] == 1 && gate->_inv[in] == 1)
               gate->_fanoutList[i]->_inv[j] = 0;
            else if (gate->_fanoutList[i]->_inv[j] == 1 || gate->_inv[in] == 1)
               gate->_fanoutList[i]->_inv[j] = 1;
            else gate->_fanoutList[i]->_inv[j] = 0;
         }
      }
      if (upd){
         for (size_t k = 0; k < gate->_faninList[in]->_fanoutList.size(); k++){
             if (gate->_faninList[in]->_fanoutList[k] == gate){
                gate->_faninList[in]->_fanoutList.clear();
                gate->_faninList[in]->_fanoutList.push_back(gate->_fanoutList[i]);
             }
          }
      }
   } 
   cout<<"Simplifying: "<<fanin->_varId<<" merging ";
   if (gate->_inv[in]) cout<<"!";
   cout<<gate->_varId<<"..."<<endl;
}

void
CirMgr::faninIsZero(CirGate* gate, size_t id)
{
   //_gate->_faninList[id] = CONST 0
   size_t input; 
   if (id == 0) input = 1; else input = 0;
   optSetUpd(gate, _CONST[0], id, 1);
   for (size_t i = 0; i < gate->_faninList[input]->_fanoutList.size(); i++)
      if (gate->_faninList[input]->_fanoutList[i] == gate)
         gate->_faninList[input]->
         _fanoutList.erase(gate->_faninList[input]->_fanoutList.begin() + i);
}

void
CirMgr::faninInv(CirGate* gate, size_t id)
{
   optSetUpd(gate, _CONST[0], id, 0);
   for (size_t i = 0; i < gate->_faninList.size(); i++)
      for (size_t j = 0; j < gate->_faninList[i]->_fanoutList.size(); j++)
         if (gate->_faninList[i]->_fanoutList[j] == gate)
             gate->_faninList[i]->
             _fanoutList.erase(gate->_faninList[i]->_fanoutList.begin() + j);
}

bool
CirMgr::updateOpt(vector <size_t> del, bool change)
{
   int delSize = (int)del.size();
   size_t flIdSize = _flId.size();
   for (size_t i = 0; i < _AIG.size(); i++)
      for (size_t j = 0; j < del.size(); j++)
            if (_AIG[i]->_varId == _NET[del[j]]->_varId){
               _AIG.erase(_AIG.begin() + i);
               i--;
               break;
            }
   if (delSize != 0){
      change = 1;
      for (int i = delSize - 1; i >= 0; i--){
         _TOTAL[_NET[del[i]]->_varId] = 0;
         //_AIG.erase(_AIG.begin() + del[i]);
      }
   }

   _NET.clear();
   _notUsed = 0;

   for (size_t i = 0; i < _maxVar + _poSize + 1; i++)
      if (_TOTAL[i] != 0) _TOTAL[i]->_dfs = 0;
   for (size_t i = 0; i < _PO.size(); i++)
      dfsTraversal(_PO[i]);
   _PO[0]->globalRefPlus1();
   if (flIdSize != 0){
      for(int i = flIdSize - 1; i >= 0; i--)
         if (_TOTAL[_flId[i]] -> getTypeStr() != "UNDEF"){ 
            _flId.erase(_flId.begin() + i);
            _FL.erase(_FL.begin() + i);
         }
   }
   setFlGate(_AIG);
   setFlGate(_PI);
   return change;
}
