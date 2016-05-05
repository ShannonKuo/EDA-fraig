/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <math.h>
#include <string>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static vector<vector<string> > usefulPattern;
static int cntFail = 0;
/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
   size_t mergeGate;
   vector <size_t> vectorKey;
   HashMap<HashKey, size_t> hash(_maxVar + _poSize + 1);
   for (size_t i = 0; i < _NET.size(); i++){
      if (_NET[i]->getTypeStr() == "AIG"){
         vectorKey.clear();
         vectorKey.push_back(_NET[i]->_faninList[0]->_varId);
         vectorKey.push_back(_NET[i]->_faninList[1]->_varId);
         HashKey key(vectorKey, 0, 0, _NET[i]->_inv[0], _NET[i]->_inv[1]);

         if (hash.check(key, mergeGate)){
            merge(_NET[i], mergeGate, 0);
            cout<<"Strashing: "<<mergeGate<<" merging "<<_NET[i]->_varId<<"..."<<endl;
            _TOTAL[_NET[i]->_varId]->_del = 1;
            _TOTAL[_NET[i]->_varId] = 0;
         }
         else hash.forceInsert(key, _NET[i]->_varId);
      }
   }
   updateList();
}

void
CirMgr::fraig()
{
   SatSolver solver;
   solver.initialize();
   genProofModel(solver);
   sortFecGrp();
   int cntTotal = 0;
   while (fecGrps.size() != 0){
      for (int i = (int)fecGrps.size(); i > 0; i--){
         if (proofFecGrp(solver, fecGrps[i - 1])){
            fecGrps.pop_back();
         }
      } 
   }
   updateFecGrp();
   updateList();
   for (size_t i = 0; i < _maxVar + _poSize + 1; i++)
      if (_TOTAL[i] != 0)
         cout<<cntTotal++;
   /*int cnt = 0;//test
   int cntMerge = 0;
   static int cntSim = 0;
   SatSolver solver;
   vector <vector<size_t> > mergePair;
   
   solver.initialize();
   genProofModel(solver);

   for (size_t i = 0; i < fecGrps.size(); i++){
      if (cnt % (int)sqrt(_NET.size()) == 0){ 
         mergePair.resize((int)sqrt(_NET.size()));
         genProofModel(solver);
      }

      for (size_t j = 0; j < fecGrps[i].size(); j++){
         for (size_t k = j + 1; k < fecGrps[i].size(); k++){
             //delete the second one, fecGrps[i][k] is the second one;
             if (getGate(fecGrps[i][k])->_del != 1 && getGate(fecGrps[i][j])->_del != 1){
                cout<<"proof ("<<fecGrps[i][j]<<","<<fecGrps[i][k]<<") ..."<<endl;
                //cnt++;
                if (proofSat(solver, fecGrps[i][k], fecGrps[i][j])){
                   //cntMerge++;
                   mergePair.resize(mergePair.size() + 1);
                   mergePair[mergePair.size() - 1].push_back(fecGrps[i][j]);
                   mergePair[mergePair.size() - 1].push_back(fecGrps[i][k]);
                   getGate(fecGrps[i][k])->_del = 1;
                   fraigMerge(mergePair);
                   if (cntMerge == (int)sqrt(_NET.size())){
                      fraigMerge(mergePair, cntMerge);
                      mergePair.resize((int)sqrt(_NET.size()));
                      cntMerge = 0; 
                      if (usefulPattern.size() != 0) {
                         updateFecGrp();
                         updateList();
                         simulate(usefulPattern);
                         cntSim++;
                         cout<<"                       "<<cntSim<<endl;
                      }  
                   } 
                }
                else if (fecGrps[i].size() == 2){
                   getGate(fecGrps[i][j])->_del = 1;
                   getGate(fecGrps[i][k])->_del = 1;
                }
             }
         }
      }
   }
   updateFecGrp();
   updateList();


   cout<<"cntFail"<<cntFail<<endl;*/
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

void 
CirMgr::merge(CirGate* gate, size_t mergeGate, bool fecInv)
{
  for (size_t i = 0; i < gate->_fanoutList.size(); i++){
     for (size_t j = 0; j < gate->_fanoutList[i]->_faninList.size(); j++){
        if (gate->_fanoutList[i]->_faninList[j] == gate){
           gate->_fanoutList[i]->_faninList[j] = _TOTAL[mergeGate];
           if (fecInv){
              if (gate->_fanoutList[i]->_inv[j] == 1) 
                 gate->_fanoutList[i]->_inv[j] = 0;
              else gate->_fanoutList[i]->_inv[j] = 1;
           }
        }
     }
     _TOTAL[mergeGate]->_fanoutList.push_back(gate->_fanoutList[i]);
  }
  for (size_t i = 0; i < gate->_faninList.size(); i++)  
     for (size_t j = 0; j < gate->_faninList[i]->_fanoutList.size(); j++){
        if (gate->_faninList[i]->_fanoutList[j] == gate)
           gate->_faninList[i]->_fanoutList.erase
           (gate->_faninList[i]->_fanoutList.begin() + j);
  }
}

void 
CirMgr::sortFecGrp()
{
   for (size_t i = 0; i < _NET.size(); i++){
      for (size_t j = 0; j < fecGrps.size(); j++)
         for (size_t k = 0; k < fecGrps[j].size(); k++)
             if (fecGrps[j][k] == _NET[i]->_varId)
                getGate(fecGrps[j][0])->_distance += (i / fecGrps[j].size());
  
   }
   for (size_t i = 0; i < fecGrps.size(); i++)
      for (size_t j = i + 1; j < fecGrps.size(); j++)
         if (getGate(fecGrps[i][0])->_distance > getGate(fecGrps[j][0])->_distance){
            swap(fecGrps[i], fecGrps[j]);
         }
             
}

void
CirMgr::updateList()
{
   int aigSize = (int)_AIG.size();
   for (int i = aigSize - 1; i >= 0; i--){
      if (_AIG[i]->_del == 1)
         _AIG.erase(_AIG.begin() + i);
   }
   for (size_t i = 0; i < _maxVar + _poSize + 1; i++)
      if (_TOTAL[i] != 0)
         if (_TOTAL[i]->_del == 1) _TOTAL[i] = 0;
   _NET.clear();
   for (size_t i = 0; i < _maxVar + _poSize + 1; i++)
      if (_TOTAL[i] != 0) _TOTAL[i]->_dfs = 0;
   for (size_t i = 0; i < _PO.size(); i++)
      dfsTraversal(_PO[i]);
   _PO[0]->globalRefPlus1();
   setFlGate(_AIG);
   setFlGate(_PI);
}

void 
CirMgr::updateFecGrp()
{   
   /*vector <vector<size_t> > temp;
   for (size_t i = 0; i < fecGrps.size(); i++){
      for (size_t j = 0; j < fecGrps[i].size(); j++){
         if (getGate(fecGrps[i][j])->_del == 1){
            cout<<"erase                     erase"<<endl;
            _TOTAL[fecGrps[i][j]] = 0;
            fecGrps[i].erase(fecGrps[i].begin() + j);
            j--;
         }
      }
      if (fecGrps[i].size() > 1)
         temp.push_back(fecGrps[i]);
   }
   fecGrps.clear();
   for (size_t i = 0; i < temp.size(); i++)
      fecGrps.push_back(temp[i]);
   */
  for (size_t i = 0; i < _NET.size(); i++)
      _NET[i]->_fecGrp.clear();
   _CONST[0]->_fecGrp.clear();
}

bool 
CirMgr::proofFecGrp(SatSolver& solver, vector<size_t> fecGrp)
{
   bool fecInv = 0;
   int cnt = 0;
   for (size_t i = 0, ii = fecGrp.size(); i < ii; i++){
      for (size_t j = i + 1, jj = fecGrp.size(); j < jj; j++){
         if (getGate(fecGrp[i])->_del == 0 
          && getGate(fecGrp[i])->_del == 0){
            if (proofSat(solver, fecGrp[i], fecGrp[j], cnt)){
               fecInv = 0;
               cout<<"Fraig: "<<fecGrp[i]<<" merging ";
               if (getGate(fecGrp[i])->_sim != getGate(fecGrp[j])->_sim) {
                  cout<<"!";
                  fecInv = 1;
               }
               cout<<fecGrp[j]<<"..."<<endl; 
               if (getGate(fecGrp[i])->getTypeStr() == "AIG")
                  if (getGate(fecGrp[i])->_faninList[0] == getGate(fecGrp[j])
                    ||getGate(fecGrp[i])->_faninList[1] == getGate(fecGrp[j]))
                     swap(fecGrp[i], fecGrp[j]);
               merge(getGate(fecGrp[j]), fecGrp[i], fecInv);
               getGate(fecGrp[j])->_del = 1;
            }
            else if (cnt > ((int)sqrt(fecGrp.size())) && fecGrp.size() > 10) return true; 
         }
      }
   }
   return true;
}


bool 
CirMgr::proofSat(SatSolver& solver, size_t gate1, size_t gate2, int& cnt)
{
   bool result;
  
   Var newV = solver.newVar();
   cout<<"proof ("<<gate1<<","<<gate2<<")"<<fecGrps.size()<<endl;
   if (getGate(gate1)->_sim != getGate(gate2)->_sim)
      solver.addXorCNF(newV, getGate(gate1)->getVar(), false, getGate(gate2)->getVar(), true);
   else  
      solver.addXorCNF(newV, getGate(gate1)->getVar(), false, getGate(gate2)->getVar(), false);//test
   solver.assumeRelease();
   solver.assumeProperty(_CONST[0]->getVar(), false);
   solver.assumeProperty(newV, true);
   result = solver.assumpSolve();
   if (result == 1){
      cnt ++;
      cout<<"proof fail("<<gate1<<","<<gate2<<")"<<fecGrps.size()<<endl;
      /*usefulPattern.resize(usefulPattern.size() + 1);
      for (size_t i = 0; i < usefulPattern.size(); i++)
         usefulPattern[i].resize(_PI.size());
      for (size_t i = 0; i < _PI.size(); i++){
         if (_PI[i]->_dfs == 1){
            if (solver.getValue(_PI[i]->getVar()) == 1)
               usefulPattern[usefulPattern.size() - 1][i] += "1";
            else usefulPattern[usefulPattern.size() - 1][i] += "0";
         }
      }*/
   }
   return !result;
}

void
CirMgr::genProofModel(SatSolver& s)
{
   Var v = s.newVar();
   if (_CONST.size() != 0) _CONST[0]->setVar(v);
   for (size_t i = 0; i < _NET.size(); i++){
      if (_NET[i]->getTypeStr() != "PO"){
         Var v = s.newVar();
         _NET[i]->setVar(v);
      }
   }
   for (size_t i = 0; i < _PO.size(); i++) 
      _PO[i]->setVar(_PO[i]->_faninList[0]->getVar());
   for (size_t i = 0; i < _NET.size(); i++){
      if (_NET[i]->getTypeStr() == "AIG")
         s.addAigCNF(_NET[i]->getVar(),
                     _NET[i]->_faninList[0]->getVar(), _NET[i]->_inv[0], 
                     _NET[i]->_faninList[1]->getVar(), _NET[i]->_inv[1]);
   
   }
}

void
CirMgr::fraigMerge(vector<vector<size_t> >& mergePair)
{
   bool fecInv;
   for (size_t i = 0; i < mergePair.size(); i++){
      if (getGate(mergePair[i][0])->_sim != getGate(mergePair[i][1])->_sim)
         fecInv = 1;
      else fecInv = 0;
      merge(getGate(mergePair[i][1]), mergePair[i][0], fecInv);
      cout<<"Fraig: "<<mergePair[i][0]<<" merging ";
      if (fecInv) cout<<"!"; 
      cout<<mergePair[i][1]<<"..."<<endl;   
   }
   mergePair.clear();
}


