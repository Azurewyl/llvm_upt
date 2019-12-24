//===-- UPTTargetMachine.h - Define TargetMachine for UPT ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the UPT specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LIB_TARGET_UPT_UPTTARGETMACHINE_H
#define LIB_TARGET_UPT_UPTTARGETMACHINE_H
#include "UPT.h"
#include "UPTSubtarget.h"

namespace llvm {
// All target-specific information should be accessible through this interface.
class UPTTargetMachine : public LLVMTargetMachine {
  UPTSubtarget Subtarget;
  std::unique_ptr<TargetLoweringObjectFile> TLOF;

public:

  UPTTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                   StringRef FS, const TargetOptions &Options,
                   Optional<Reloc::Model> RM,
                   Optional<CodeModel::Model> CM, CodeGenOpt::Level OL, bool JIT = false);

  // returns a reference to that target's SubtargetInfo-derived member variable
  const UPTSubtarget *getSubtargetImpl(const Function &F) const override {
    return &Subtarget;
  }
  const UPTSubtarget *getSubtargetImpl() const { return &Subtarget; }

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

} // end namespace llvm

#endif

