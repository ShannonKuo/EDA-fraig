/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"
#include "cirGate.h"
#include "sat.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr():_undefined(0),_notUsed(0){}
   ~CirMgr() {
      for (size_t i = 0; i < _maxVar+_poSize+1; i++)
         delete _TOTAL[i];
   } 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const;

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
      
   void optimize();
   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void dfsTraversal(CirGate*);
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

private:
   ofstream           *_simLog;
   void setFaninList(vector <CirGate*>);
   void setFanoutList(vector <CirGate*>);
   void setFlGate(vector <CirGate*>);
   void setConst(CirGate*);
   void setTotalList();
//sweep   
   //void dfsSweep(CirGate*);
   void updateSweep();

//optimize
   void optSetUpd(CirGate*, CirGate*, size_t, bool );
   void faninIsZero(CirGate*, size_t);
   void faninInv(CirGate*, size_t);
   bool updateOpt(vector <size_t>, bool);

//strash
   void merge(CirGate*, size_t, bool);

//simulate
   void simulate(vector<vector<string> > &);
   void setSimVal(CirGate*);
   void outputFile(vector<vector<string> > &, size_t);
   void fecPair(vector<vector<size_t> > &);
   void collectFecPair();
   void setGateFecGrp();

//fraig
   void sortFecGrp();
   bool proofFecGrp(SatSolver& ,vector<size_t>);
   bool proofSat(SatSolver&, size_t, size_t, int&);
   void updateList();
   void updateFecGrp();
   void genProofModel(SatSolver&);
   void fraigMerge(vector<vector<size_t> >&);

   vector <CirGate*>         _PI;
   vector <CirGate*>         _PO;
   vector <CirGate*>         _FL;
   vector <int>              _flId;
   vector <CirGate*>         _AIG;
   vector <CirGate*>         _CONST;
   vector <CirGate*>         _NET;
   CirGate **                _TOTAL;
   vector <vector<size_t> >  fecGrps;
   
   size_t        _piSize;
   size_t        _poSize;
   size_t        _aigSize;
   size_t        _maxVar;
   size_t        _undefined;
   size_t        _notUsed;


};

#endif // CIR_MGR_H
