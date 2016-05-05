/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"
using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate
{
   friend class CirMgr;

public:
   CirGate(): _lineNo(0), _grpNo(0), _varId(0) {
      _inv[0] = 0;
      _inv[1] = 0;
      _del = 0;
      _symbol = "";
   }
   virtual ~CirGate() {}

   // Basic access methods
   virtual string getTypeStr() const { return ""; }
   unsigned getLineNo() const { return _lineNo; }
   //size_t getSim() const { return _sim; }
   virtual bool isAig() { if (getTypeStr() == "AIG") return true; else return false; }
   virtual size_t getFaninId(size_t i) { return _faninId[i]; }
   virtual size_t getFaninIdSize() { return _faninId.size(); }
   virtual void getFaninVarId(size_t i) {
      if (_faninId[i] % 2 == 1) cout<<"!";
      cout<<_faninId[i]/2;
   }
   Var getVar() const {return _newVar;}
   void setVar(const Var& v) {_newVar = v;}
   void setToGlobalRef() {_ref = _globalRef;}
   bool isGlobalRef() {return (_ref == _globalRef);}
   static void globalRefPlus1(){_globalRef++;}
   int dfsTraversalIn(int level, int, int) const;
   int dfsTraversalOut(int level, int, int) const;


   // Printing functions
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;

protected:
   bool               _inv[2];
   bool               _del;
   bool               _dfs;
   int                _lineNo;
   int                _grpNo;
   int                _distance;
   int                _proofCnt;
   size_t             _varId;
   Var                _newVar;
   unsigned long int  _sim;
   static int         _globalRef;
   mutable int        _ref;
   
   string             _type;
   string             _symbol;
   
   vector <CirGate *> _faninList; 
   vector <CirGate *> _fanoutList;
   vector <size_t>    _faninId;   

   vector <CirGate *> _fecGrp;
};

class CirAigGate : public CirGate
{
   friend class CirMgr;
public:  
   CirAigGate() : CirGate() {_type = "AIG";}
   virtual ~CirAigGate(){};
   virtual string getTypeStr() const { return _type; }
   virtual size_t getFaninId(size_t i){return _faninId[i];}
   virtual size_t getFaninIdSize(){return _faninId.size();}
   virtual void getFaninVarId(size_t i){
      if (_faninId[i] % 2 == 1) cout<<"!";
      cout<<_faninId[i]/2;
   }
   void setToGlobalRef() {_ref = _globalRef;}
   bool isGlobalRef() {return (_ref == _globalRef);}
   static void globalRefPlus1(){_globalRef++;}
protected:
   static int _globalRef;
   mutable int       _ref;
   
   string              _type;
   vector <CirGate *>  _faninList;
   vector <CirGate *>  _fanoutList;
   vector <size_t>     _faninId;
};

class CirPiGate : public CirGate
{
   friend class CirMgr;
public:
   CirPiGate() : CirGate() {_type = "PI";}
   virtual ~CirPiGate(){};
   virtual string getTypeStr() const { return _type; }
protected:
   string             _type;
   vector <CirGate *>  _fanoutList;
};

class CirPoGate : public CirGate
{
   friend class CirMgr;
public:
   CirPoGate() : CirGate() {_type = "PO";}
   virtual ~CirPoGate(){};
   virtual string getTypeStr() const { return _type; }
   virtual size_t getFaninId(size_t i){return _faninId[i];}
   virtual size_t getFaninIdSize(){return _faninId.size();}
   virtual void getFaninVarId(size_t i){
      if (_faninId[i] % 2 == 1) cout<<"!";
      cout<<_faninId[i]/2;
   }
protected:
   string              _type;
   vector <CirGate *>  _faninList;
   vector <CirGate *>  _fanoutList;
   vector <size_t>     _faninId;
};

class CirConsGate : public CirGate
{
   friend class CirMgr;
public:
   CirConsGate() : CirGate() {_type = "CONST";}
   virtual ~CirConsGate(){};
   virtual string getTypeStr() const { return _type; }
protected:
   string             _type;
   vector <CirGate *>  _fanoutList;
};

class CirFlGate : public CirGate
{
   friend class CirMgr;
   friend bool compare(CirGate*, CirGate* );
public:   
   CirFlGate() : CirGate(){};
   virtual ~CirFlGate(){};
   virtual string getTypeStr() const { return _type; }
   virtual void getFaninVarId(size_t i){
      if (_faninId[i] % 2 == 1) cout<<"!";
      cout<<_faninId[i]/2;
   }

   virtual size_t getFaninIdSize(){return _faninId.size();}
protected:
   string             _type;
   vector <CirGate *> _faninList;
   vector <CirGate *> _fanoutList;
   
};
#endif // CIR_GATE_H
