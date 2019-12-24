//===-- UPTSubtarget.cpp - UPT Subtarget Information ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the UPT specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "UPTSubtarget.h"
#include "UPT.h"
#include "llvm/Support/TargetRegistry.h"

#define DEBUG_TYPE "upt-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "UPTGenSubtargetInfo.inc"

using namespace llvm;

void UPTSubtarget::anchor() {}

UPTSubtarget &UPTSubtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS) {
  // Determine default and user-specified characteristics
  std::string CPUName = CPU;
  if (CPUName.empty())
    CPUName = "generic-upt";
  ParseSubtargetFeatures(CPUName, FS);
  return *this;
}
UPTSubtarget::UPTSubtarget(const Triple &TT, StringRef CPU, StringRef FS, UPTTargetMachine &TM)
    : UPTGenSubtargetInfo(TT, CPU, FS), InstrInfo(), TLInfo(TM),
      FrameLowering(initializeSubtargetDependencies(CPU, FS)), RegInfo() {
}
