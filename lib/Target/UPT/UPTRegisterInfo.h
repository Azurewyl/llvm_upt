//===-- UPTRegisterInfo.h - UPT Register Information Impl ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the UPT implementation of the MRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LIB_TARGET_UPT_UPTREGISTERINFO_H
#define LIB_TARGET_UPT_UPTREGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "UPTGenRegisterInfo.inc"

namespace llvm {

class TargetInstrInfo;

struct UPTRegisterInfo : public UPTGenRegisterInfo {
public:
  UPTRegisterInfo();

  /// Code Generation virtual methods...
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  const uint32_t *getCallPreservedMask(const MachineFunction &MF, CallingConv::ID) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  bool isConstantPhysReg(unsigned PhysReg) const override;
  bool requiresRegisterScavenging(const MachineFunction &MF) const override;

  bool trackLivenessAfterRegAlloc(const MachineFunction &MF) const override;

  bool useFPForScavengingIndex(const MachineFunction &MF) const override;

  void eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum, RegScavenger *RS) const override;

  // Debug information queries.
  Register getFrameRegister(const MachineFunction &MF) const override;
};

} // end namespace llvm

#endif
