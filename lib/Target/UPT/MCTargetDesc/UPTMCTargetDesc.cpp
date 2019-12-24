//===-- UPTMCTargetDesc.cpp - UPT Target Descriptions -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides UPT specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "UPTMCTargetDesc.h"
#include "InstPrinter/UPTInstPrinter.h"
#include "UPTMCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
#include "TargetInfo/UPTTargetInfo.h"

#define GET_INSTRINFO_MC_DESC
#include "UPTGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "UPTGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "UPTGenRegisterInfo.inc"

using namespace llvm;


static MCInstrInfo *createUPTMCInstrInfo() {
  auto *X = new MCInstrInfo();
  InitUPTMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createUPTMCRegisterInfo(const Triple &TT) {
  auto *X = new MCRegisterInfo();
  InitUPTMCRegisterInfo(X, UPT::RA);  //initialize pc
  return X;
}

static MCSubtargetInfo *createUPTMCSubtargetInfo(const Triple &TT,
                                                 StringRef CPU,
                                                 StringRef FS) {
  return createUPTMCSubtargetInfoImpl(TT, CPU, FS);
}

static MCAsmInfo *createUPTMCAsmInfo(const MCRegisterInfo &MRI,
                                     const Triple &TT) {
  return new UPTMCAsmInfo(TT);
}


static MCInstPrinter *
createUPTMCInstPrinter(const Triple &TT, unsigned SyntaxVariant,
                       const MCAsmInfo &MAI, const MCInstrInfo &MII,
                       const MCRegisterInfo &MRI) {
  return new UPTInstPrinter(MAI, MII, MRI);
}


// Force static initialization.
extern "C" void LLVMInitializeUPTTargetMC() {
  for (Target *T : {&getTheUPTTarget(), &getTheUPTTarget()}) {
    // Register the MC asm info.
    TargetRegistry::RegisterMCAsmInfo(*T, createUPTMCAsmInfo);

    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createUPTMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createUPTMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T, createUPTMCSubtargetInfo);

    // Register the MCInstPrinter
    TargetRegistry::RegisterMCInstPrinter(*T, createUPTMCInstPrinter);

    // Register the ASM Backend.
    TargetRegistry::RegisterMCAsmBackend(*T, createUPTAsmBackend);

    // Register the MCCodeEmitter
    TargetRegistry::RegisterMCCodeEmitter(*T, createUPTMCCodeEmitter);
  }
}
