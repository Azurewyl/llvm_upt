//===-- UPTRegisterInfo.cpp - UPT Register Information ----------------===//
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

#define DEBUG_TYPE "register info"
#include "UPT.h"
#include "UPTRegisterInfo.h"
#include "UPTSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

#define GET_REGINFO_TARGET_DESC
#include "UPTGenRegisterInfo.inc"

using namespace llvm;

UPTRegisterInfo::UPTRegisterInfo() : UPTGenRegisterInfo(UPT::RA) {}

/// Return a null-terminated list of all of the callee-saved registers on
/// this target. The register should be in the order of desired callee-save
/// stack frame offset. The first register is closest to the incoming stack
/// pointer if stack grows down, and vice versa.
/// Notice: This function does not take into account disabled CSRs.
const MCPhysReg *UPTRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CC_Save_SaveList; //from tablegen
}

/// Returns a bitset indexed by physical register number indicating if a
/// register is a special register that has particular uses and should be
/// considered unavailable at all times, e.g. stack pointer, return address.
/// A reserved register:
/// - is not allocatable
/// - is considered always live
/// - is ignored by liveness tracking
BitVector UPTRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  Reserved.set(UPT::SP);
  Reserved.set(UPT::RA);
  Reserved.set(UPT::ZERO);
  Reserved.set(UPT::SR);
  return Reserved;
}

/// Returns true if PhysReg is unallocatable and constant throughout the
/// function.  Used by MachineRegisterInfo::isConstantPhysReg()
bool UPTRegisterInfo::isConstantPhysReg(unsigned PhysReg) const {
  return PhysReg == UPT::ZERO;
}

/// Return a mask of call-preserved registers for the given calling convention
/// on the current function. The mask should include all call-preserved
/// aliases. This is used by the register allocator to determine which
/// registers can be live across a call.
const uint32_t *UPTRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                                      CallingConv::ID) const {
  return CC_Save_RegMask;
}

/// Returns true if the target requires (and can make use of) the register scavenger.
bool UPTRegisterInfo::requiresRegisterScavenging(const MachineFunction &MF) const {
  return true;
}

/// Returns true if the live-ins should be tracked after register allocation.
bool UPTRegisterInfo::trackLivenessAfterRegAlloc(const MachineFunction &MF) const {
  return true;
}

// Returns true if the target wants to use "frame pointer based accesses" to
// spill to the scavenger emergency spill slot.
bool UPTRegisterInfo::useFPForScavengingIndex(const MachineFunction &MF) const {
  return false;
}

/// Eliminate abstract frame indices(MO_FrameIndex) from instructions which may use them.
/// SPAdj is the SP adjustment due to call frame setup instruction.
/// FIOperandNum is the FI operand number.
//  What do here:LDR　%RA,<fi#0> ,0 -> LDR %RA,%SP,imm ,STR
void UPTRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                          int SPAdj, unsigned FIOperandNum,
                                          RegScavenger *RS) const {
  LLVM_DEBUG(dbgs() << ">> RegisterInfo::eliminateFrameIndex <<\n";);

  // get the frame index
  MachineInstr &MI = *II;
  const MachineFunction &MF = *MI.getParent()->getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  MachineOperand &FIOp = MI.getOperand(FIOperandNum);
  int FI = FIOp.getIndex();

  // Determine if we can eliminate the index from this kind of instruction.
  unsigned ImmOpIdx = 0;
  switch (MI.getOpcode()) {
  default:return;
  case UPT::LDR:
  case UPT::STR:
    ImmOpIdx = FIOperandNum + 1;
    break;
  }

  // FIXME: check the size of offset.
  MachineOperand &ImmOp = MI.getOperand(ImmOpIdx);
  int Offset = MFI.getObjectOffset(FI) + MFI.getStackSize() + ImmOp.getImm();
  FIOp.ChangeToRegister(UPT::SP, false);
  ImmOp.ChangeToImmediate(Offset);
}

/// return the register used as a base for values allocated in the current stack frame.
Register UPTRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return UPT::SP;
}
