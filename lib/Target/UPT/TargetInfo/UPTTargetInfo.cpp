//===-- UPTTargetInfo.cpp - UPT Target Implementation -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "UPTTargetInfo.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheUPTTarget() {
  static Target TheUPTTarget;
  return TheUPTTarget;
}

extern "C" void LLVMInitializeUPTTargetInfo() {
  RegisterTarget<Triple::upt> X(getTheUPTTarget(), "upt", "UPT, BUPT 32bit Cpu", "UPT");
}
