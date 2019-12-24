//===-- UPTFrameLowering.h - Frame info for UPT Target ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains UPT frame information that doesn't fit anywhere else
// cleanly...
//
//===----------------------------------------------------------------------===//

#ifndef UPTFRAMEINFO_H
#define UPTFRAMEINFO_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {
class UPTSubtarget;

class UPTFrameLowering : public TargetFrameLowering {
public:
  explicit UPTFrameLowering(const UPTSubtarget &STI)
      : TargetFrameLowering(StackGrowsDown,
      /*StackAlignment=*/Align(4),
      /*LocalAreaOffset=*/0),
        STI(STI) {}

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into the function.
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override;

  bool hasFP(const MachineFunction &MF) const override;

  //! Stack slot size (4 bytes)
  static int stackSlotSize() { return 4; }

protected:
  const UPTSubtarget &STI;

private:
  uint64_t computeStackSize(MachineFunction &MF) const;

};
}

#endif // UPTFRAMEINFO_H

