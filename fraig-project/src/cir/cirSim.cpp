/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <bitset>
#include <string>
#include <cmath>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

static size_t size = sizeof (unsigned long int) * 8;
/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
   vector <vector<string> > pattern;
   size_t numOfPat = sqrt(_NET.size());
   pattern.resize(numOfPat);
   
   for (size_t i = 0; i < numOfPat; i++)
      pattern[i].resize(_PI.size());
   for (size_t i = 0; i < numOfPat; i++)
      for (size_t j = 0; j < _PI.size(); j++)
         for (size_t k = 0; k < size; k++){  
            if (rnGen(2) == 1)  pattern[i][j] += "1";
            else pattern[i][j] += "0";
         }
   
   simulate(pattern);
   
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   vector <vector<string> > pattern;
   int line = 0;
   string buf;
   int numOfPat = 0;
   while (patternFile >> buf){
      int input = 0;
      while (buf[input] != 0){
         if (input == 0 && line % size == 0){ 
            numOfPat++;
            vector <string> pat;
            pattern.push_back(pat);
            pattern[numOfPat-1].resize(_PI.size());
         }
         pattern[numOfPat-1][input] += buf[input];
         input++;
      } 
      line ++;
   }
          
   simulate(pattern);
}



/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

void
CirMgr::simulate(vector <vector <string> > &pattern)
{
   for (size_t i = 0; i < pattern.size(); i++){
      for (size_t j = 0; j < _PI.size(); j++){
         unsigned long int pat;
         pat = strtoul(pattern[i][j].c_str() ,NULL,2);
         _PI[j]->_sim = pat;
         _PI[j]->setToGlobalRef();
      }
      _CONST[0]->_sim = 0;
      for (size_t j = 0; j < _NET.size(); j++){
         if (_NET[j]->_faninList.size() == 2)
            setSimVal(_NET[j]); 
      }
      if (_NET.size() != 0) _NET[0]->globalRefPlus1();
      
      for (size_t j = 0; j < _PO.size(); j++){
         if (_PO[j]->_inv[0] == 1)
            _PO[j]->_sim = ~_PO[j]->_faninList[0]->_sim;
         else 
            _PO[j]->_sim = _PO[j]->_faninList[0]->_sim;
      }
      fecPair(fecGrps);
      if (_simLog != 0) outputFile(pattern, i);
   }
   for (size_t i = 0; i < fecGrps.size(); i++)
      sort(fecGrps[i].begin(), fecGrps[i].end());
   setGateFecGrp();
}

void 
CirMgr::setSimVal(CirGate* gate)
{
   if (gate->_faninList[0]->isGlobalRef() && gate->_faninList[1]->isGlobalRef()){
      if (gate->isGlobalRef() == 0){
         if (gate->_inv[0] == 1){
            if (gate->_inv[1] == 1)
               gate->_sim = (~gate->_faninList[0]->_sim) & (~gate->_faninList[1]->_sim);
            else 
               gate->_sim = (~gate->_faninList[0]->_sim) & (gate->_faninList[1]->_sim);
         }
         else{
            if (gate->_inv[1] == 1)
               gate->_sim = (gate->_faninList[0]->_sim) & (~gate->_faninList[1]->_sim);
            else 
               gate->_sim = (gate->_faninList[0]->_sim) & (gate->_faninList[1]->_sim);
         
         } 
         gate->setToGlobalRef();
      }
   } 
}
  
void 
CirMgr::outputFile(vector<vector<string> > &pattern, size_t i)
{
   vector <string> output;
   output.resize(_PO.size());
   size_t correct = size - pattern[i][0].size();
  
   for (size_t j = 0; j < _PO.size(); j++){
      bitset<sizeof(unsigned long int) * 8> outputbits = _PO[j]->_sim;
      output[j] = outputbits.to_string<char,string::traits_type, string::allocator_type>(); 
   }

   for (size_t k = 0; k < pattern[i][0].size(); k++){  
      for (size_t j = 0; j < pattern[i].size(); j++)
         *_simLog<<pattern[i][j][k]; 
      *_simLog<<" ";
      for (size_t j = 0; j < output.size(); j++)
         *_simLog<<output[j][k + correct];
      *_simLog<<endl;
   }
}

void 
CirMgr::fecPair(vector< vector<size_t> > &fecGrps)
{
   vector<size_t> v;
   size_t fecGrp;
   size_t sim;
   
   if (fecGrps.size() == 0) {
      fecGrps.resize(1);
      fecGrps[0].push_back(_CONST[0]->_varId);
      for (size_t i = 0; i < _NET.size(); i++)
         if (_NET[i]->getTypeStr() == "AIG")
            fecGrps[0].push_back(_NET[i]->_varId);
   }
   for (size_t i = 0; i < fecGrps.size(); i++){
      HashMap<HashKey, size_t> hash(size);
      for (size_t j = 0; j < fecGrps[i].size(); j++){
         if (fecGrps[i].size() == 1) break;
         sim = getGate(fecGrps[i][j])->_sim;
         HashKey key(v, sim, 1, 0, 0);
         if (j == 0) hash.insert(key,fecGrps[i][0]);
         else if (hash.check(key, fecGrp)){
            if (getGate(fecGrps[i][j])->_grpNo != getGate(fecGrp)->_grpNo){
               getGate(fecGrps[i][j])->_grpNo = getGate(fecGrp)->_grpNo;
               fecGrps[getGate(fecGrps[i][j])->_grpNo].push_back(fecGrps[i][j]);
               fecGrps[i].erase(fecGrps[i].begin() + j);
               j--;
            }
         }
         else{
            hash.forceInsert(key, fecGrps[i][j]);
            fecGrps.resize(fecGrps.size() + 1);
            fecGrps[fecGrps.size() - 1].push_back(fecGrps[i][j]);
            getGate(fecGrps[i][j])->_grpNo = fecGrps.size() - 1;
            fecGrps[i].erase(fecGrps[i].begin() + j);
            j--;
         }
      }
   }
   collectFecPair();
   //cout<<"Total fec group"<<fecGrps.size()<<endl;
   
}

void
CirMgr::collectFecPair()
{
   vector < vector<size_t> > temp;
   for (size_t i = 0; i < fecGrps.size(); i++){
      if (fecGrps[i].size() > 1)
         temp.push_back(fecGrps[i]);
   } 
   for (size_t i = 0; i < fecGrps.size(); i++)
      fecGrps[i].clear();
   fecGrps.clear();
   for (size_t i = 0; i < temp.size(); i++)
      fecGrps.push_back(temp[i]);
}

void 
CirMgr::setGateFecGrp()
{
   for (size_t i = 0; i < fecGrps.size(); i++)
      for (size_t j = 0; j < fecGrps[i].size(); j++)
         for (size_t k = 0; k < fecGrps[i].size(); k++){
            if (k != j)
               getGate(fecGrps[i][j])->_fecGrp.push_back(getGate(fecGrps[i][k]));
         }  
}

