CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/src/base/MinotaurConfig.h.cmake"
  "${PROJECT_BINARY_DIR}/src/base/MinotaurConfig.h"
  )

configure_file (
  "${PROJECT_SOURCE_DIR}/src/base/Version.h.cmake"
  "${PROJECT_BINARY_DIR}/src/base/Version.h"
  )

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}/src/base")
 
set (MINOTAUR_SOURCES
     AnalyticalCenter.cpp
     BndProcessor.cpp 
     Branch.cpp 
     BranchAndBound.cpp 
     Brancher.cpp 
     BrCand.cpp 
     BrVarCand.cpp 
     Chol.cpp
     CGraph.cpp
     CNode.cpp
     ConBoundMod.cpp
     Constraint.cpp
     CoverCutGenerator.cpp 
     Cut.cpp
     CutInfo.cpp
     CutMan1.cpp
     CutMan2.cpp
     CxQuadHandler.cpp 
     CxUnivarHandler.cpp
     Eigen.cpp 
     Engine.cpp 
     Environment.cpp 
     FeasibilityPump.cpp 
     Function.cpp 
     HessianOfLag.cpp 
     IntVarHandler.cpp 
     Jacobian.cpp
     KnapsackList.cpp 
     KnapCovHandler.cpp 
     LGCIGenerator.cpp 
     LPRelaxation.cpp 
     LexicoBrancher.cpp 
     LinBil.cpp 
     LinConMod.cpp 
     LinMods.cpp 
     LinearCut.cpp 
     LinearFunction.cpp 
     LinearHandler.cpp
     LinFeasPump.cpp 
     Linearizations.cpp
     Logger.cpp 
     MaxFreqBrancher.cpp
     MaxVioBrancher.cpp
     MINLPDiving.cpp
     #MultiSolHeur.cpp
     MsProcessor.cpp	
     MultilinearTermsHandler.cpp
     NLPRelaxation.cpp 
     NlPresHandler.cpp
     NLPMultiStart.cpp
     NlWriter.cpp
     Node.cpp 
     NodeFullRelaxer.cpp
     NodeHeap.cpp 
     NodeIncRelaxer.cpp 
     NodeProcessor.cpp 
     NodeStack.cpp 
     NonlinearFunction.cpp 
     OAHandler.cpp
     Objective.cpp 
     Operations.cpp 
     Option.cpp 
     ParBndProcessor.cpp
     ParBranchAndBound.cpp
     ParCutMan.cpp
     ParMINLPDiving.cpp
     ParNodeIncRelaxer.cpp
     ParQGBranchAndBound.cpp
     ParQGHandler.cpp
     ParQGHandlerAdvance.cpp
     ParPCBProcessor.cpp
     ParReliabilityBrancher.cpp
     ParTreeManager.cpp
     PCBProcessor.cpp 
     PerspCon.cpp
     PerspCutGenerator.cpp 
     #PerspCutHandler.cpp 
     PolynomialFunction.cpp 
     PreAuxVars.cpp
     PreDelVars.cpp
     PreSubstVars.cpp
     Presolver.cpp 
     Problem.cpp
     ProbStructure.cpp 
     #QGAdvHandler.cpp 
     QGHandler.cpp 
     QGHandlerAdvance.cpp 
     #QGHandlerPDE.cpp 
     QPDRelaxer.cpp 
     QPDProcessor.cpp 
     QuadHandler.cpp 
     QuadraticFunction.cpp 
     RandomBrancher.cpp
     RCHandler.cpp
     Relaxation.cpp 
     ReliabilityBrancher.cpp 
     SecantMod.cpp 
     SimpleCutMan.cpp 
     SimpleTransformer.cpp 
     Solution.cpp 
     SolutionPool.cpp 
     SOS.cpp 
     SOS1Handler.cpp
     SOS2Handler.cpp
     SOSBrCand.cpp
     STOAHandler.cpp
     Transformer.cpp 
     TransPoly.cpp 
     TransSep.cpp
     TreeManager.cpp 
     Types.cpp 
     UnambRelBrancher.cpp
     VarBoundMod.cpp 
     Variable.cpp 
     YEqCGs.cpp
     YEqLFs.cpp
     YEqMonomial.cpp
     YEqUCGs.cpp
     YEqVars.cpp
     YEqQfBil.cpp
)

set (MINOTAUR_HEADERS
     MinotaurDeconfig.h
     ActiveNodeStore.h
     AnalyticalCenter.cpp
     BndProcessor.h
     Branch.h
     Brancher.h
     BranchAndBound.h
     BrCand.h
     BrVarCand.h
     CGraph.h
     CNode.h
     ConBoundMod.h
     Constraint.h
     CoverCutGenerator.h # Serdar
     CutInfo.h
     CutManager.h
     CxQuadHandler.h 
     CxUnivarHandler.h
     Eigen.h
     Engine.h
     Environment.h
     FeasibilityPump.h 
     Exception.h
     Function.h
     Handler.h
     HessianOfLag.h
     Heuristic.h
     Iterate.h
     IntVarHandler.h
     Jacobian.h
     KnapsackList.h # Serdar
     KnapCovHandler.h # Serdar
     LexicoBrancher.h
     LinearCut.h
     LinearFunction.h
     LinearHandler.h
     LinFeasPump.h 
     Linearizations.h
     LinBil.h
     LinConMod.h
     LinMods.h
     LGCIGenerator.h # Serdar
     Logger.h
     LPEngine.h
     LPRelaxation.h
     MaxFreqBrancher.h
     MaxVioBrancher.h
     MILPEngine.h
     MINLPDiving.h
     #MultiSolHeur.h
     Modification.h
     MsProcessor.h
     MultilinearTermsHandler.h
     NLPEngine.h
     NLPRelaxation.h
     NlPresHandler.h
     NLPMultiStart.h
     NlWriter.h
     Node.h
     NodeHeap.h
     NodeRelaxer.h
     NodeIncRelaxer.h
     NodeProcessor.h
     NodeStack.h
     NonlinearFunction.h
     OAHandler.h
     Objective.h
     Operations.h
     Option.h
     OpCode.h
     ParBndProcessor.h
     ParBranchAndBound.h
     ParCutMan.h
     ParMINLPDiving.h
     ParNodeIncRelaxer.h
     ParQGBranchAndBound.h
     ParQGHandler.h
     ParQGHandlerAdvance.h
     ParPCBProcessor.h
     ParReliabilityBrancher.h
     ParTreeManager.h
     PCBProcessor.h
     PerspCon.h
     PerspCutGenerator.h 
     #PerspCutHandler.h
     PolynomialFunction.h
     PreAuxVars.h
     PreDelVars.h
     PreMod.h
     Presolver.h
     PreSubstVars.h
     Problem.h
     ProblemSize.h
     ProbStructure.h # Serdar
     QPEngine.h
     QGHandler.h
     QGHandlerAdvance.h 
     #QGAdvHandler.h 
     #QGHandlerPDE.h
     QPDProcessor.h 
     QuadHandler.h 
     QPDRelaxer.h 
     QuadraticFunction.h
     RandomBrancher.h
     RCHandler.h
     Relaxation.h
     ReliabilityBrancher.h
     SecantMod.h
     SimpleCutMan.h 
     SimpleTransformer.h 
     Solution.h
     SolutionPool.h
     SOS.h
     SOS1Handler.h
     SOS2Handler.h
     SOSBrCand.h
     STOAHandler.h
     Timer.h
     Transformer.h 
     TransPoly.h 
     TransSep.h
     TreeManager.h
     Types.h
     UnambRelBrancher.h
     WarmStart.h
     Variable.h
     YEqCGs.h
     YEqLFs.h
     YEqMonomial.h
     YEqUCGs.h
     YEqVars.h
     YEqQfBil.h
     )


add_library(minotaur ${MINOTAUR_SOURCES})

if (BUILD_SHARED_LIBS)
  install(TARGETS minotaur 
          LIBRARY 
          DESTINATION lib)
else (BUILD_SHARED_LIBS)
  install(TARGETS minotaur 
          ARCHIVE 
          DESTINATION lib)
endif (BUILD_SHARED_LIBS)

install(FILES ${MINOTAUR_HEADERS} DESTINATION include/minotaur)
install(FILES ${PROJECT_BINARY_DIR}/src/base/MinotaurConfig.h DESTINATION include/minotaur)
install(FILES ${PROJECT_BINARY_DIR}/src/base/MinotaurCFortran.h DESTINATION include/minotaur)
install(FILES ${PROJECT_BINARY_DIR}/src/base/Version.h DESTINATION include/minotaur)

