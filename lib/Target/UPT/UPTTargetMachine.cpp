//===-- UPTTargetMachine.cpp - Define TargetMachine for UPT -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// Implements the info about UPT target spec.
// This file contains the entry points for global functions defined in
// the LLVM UPT back-end.
//===----------------------------------------------------------------------===//
#include "UPT.h"
#include "UPTTargetMachine.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Support/TargetRegistry.h"
#include "TargetInfo/UPTTargetInfo.h"

using namespace llvm;

static llvm::StringRef computeDataLayout(const Triple &TT) {
  // see http://llvm.org/docs/LangRef.html#data-layout
  // It is kept here only to avoid reparsing the string ,
  // but should not really be used during compilation
  llvm::StringRef DataLayout = "e-m:e-p:32:32-i1:8:32-i8:8:32-i16:16:32-i64:32-f64:32-a:0:32-n32";
  //   "e";  Little-endian
  //   "-m:e";  ELF style name mangling
  //   "-p:32:32";  Set 32-bit pointer size with 32-bit alignment
  //   "-i1:8:32";  Align i1 to a 32-bit word
  //   "-i8:8:32";  Align i8 to a 32-bit word
  //   "-i16:16:32";  Align i16 to a 32-bit word
  //   "-i64:32";  Align i64 to a 32-bit word
  //   "-f64:32";  Align f64 to a 32-bit word
  //   "-a:0:32"; Align aggregates to a 32-bit word
  //   "-n32"; Set native integer width to 32-bits
  return DataLayout;
}

static Reloc::Model getEffectiveRelocModel(Optional<Reloc::Model> RM) {
  if (!RM.hasValue()) {
    return Reloc::Static;
  }
  return *RM;
}


// Target ,Triple string, CPU name, and target feature strings,
// code generation relocation model.(Choices:static,PIC,ynamic-no-pic,target default)
// Code model(choices:small, kernel, medium, large)
// optimization level: None, Less, Default, or Aggressive.
UPTTargetMachine::UPTTargetMachine(const Target &T, const Triple &TT,
                                   StringRef CPU, StringRef FS,
                                   const TargetOptions &Options,
                                   Optional<Reloc::Model> RM,
                                   Optional<CodeModel::Model> CM,
                                   CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(TT), TT, CPU, FS,
                        Options,
                        getEffectiveRelocModel(RM),
                        getEffectiveCodeModel(CM,CodeModel::Small), OL),
      Subtarget(TT, CPU, FS, *this),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()) {
  initAsmInfo();
}

namespace {
/// UPT Code Generator Pass Configuration Options.
class UPTPassConfig : public TargetPassConfig {
public:
  UPTPassConfig(UPTTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  UPTTargetMachine &getUPTTargetMachine() const {
    return getTM<UPTTargetMachine>();
  }

  bool addPreISel() override {return false;};
  bool addInstSelector() override {
    addPass(createUPTISelDag(getUPTTargetMachine(),getOptLevel()));
    return false;
  };
  void addPreEmitPass() override { };
};
} // namespace

TargetPassConfig *UPTTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new UPTPassConfig(*this, PM);
}

// Force static initialization.
extern "C" void LLVMInitializeUPTTarget() {
  RegisterTargetMachine<UPTTargetMachine> X(getTheUPTTarget());
}



