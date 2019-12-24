//===-- UPTInstrInfo.h - UPT Instruction Information --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the UPT implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef UPTINSTRUCTIONINFO_H
#define UPTINSTRUCTIONINFO_H

#include "UPTRegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "UPTGenInstrInfo.inc"

namespace llvm {

class UPTInstrInfo : public UPTGenInstrInfo {
  const UPTRegisterInfo RI;
  virtual void anchor();

public:
  UPTInstrInfo();

  /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  /// such, whenever a client has an instance of instruction info, it should
  /// always be able to get register info as well (through this method).
  ///
  const UPTRegisterInfo &getRegisterInfo() const { return RI; }
  unsigned int isLoadFromStackSlot(const MachineInstr &MI, int &FrameIndex) const override;
  unsigned int isStoreToStackSlot(const MachineInstr &MI, int &FrameIndex) const override;
  bool analyzeBranch(MachineBasicBlock &MBB,
                     MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify) const override;
  unsigned int removeBranch(MachineBasicBlock &MBB, int *BytesRemoved) const override;
  unsigned int insertBranch(MachineBasicBlock &MBB,
                            MachineBasicBlock *TBB,
                            MachineBasicBlock *FBB,
                            ArrayRef<MachineOperand> Cond,
                            const DebugLoc &DL,
                            int *BytesAdded) const override;
  void copyPhysReg(MachineBasicBlock &MBB,
                   MachineBasicBlock::iterator MI,
                   const DebugLoc &DL,
                   unsigned DestReg,
                   unsigned SrcReg,
                   bool KillSrc) const override;
  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI,
                           unsigned SrcReg,
                           bool isKill,
                           int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI) const override;
  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI,
                            unsigned DestReg,
                            int FrameIndex,
                            const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI) const override;
  bool isAsCheapAsAMove(const MachineInstr &MI) const override;
  bool expandPostRAPseudo(MachineInstr &MI) const override;
  bool verifyInstruction(const MachineInstr &MI, StringRef &ErrInfo) const override;
};
}

#endif
