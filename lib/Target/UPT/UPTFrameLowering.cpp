//===-- UPTFrameLowering.cpp - Frame info for UPT Target --------------===//
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

#define  DEBUG_TYPE "UPT frame lowering"
#define STACK_DYNAMIC
#include "UPTFrameLowering.h"
#include "UPT.h"
#include "UPTInstrInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Target/TargetOptions.h"
#include <algorithm> // std::sort

using namespace llvm;

//===----------------------------------------------------------------------===//
// UPTFrameLowering:
//===----------------------------------------------------------------------===//


/// hasFP - Return true if the specified function should have a dedicated
/// frame pointer register. For most targets this is true only if the function
/// has variable sized allocas or if frame pointer elimination is disabled.
bool UPTFrameLowering::hasFP(const MachineFunction &MF) const {
  return true;
}

// Fix stacksize if has alignment
uint64_t UPTFrameLowering::computeStackSize(MachineFunction &MF) const {
  MachineFrameInfo MFI = MF.getFrameInfo();
  uint64_t StackSize = MFI.getStackSize();
  unsigned StackAlign = getStackAlignment();
  if (StackAlign > 0) {
    StackSize = alignTo(StackSize, StackAlign);;
  }
  return StackSize;
}

// Materialize an offset for a ADD/SUB stack operation(specifically,ADDI/SUBI)
// Return zero if the offset fits into the instruction as an immediate(16bit),
// or the number of the register where the offset is materialized.
static unsigned materializeOffset(MachineFunction &MF, MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator MBBI,
                                  unsigned Offset) {
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  const uint64_t MaxSubImm = 0xffff;
  if (Offset <= MaxSubImm) {
    // The stack offset fits in the ADD/SUB instruction.
    return 0;
  } else {
    // The stack offset does not fit in the ADD/SUB instruction.
    // Materialize the offset using MOVLO/MOVHI.
    unsigned OffsetReg = UPT::T0;
    unsigned OffsetLo = (unsigned)(Offset & 0xffff);
    unsigned OffsetHi = (unsigned)((Offset & 0xffff0000) >> 16);
    BuildMI(MBB, MBBI, dl, TII.get(UPT::MOVLOi16), OffsetReg)
        .addImm(OffsetLo)
        .setMIFlag(MachineInstr::FrameSetup);
    if (OffsetHi) {
      BuildMI(MBB, MBBI, dl, TII.get(UPT::MOVHIi16), OffsetReg)
          .addReg(OffsetReg)
          .addImm(OffsetHi)
          .setMIFlag(MachineInstr::FrameSetup);
    }
    return OffsetReg;
  }
}

//===----------------------------------------------------------------------===//
// Function Prologue and Epilogue
// Functions need a prologue and an epilogue to be complete.
// The former sets up the stack frame and chanshallee-saved registers during the beginning of a function, whereas
// The latter cleans up the stack frame prior to function return
//===----------------------------------------------------------------------===//


// Fixed sized stack or dynamic sized

//#define STACK_DYNAMIC true
void UPTFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  LLVM_DEBUG(dbgs() << ">> FrameLowering::emitPrologue <<\n");

  // Compute the stack size, to determine if we need a prologue at all.
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

#ifdef STACK_DYNAMIC
  uint64_t StackSize = computeStackSize(MF);
  if (!StackSize) {
    return;
  }
  // Adjust the stack pointer.
  unsigned StackReg = UPT::SP;
  unsigned OffsetReg = materializeOffset(MF, MBB, MBBI, (unsigned)StackSize);
  if (OffsetReg) {
    BuildMI(MBB, MBBI, dl, TII.get(UPT::SUBR), StackReg)
        .addReg(StackReg)
        .addReg(OffsetReg)
        .setMIFlag(MachineInstr::FrameSetup);
  } else {
    BuildMI(MBB, MBBI, dl, TII.get(UPT::SUBI), StackReg)
        .addReg(StackReg)
        .addImm(StackSize)
        .setMIFlag(MachineInstr::FrameSetup);
  }
#else
  // allocate fixed size for simplicity
  uint64_t StackSize = 4 * 16;

  MF.getFrameInfo().setStackSize(StackSize);
  unsigned StackReg = UPT::SP;
  BuildMI(MBB, MBBI, dl, TII.get(UPT::SUBI), StackReg)
      .addReg(UPT::SP)
      .addImm(StackSize)
      .setMIFlag(MachineInstr::FrameSetup);
#endif

}

void UPTFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  LLVM_DEBUG(dbgs() << ">> FrameLowering::emitEpilogue <<\n");

  // Compute the stack size, to determine if we need an epilogue at all.
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  DebugLoc dl = MBBI->getDebugLoc();

  uint64_t StackSize = computeStackSize(MF);
  if (!StackSize) {
    return;
  }

  // Restore the stack pointer.
  unsigned StackReg = UPT::SP;
  unsigned OffsetReg = materializeOffset(MF, MBB, MBBI, (unsigned)StackSize);
  if (OffsetReg) {
    BuildMI(MBB, MBBI, dl, TII.get(UPT::ADDR), StackReg)
        .addReg(StackReg)
        .addReg(OffsetReg)
        .setMIFlag(MachineInstr::FrameSetup);
  } else {
    BuildMI(MBB, MBBI, dl, TII.get(UPT::ADDI), StackReg)
        .addReg(StackReg)
        .addImm(StackSize)
        .setMIFlag(MachineInstr::FrameSetup);
  }
}

// This function eliminates ADJCALLSTACKDOWN/ADJCALLSTACKUP pseudo instructions
MachineBasicBlock::iterator UPTFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  if (I->getOpcode() == UPT::ADJCALLSTACKUP ||
      I->getOpcode() == UPT::ADJCALLSTACKDOWN) {
    return MBB.erase(I);
  }

}
