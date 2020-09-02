//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2009 - 2017 The MINOTAUR Team.
//

/**
 * \file PerspCon.cpp
 * \brief Define base class for finding constraints amenable to
 *  perspective reformulation (PR) in the given problem.
 * \Author Meenarli Sharma, Indian Institute of Technology Bombay
 */

#include <cmath>
#include <iostream>
#include <algorithm>
#include <iterator>     // std::distance
#include <forward_list>


#include "Option.h"
#include "Logger.h"
#include "Function.h"
#include "PerspCon.h"
#include "LinearFunction.h"
#include "NonlinearFunction.h"
#include "QuadraticFunction.h"

using namespace Minotaur;
const std::string PerspCon::me_ = "PerspCon: ";

PerspCon::PerspCon()
:env_(EnvPtr()), p_(ProblemPtr()), cons_(0), bVar_(0), prConsVec_(0), 
  gubList0_(0), gubList1_(0)
{
  absTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
}
//MS: include all the declared variables in .h file

PerspCon::PerspCon(EnvPtr env, ProblemPtr p)
: env_(env), p_(p), cons_(0), bVar_(0),
  prConsVec_(0), gubList0_(0), gubList1_(0) 
{
  timer_ = env->getNewTimer();
  logger_ = env->getLogger();
  absTol_ = env_->getOptions()->findDouble("solAbs_tol")->getValue();
}


PerspCon::~PerspCon()
{
  if (timer_) {
    delete timer_;
  }
  
  // What else to reset here?
  p_ = 0;
  env_ = 0;
}


void PerspCon::populate_(UInt type)
{
 prCons p;
 p.type = type;
 p.cons = cons_;
 p.binVar = bVar_;
 p.binVal = binVal_;
 prConsVec_.push_back(p);
  return;
}


bool PerspCon::isControlled_(std::vector<VariablePtr> binaries)
{
  bool isFound;
  std::unordered_map<VariablePtr, std::forward_list<impliVar>> implications;

  if (binVal_) {
    implications = impli1_;
  } else {
    implications = impli0_;
  }
    
  iit_ = implications.begin();

  for (; iit_ != implications.end(); ++iit_) {
    bVar_ = iit_->first;
    isFound = false;
    for (std::vector<VariablePtr>::iterator it = binaries.begin();
         it!=binaries.end(); ++it) {
      if (bVar_ == *it ) {
        isFound = true;
        break;      
      }
    }  

    if (isFound) {
      continue;    
    } else {
      if (boundBinVar_()) {
        return true;      
      }
    }
  }

  return false;
}


bool PerspCon::boundBinVar_()
{
  double * x = new double[p_->getNumVars()];
  std::fill(x, x+p_->getNumVars(), 0.);
  bool boundsok = checkNVars_(x);

  if (boundsok) {
    int error = 0;
    // checking for on-off set of type 2
    double cu = cons_->getUb(), val = 0;
    QuadraticFunctionPtr qf = cons_->getQuadraticFunction();
    NonlinearFunctionPtr nlf = cons_->getNonlinearFunction();
    if (nlf) {
      val = nlf->eval(x, &error); 
    }

    if (error == 0) {
      if (qf) {
        val = val + qf->eval(x); 
      }

      if (cu - val >= absTol_) {
        boundsok = false;      
      } else {
        populate_(2); // type 2 on-off set 
      }
    } else {
      delete [] x;
      return false;
    }

    if (!boundsok) {
      LinearFunctionPtr lf = cons_->getLinearFunction();
      //check for on-off set of type 1
      if (lf) {
        boundsok = checkLVars_(x);
        val = val + lf->eval(x); 
        if (val >= cu + absTol_) {
          fixBinaryVar_(bVar_, binVal_);
          if (binVal_) {
            iit_ = impli1_.find(bVar_);
            impli1_.erase(iit_);
          } else {
            iit_ = impli0_.find(bVar_);
            impli0_.erase(iit_);
          }
        } else {
          populate_(1); // type 2 on-off set 
        }
      }
    }
  } 

  delete [] x;
  return boundsok;
}


bool PerspCon::checkLVars_(double *x)
{  
  ConstVariablePtr v;
  bool isFound;
  std::forward_list<impliVar>::iterator it1;
  const LinearFunctionPtr lf = cons_->getLinearFunction();
  std::unordered_map<VariablePtr, std::forward_list<impliVar>> implications;
   
  if (binVal_) {
    implications = impli1_;
  } else {
    implications = impli0_;
  }
 
  if (lf) {
    isFound = true;
    for (VariableGroupConstIterator it=lf->termsBegin(); it!=lf->termsEnd();
         ++it) {
      v = it->first;
      if (v == iit_->first) {
        continue; 
      }
      
      isFound = false;
      for (it1 = (iit_->second).begin(); it1 != (iit_->second).end(); ++it1) {
        if (v == (*it1).var) {
          isFound = true;
          x[v->getIndex()] = (*it1).fixedVal[0];
          break;
        }
      }

      if (!isFound) {
        return false;
      }
    }
    return true;
  }     
  return false;
}


bool PerspCon::checkNVars_(double *x)
{
  bool isFound;
  ConstVariablePtr v;
  std::forward_list<impliVar>::iterator it1;
  QuadraticFunctionPtr qf = cons_->getQuadraticFunction();
  NonlinearFunctionPtr nlf = cons_->getNonlinearFunction();

  if (nlf == 0 && qf == 0) {
    return false;
  }
   
  if (nlf) {
    for (VarSetConstIterator it=nlf->varsBegin(); it!=nlf->varsEnd(); ++it) {
      v = *it;
      if (v == iit_->first) {
        continue; 
      }
      
      isFound = false;
      for (it1 = (iit_->second).begin(); it1 != (iit_->second).end(); ++it1) {
        if (v == (*it1).var) {
          isFound = true;
          x[v->getIndex()] = (*it1).fixedVal[0];
          break;
        }
      }

      if (!isFound) {
        return false;
      }
    }
  } 

  if (qf) {
    for (VarIntMapConstIterator it=qf->varsBegin(); it!=qf->varsEnd(); ++it) {
      v = it->first;
      if (nlf && nlf->hasVar(v)) {
        continue;        
      }

      if (v == iit_->first) {
        continue; 
      }
      
      isFound = false;
      for (it1 = (iit_->second).begin(); it1 != (iit_->second).end(); ++it1) {
        if (v == (*it1).var) {
          isFound = true;
          x[v->getIndex()] = (*it1).fixedVal[0];
          break;
        }
      }

      if (!isFound) {
        return false;
      }
    }
  }
  return true;
}


void PerspCon::findBinVarsInCons_(std::vector<VariablePtr>* binaries)
{
  VariablePtr var;
  FunctionPtr f = cons_->getFunction();

  for (VarSetConstIterator it=f->varsBegin(); it!=f->varsEnd(); ++it) {
    var = *it;
    if (isBinary_(var)) {
      (*binaries).push_back(var);
    }
  }
  return;
}


void PerspCon::displayInfo_()
{
  //MS: make display better. Keep limited information remove S1 and S2
  prCons p;
  UInt s = prConsVec_.size();
  std::ostream &out = logger_->msgStream(LogInfo);
 
  std::forward_list<impliVar>::iterator it1;
  std::unordered_map<VariablePtr, std::forward_list<impliVar>>::iterator mit;
    
  out << "----------------------------------------------------"<< std::endl;
  out << "------- Variable Fixing ------------------------------"<< std::endl;
  out << me_ << "----------------z = 0 case-----------------------\n";
  for (mit = impli0_.begin(); mit != impli0_.end(); ++mit) {
    out << me_ << "---------------------------------------\n";
    out << me_ << "Binary var " << mit->first->getName() << "\n";
    for (it1 = (mit->second).begin(); it1 != (mit->second).end(); ++it1) {
      out << me_ << "var " << (*it1).var->getName() << " " << (*it1).fixedVal[0] << "\n";
    }
  }
  
  out << me_ << "--------------------------\n"; 
  out << me_ << "----------------z = 1 case-----------------------\n";
  for (mit = impli1_.begin(); mit != impli1_.end(); ++mit) {
    out << me_ << "---------------------------------------\n";
    out << me_ << "Binary var " << mit->first->getName() << "\n";
    for (it1 = (mit->second).begin(); it1 != (mit->second).end(); ++it1) {
      out << me_ << "var " << (*it1).var->getName() << " " << (*it1).fixedVal[0] << "\n";
    }
  }  

  out << "---------- PR details --------------------------------"<< std::endl;
  if (s > 0) {
    out << me_ <<"Total nonlinear constraints in problem = " <<
      p_->getNumCons() - p_->getNumLinCons() << std::endl; 
    out << me_ <<"Number of constraints amenable to PR = " << 
      prConsVec_.size() << std::endl;


    out << "----------------------------------------------------"<< std::endl;
    out << me_ << "Details of constraints amenable to perspective reformulation:";
    for (UInt i = 0; i < prConsVec_.size() ; ++i) {
      out << std::endl; 
      p = prConsVec_[i];
      out << i+1 << ". ";
      p.cons->write(out);
      out << "Structure type: S" << p.type << std::endl;
      out << "Associated binary variable and val: " << (p.binVar)->getName() << " and " <<
        (p.binVal) << std::endl;
    }
    out << "----------------------------------------------------"<< std::endl;
 
  } else {
    out << me_ <<"Number of constraints amenable to PR = 0" << std::endl;
  }
}


void PerspCon::evalConstraint_()
{
  bool isFound;
  std::vector<VariablePtr> binaries;
  /// Binary variables in the constraint con_
  findBinVarsInCons_(&binaries);
  for (std::vector<VariablePtr>::iterator it = binaries.begin();
       it!=binaries.end(); ++it) {
    bVar_ = *it;
    
    iit_ = impli0_.find(bVar_);
    if (iit_ != impli0_.end()) {
      binVal_ = 0;
      isFound = boundBinVar_();    
    }
    if (isFound) {
      return;
    } else {
      binVal_ = 1;
      iit_ = impli1_.find(bVar_);
      if (iit_ != impli1_.end()) {
        isFound = boundBinVar_();    
      }

      if (isFound) {
        return;
      }
    }
  }

  if (impli0_.size() > 0) {
    binVal_ = 0;
    isFound = isControlled_(binaries);
  }

  if (!isFound && impli1_.size() > 0) {
    binVal_ = 1;
    isFound = isControlled_(binaries);
  }

  return;
}


bool PerspCon::multiTermsFunc_(ConstraintPtr c, VariablePtr var, 
                            std::forward_list<impliVar> *varList, 
                            double val, bool z)
{
  VariablePtr v;
  impliVar impli;
  std::forward_list<impliVar>::iterator it1;
  LinearFunctionPtr lf = c->getLinearFunction();
  double vc, vlb, vub, lb = c->getLb(), ub = c->getUb();
  bool isFound = true, ubChecked = false, cFeas = false, cInf = false, 
       allBin = true;
  
  if (fabs(ub-val) < absTol_) {
    ubChecked = 1;
    for (VariableGroupConstIterator itvar=lf->termsBegin();
         itvar!=lf->termsEnd(); ++itvar) {
      v = itvar->first;
      if (v != var) {
        vc = itvar->second;
        if (z && allBin) {
          if (isBinary_(v)) {
            if ((fabs(vc - ub) >= absTol_)) {
              allBin = false;
            }
          } else {
            if ((fabs(vc) >= absTol_)) {
              allBin = false;
            }
          }
        }

        vlb = v->getLb();
        vub = v->getUb();
        if (vc >= absTol_) {
          if (fabs(vlb) < absTol_) {
            cFeas = 1;
          } else if (vlb >= absTol_) {
            cInf = 1; 
          } else {
            isFound = false;
            break;
          }
        } else if (vc <= -absTol_) {
          if (fabs(vub) < absTol_) {
            cFeas = 1;
          } else if (vub <= -absTol_) {
            cInf = 1; 
          } else {
            isFound = false;
            break;
          }
        }
      }
    }
  }

  if ((fabs(lb-val) < absTol_) && (fabs(lb-ub) >= absTol_)) {
    if ((ubChecked && !isFound) || (ubChecked == 0)) {
      isFound = true; cFeas = false; cInf = false;
      for (VariableGroupConstIterator itvar=lf->termsBegin();
           itvar!=lf->termsEnd(); ++itvar) {
        v = itvar->first;
        if (v != var) {
          vc = itvar->second;
          if (z && allBin) {
            if (isBinary_(v)) {
              if ((fabs(vc - lb) >= absTol_)) {
                allBin = false;
              }
            } else {
              if ((fabs(vc) >= absTol_)) {
                allBin = false;
              }
            }
          }

          vlb = v->getLb();
          vub = v->getUb();
          if (vc >= absTol_) {
            if (fabs(vub) < absTol_) {
              cFeas = 1;
            } else if (vub <= -absTol_) {
              cInf = 1; 
            } else {
              isFound = false;
              break;
            }
          } else if (vc <= -absTol_) {
            if (fabs(vlb ) < absTol_) {
              cFeas = 1;
            } else if (vlb >= absTol_) {
              cInf = 1; 
            } else {
              isFound = false;
              break;
            }
          } 
        }
      }   
    }
  }
  
  if (z && allBin) {
    bool consFound = false;
    for (std::forward_list<ConstraintPtr>::iterator cit = gubList1_.begin();
         cit != gubList1_.end(); ++cit) {
      if (c == *cit) {
        consFound = true;
        break;      
      }    
    }
    if (!consFound) {
      gubList1_.push_front(c);
    }
  }

  if (isFound) {
    if (cFeas && !cInf) {
      impli.fixedVal.push_back(0);
      impli.fixedVal.push_back(0);
      for (VariableGroupConstIterator itvar=lf->termsBegin();
           itvar!=lf->termsEnd(); ++itvar) {
        v = itvar->first;
        if ((v == var) || (fabs(itvar->second) < absTol_)) {
          continue;
        }
        for (it1 = (*varList).begin(); it1 != (*varList).end(); 
           ++it1) {
          if (v == (*it1).var) {
            break;
          }
        }
        if (it1 == (*varList).end()) {
          impli.var = v;
          impli.fixedVal[0] = 0;
          impli.fixedVal[1] = 0;
          (*varList).push_front(impli);
        } else {
          if ((*it1).fixedVal[0] <= -absTol_) {
            (*it1).fixedVal[0] = 0;
          }
          if ((*it1).fixedVal[1] >= absTol_) {
            (*it1).fixedVal[1] = 0;
          }
        }
      }
    } else if (cInf) {
      fixBinaryVar_(var, z);
      return true;
    }
  }
  return false;
}


bool PerspCon::twoTermsFunc_(ConstraintPtr c, VariablePtr var, 
                            std::forward_list<impliVar> *varList, bool z)
{
  VariablePtr v;
  impliVar impli;
  double vc, bc = 0, val, vlb, vub;
  bool isFixed = false, allBin = true;
  double lb = c->getLb(), ub = c->getUb();
  std::forward_list<impliVar>::iterator it1;
  LinearFunctionPtr lf = c->getLinearFunction();

  for (VariableGroupConstIterator itvar=lf->termsBegin();
       itvar!=lf->termsEnd(); ++itvar) {
    if (itvar->first != var) {
      v = itvar->first;
      vc = itvar->second;
      if (fabs(vc) < absTol_) {
        return false;
      }
      if (z || (fabs(lb-ub) < absTol_)) {
        if (isBinary_(v)) {
          if ((fabs(vc - ub) >= absTol_)) {
            allBin = false;
          }        
        } else {
          if ((fabs(vc) >= absTol_)) {
            allBin = false;
          }
        }
      } else {
        allBin = false;
      }

    } else {
      bc = itvar->second;
      if ((fabs(bc - ub) >= absTol_)) {
        allBin = false;
      }
      if (!z) {
        bc = 0;
      } 
    }
  }

  
  if (allBin) {
    bool consFound = false;
    if (z) {
      for (std::forward_list<ConstraintPtr>::iterator cit = gubList1_.begin();
           cit != gubList1_.end(); ++cit) {
        if (c == *cit) {
          consFound = true;
          break;      
        }    
      }
      if (!consFound) {
        gubList1_.push_front(c);
      }
    } else {
      for (std::forward_list<ConstraintPtr>::iterator cit = gubList0_.begin();
           cit != gubList0_.end(); ++cit) {
        if (c == *cit) {
          consFound = true;
          break;      
        }    
      }
      if (!consFound) {
        gubList0_.push_front(c);
      }
    }
  } 
  
  for (it1 = (*varList).begin(); it1 != (*varList).end(); 
        ++it1) {
    if (v == (*it1).var) {
      break;
    }
  } 
        
  vlb = v->getLb();
  vub = v->getUb();

  if (fabs(lb-ub) < absTol_) {
    val = (ub-bc)/vc;
    if ((val <= vlb - absTol_)  || (val >= vub + absTol_)) {
      isFixed = true;
    }
    if (isBinary_(v)) {
      if ((fabs(val) >= absTol_) && (fabs(val) <= 1-absTol_)) {
        isFixed = true;            
      }          
    }
    
    if (isFixed) {
      fixBinaryVar_(var, z);
      return true;
    }
    if (it1 == (*varList).end()) {
      impli.var = v;
      impli.fixedVal.push_back(val);
      impli.fixedVal.push_back(val);
      (*varList).push_front(impli);
    } else {
      if (((*it1).fixedVal[0] + absTol_) <= val) {
        (*it1).fixedVal[0] = val;
      }
      if (val <= ((*it1).fixedVal[1] - absTol_)) {
        (*it1).fixedVal[1] = val;
      }
    }
  } else {
    if (ub != INFINITY) {
      val = (ub-bc)/vc;
      if (vc >= absTol_) {
        if (val <= vlb - absTol_) {
          fixBinaryVar_(var, z);
          return true;
        }
        // obtained upper bound
        if (it1 == (*varList).end()) {
          impli.var = v;
          // new variable
          impli.fixedVal.push_back(vlb);
          impli.fixedVal.push_back(val);
          (*varList).push_front(impli);
        } else {
          // existing variable
          if (val <= (*it1).fixedVal[1] - absTol_) {
            (*it1).fixedVal[1] = val;
          }
        }
      } else {
        if (val >= vub + absTol_) {
          fixBinaryVar_(var, z);
          return true;
        }
         // obtained lower bound
         if (it1 == (*varList).end()) {
          impli.var = v;
          impli.fixedVal.push_back(val);
          impli.fixedVal.push_back(vub);
          (*varList).push_front(impli);
        } else {
          if (val >= (*it1).fixedVal[0] + absTol_) {
            (*it1).fixedVal[0] = val;
          }
        }               
      }              
    }

    if (lb != -INFINITY) {
      val = (lb-bc)/vc;
      if (vc >= absTol_) {
         if (val >= vub + absTol_) {
          fixBinaryVar_(var, z);
           return true;
         }
          // obtained lower bound
        if (it1 == (*varList).end()) {
          impli.var = v;
          impli.fixedVal.push_back(val);
          impli.fixedVal.push_back(vub);
          (*varList).push_front(impli);
        } else {
          if (val >= (*it1).fixedVal[0] + absTol_) {
            (*it1).fixedVal[0] = val;
          }
        }
      } else {
         if (val <= vlb - absTol_) {
          fixBinaryVar_(var, z);
           return true;
         }
        // obtained upper bound
        if (it1 == (*varList).end()) {
          impli.var = v;
          impli.fixedVal.push_back(vlb);
          impli.fixedVal.push_back(val);
          (*varList).push_front(impli);
        } else {
          if (val <= (*it1).fixedVal[1] - absTol_) {
            (*it1).fixedVal[1] = val;
          }
        }              
      }             
    }
  }
      
  return false;
}

void PerspCon::fixBinaryVar_(VariablePtr var, bool z)
{
  if (z) {
    // Infeasibility with var = 1. Fixing this variable to 0.
    var->setUb_(0);      
  } else {
    // Infeasibility with var = 0. Fixing this variable to 1.
    var->setLb_(1);
  }
  return;
}

void PerspCon::deriveImpli_(VariablePtr var)
{
  UInt numVars;
  impliVar impli;
  ConstraintPtr c;
  double lb, ub, val;
  LinearFunctionPtr lf;
  bool isFixed0 = false, isFixed1 = false;
  std::forward_list<impliVar>::iterator it1;
  std::forward_list<impliVar> varList0, varList1;

  // Iterate over each constraint in which variable var appears
  for (ConstrSet::iterator it=var->consBegin(); it!=var->consEnd(); ++it) {
    c = *it;
    // consider a cons if linear
    if (c->getFunctionType() == Linear) {
      lf = c->getLinearFunction();
      val = lf->getWeight(var);
      // continue if var has 0 coefficient
      if (fabs(val) < absTol_) {
        continue;      
      }
    
      numVars = lf->getNumTerms();
      if (numVars == 2) {
        // store vars whose lower or upper bound are set by var = 0
        isFixed0 = twoTermsFunc_(c, var, &varList0, 0);
        
        // if var is not fixed to 0, then store vars whose lower or upper 
        // bound are set by var = 1
        if (!isFixed0) {
          isFixed0 = twoTermsFunc_(c, var, &varList1, 1);
        }
      } else if (numVars > 2) {
        lb = c->getLb();
        ub = c->getUb();
        if ((fabs(lb) < absTol_) || (fabs(ub) < absTol_)) {
          isFixed0 = multiTermsFunc_(c, var, &varList0, 0, 0);
        } else {
          isFixed0 = false;
        }
        if (!isFixed0) {
          if ((fabs(lb-val) < absTol_) || (fabs(ub-val) < absTol_)) {
            isFixed0 = multiTermsFunc_(c, var, &varList1, val, 1);
          }
        }
      } 
     // if the binary var is fixed to either 0 or 1 then return 
      if (isFixed0) {
        return;
      }
    }
  }
  
  // based on bounds obtained obtain vars that can be controlled with var = 0
  // and 1
  isFixed0 = addImplications_(&varList0);
  isFixed1 = addImplications_(&varList1);

  if (!isFixed0 && !isFixed1) {
    // if var not fixed then save the implications obtained
    if (!(varList0.empty())) {
      impli0_.insert({var, varList0});  
    }
      
    if (!(varList1.empty())) {
      impli1_.insert({var, varList1});  
    }
  } else if (isFixed0 && isFixed1) {
    // if var is found to be fixed to both values then infeasible
    assert(!"In PerspCon: Problem infeasible.");
  } else if (isFixed0) {
    // Infeasibility with var = 0. Fixing this variable to 1.
    var->setLb_(1); 
  } else {
    // Infeasibility with var = 1. Fixing this variable to 0.
    var->setUb_(0);  
  }

  return;
}


bool PerspCon::addImplications_(std::forward_list<impliVar> *varList)
{
  /// Check this deletion once
  double lb, ub, valLb, valUb;
  std::forward_list<impliVar>::iterator it1;
  std::forward_list<impliVar>::iterator prev = (*varList).before_begin();  

  for (it1 = (*varList).begin(); it1 != (*varList).end(); ) {
    lb = (*it1).var->getLb();
    ub = (*it1).var->getUb();
    valLb = (*it1).fixedVal[0];
    valUb = (*it1).fixedVal[1];
    if ((valUb <= valLb - absTol_) || (lb - absTol_ >= valUb) 
        || (ub + absTol_ <= valLb)) {
      return true;
    }

    if (valLb + absTol_ <= valUb) {
      it1 = (*varList).erase_after(prev);
    } else {
      (*it1).fixedVal.pop_back();
      prev = it1;
      ++it1;    
    }
  }
  return false;
}


bool PerspCon::isBinary_(VariablePtr var)
{
  switch (var->getType()) {
  case Binary:
  case ImplBin:
    return true;
    break;
  case Integer:
  case ImplInt:
    if ((fabs(var->getLb()) < absTol_) && 
        (fabs(var->getUb() - 1) < absTol_)) {
      return true;
    } 
    break;
  default:
    break;
  }
  return false;
}


void PerspCon::removeSingleton_()
{
  VariablePtr var;
  ConstraintPtr c;
  LinearFunctionPtr lf;
  double vc, val1, val2;

 // Only if presolved is off perform this 
 // Move singleton constraints as bounds
  for (ConstraintConstIterator it=p_->consBegin(); it!=p_->consEnd(); ++it) {
    c = *it;
    lf = c->getLinearFunction();
    if ((c->getFunctionType() == Linear) && 
        lf->getNumTerms() == 1) {
      vc = lf->termsBegin()->second;
      var = lf->termsBegin()->first;
      if (vc > -absTol_) {
        val1 = c->getUb();
        if (val1 != INFINITY) {
          val2 = val1/vc ;
          if (val2 <= val1 - absTol_) {
            var->setUb_(val2);
          }        
        }
        val1 = c->getLb();
        if (val1 != -INFINITY) {
          val2 = val1/vc ;
          if (val2 >= val1 + absTol_) {
            var->setLb_(val2);
          }        
        }
      } else {
        val1 = c->getUb();
        if (val1 != INFINITY) {
          val2 = val1/vc ;
          if (val2 >= val1 + absTol_) {
            var->setLb_(val2);
          }        
        }
        val1 = c->getLb();
        if (val1 != -INFINITY) {
          val2 = val1/vc ;
          if (val2 <= val1 - absTol_) {
            var->setUb_(val2);
          }        
        }     
      }
    }
  }
  return;
}


void PerspCon::implications_()
{
  timer_->start();
  bool isFound;
  VariablePtr var;
  ConstraintPtr c;

  // Remove singleton if presolve is off
  if (env_->getOptions()->findBool("presolve")->getValue() == false) {
    removeSingleton_();
  }

  for (VariableConstIterator it = p_->varsBegin(); it != p_->varsEnd(); 
       ++it) {
    var = *it;
    // Find variables controlled by each binary variable
    if (isBinary_(var)) {
      deriveImpli_(var);
    }
  }

  // Return if no implication is found
  if ((impli0_.size() == 0) && (impli1_.size() == 0)) {
    return;  
  }

  // Derive more implications for equality constraints in which only one var
  // is not controlled but others are controlled by some binary variable
  for (ConstraintConstIterator it=p_->consBegin(); it!=p_->consEnd(); ++it) {
    c = *it;
    if ((c->getFunctionType() == Linear) && 
        (fabs(c->getLb() - c->getUb()) < absTol_)) {
      // Case: binary var when fixed to 0
      if (impli0_.size() != 0) {
        isFound = false;
        // Consider constraint which is not in gubList0_
        if (c->getLinearFunction()->getNumTerms() == 2) {
          for (std::forward_list<ConstraintPtr>::iterator cit = gubList0_.begin();
               cit != gubList0_.end(); ++cit) {
            if (c == *cit) {
              isFound = true;
              break;      
            }    
          }
        }

        if (!isFound) {
          addImplications_(c, 0);
        }
      }
      // Case: binary var when fixed to 1
      if (impli1_.size() != 0) {
        isFound = false;
        // Consider constraint which is not in gubList1_
        for (std::forward_list<ConstraintPtr>::iterator cit = gubList1_.begin();
             cit != gubList1_.end(); ++cit) {
          if (c == *cit) {
            isFound = true;
            break;      
          }    
        }
        if (!isFound) {
          addImplications_(c, 1);
        }
      }
    }
  }

  timer_->stop();
  return;
}

void PerspCon::delGUBList_()
{
  LinearFunctionPtr lf;
  ConstraintPtr c;
  VariablePtr v, v1;
  std::forward_list<impliVar>::iterator it1;
  std::unordered_map<VariablePtr, std::forward_list<impliVar>>::iterator 
    it;
  std::forward_list<impliVar>::iterator prev;

  while (!gubList0_.empty()) {
    c = gubList0_.front();
    lf = c->getLinearFunction();
    for (VariableGroupConstIterator itvar=lf->termsBegin();
         itvar!=lf->termsEnd(); ++itvar) {
      v = itvar->first;
      it = impli0_.find(v);
      if (it != impli0_.end()) {
        prev =  (it->second).before_begin();
        for (it1 = (it->second).begin(); it1 != (it->second).end();) {
          v1 = (*it1).var;
          if (lf->hasVar(v1)) {
            it1 = (it->second).erase_after(prev);
          } else {
            prev = it1;
            ++it1;
          }
        }

        if (it->second.empty()) {
          impli0_.erase(it);
        }
      }
    }
    gubList0_.pop_front();
  }


  while (!gubList1_.empty()) {
    c = gubList1_.front();
    lf = c->getLinearFunction();

    for (VariableGroupConstIterator itvar=lf->termsBegin();
         itvar!=lf->termsEnd(); ++itvar) {
      v = itvar->first;
      it = impli1_.find(v);
      if (it != impli1_.end()) {
        prev =  (it->second).before_begin();
        for (it1 = (it->second).begin(); it1 != (it->second).end();) {
          v1 = (*it1).var;
          if (lf->hasVar(v1)) {
            it1 = (it->second).erase_after(prev);
          } else {
            prev = it1;
            ++it1;
          }
        }
        if (it->second.empty()) {
          impli1_.erase(it);
        }
      }
    }
    gubList1_.pop_front();
  }
}


void PerspCon::addImplications_(ConstraintPtr c, bool z)
{
  bool found;
  impliVar impli;
  VariablePtr var;
  LinearFunctionPtr lf;
  UInt listSize, count;
  double ub, val, vc;
  impli.fixedVal.push_back(-INFINITY);
  std::forward_list<impliVar>::iterator it1;
  std::unordered_map<VariablePtr, std::forward_list<impliVar>>::iterator 
    it, itEnd;
  impli.fixedVal.push_back(-INFINITY);

  if (z) {
    it = impli1_.begin();
    itEnd = impli1_.end(); 
  } else {
    it = impli0_.begin();
    itEnd = impli0_.end();
  }

  ub = c->getUb();
    
  lf = c->getLinearFunction();
  for (; it != itEnd; ) {
    it1 = (it->second).begin();
    listSize = std::distance(it1, (it->second).end());
    if (lf->getNumTerms() <= (2 + listSize)) {
      val = 0;
      count = 0;
      for (VariableGroupConstIterator itvar=lf->termsBegin();
           itvar!=lf->termsEnd(); ++itvar) {
        var = itvar->first;
        if (var == it->first) {
          if (z) {
            val = val + itvar->second;
          }
          continue;
        }

        found = false;
        for (it1 = (it->second).begin(); it1 != (it->second).end(); ++it1) {
          if (var == (*it1).var) {
            found = true;
            val = val + (itvar->second)*((*it1).fixedVal[0]);
            break;
          }
        }
        if (!found) {
          ++count;
          if (count > 1) {
            break;
          }
          impli.var = var;
          vc = itvar->second;
        }
      }
      if (count == 1 && (fabs(vc) >= absTol_)) {
        val = (ub-val)/vc;
        found = true;
        if ((val <= impli.var->getLb() - absTol_) 
            || (val >= impli.var->getUb() + absTol_)) {
          found = false;
        }
        if (isBinary_(impli.var)) {
          if ((fabs(val) >= absTol_) && (fabs(val) <= 1-absTol_)) {
            found = false;            
          }          
        }
        
        if (!found) {
          if (z == 0) {
            // Infeasibility with var = 0. Fixing this variable to 1.
            (it->first)->setLb_(1);           
            impli1_.erase(it->first);
            it = impli0_.erase(it);
            if (impli0_.empty()) {
              break;
            }
          } else {
            // Infeasibility with var = 1. Fixing this variable to 0.
            (it->first)->setUb_(0);           
            impli0_.erase(it->first);
            it = impli1_.erase(it);
            if (impli1_.empty()) {
              break;
            }             
          }
        } else {
          impli.fixedVal[0] = val;
          (it->second).push_front(impli);          
          ++it;
        }
      } else {
        ++it;        
      }
    } else {
      ++it;      
    }
  }
}


void PerspCon::findPRCons()
{
  /// To determine all the variables fixed by binary variables
  //Perform this step if there are linear constraints and binary variables 
  implications_(); 

  //// This to check only nonlinear constraints one by one
  for (ConstraintConstIterator it=p_->consBegin(); it!=p_->consEnd(); ++it) {
    cons_ = *it;
    switch(cons_->getFunctionType()) {
    case Linear:
      break;
    case Nonlinear:
    case Quadratic:
    case Bilinear:
    case Polynomial:
    case Multilinear:
      evalConstraint_();
      break;
    default:
      break;
    }
  }
 
  //#if SPEW 
  displayInfo_();
  exit(1);
  //#endif  

  // Delete implications derived from constraints in gubList0_ and gubList1_
  delGUBList_();

  cons_ = 0;
  bVar_ = 0;
  return;
}


bool PerspCon::getStatus_()
{
  if(prConsVec_.size() > 0){
    return true; 
  } else {
    return false;
  }
}


// Local Variables: 
// mode: c++ 
// eval: (c-set-style "k&r") 
// eval: (c-set-offset 'innamespace 0) 
// eval: (setq c-basic-offset 2) 
// eval: (setq fill-column 78) 
// eval: (auto-fill-mode 1) 
// eval: (setq column-number-mode 1) 
// eval: (setq indent-tabs-mode nil) 
// End:
