//===-- UPTMCTargetDesc.h - UPT Target Descriptions ---------*- C++ -*-===//
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

#ifndef UPTMCTARGETDESC_H
#define UPTMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"
#include <memory>

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;
class Triple;
class StringRef;
class raw_pwrite_stream;


MCCodeEmitter *createUPTMCCodeEmitter(const MCInstrInfo &MCII,
                                      const MCRegisterInfo &MRI,
                                      MCContext &Ctx);

MCAsmBackend *createUPTAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                    const MCRegisterInfo &MRI,
                                    const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter> createUPTELFObjectWriter(uint8_t OSABI);

} // End llvm namespace

// Defines symbolic names for UPT registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "UPTGenRegisterInfo.inc"

// Defines symbolic names for the UPT instructions.
#define GET_INSTRINFO_ENUM
#include "UPTGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "UPTGenSubtargetInfo.inc"

#endif
