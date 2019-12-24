//===-- UPT.h - Top-level interface for UPT representation --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// UPT back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TARGET_UPT_H
#define TARGET_UPT_H

#include "MCTargetDesc/UPTMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class TargetMachine;
class UPTTargetMachine;

FunctionPass *createUPTISelDag(UPTTargetMachine &TM,CodeGenOpt::Level OptLevel);
} // end namespace llvm;

#endif
