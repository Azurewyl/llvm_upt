//===- UPTCallingConv.td - Calling Conventions for UPT -*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for UPT architecture.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// UPT Return Value Calling Convention
//===----------------------------------------------------------------------===//
def RetCC_UPT : CallingConv<[
  // i32 are returned in registers
  CCIfType<[i32], CCAssignToReg<[V0]>>,
  // Integer values get stored in stack slots that are 4 bytes in
  // size and 4-byte aligned.
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

//===----------------------------------------------------------------------===//
// UPT Argument Calling Conventions
//===----------------------------------------------------------------------===//
def CC_UPT : CallingConv<[
  // Promote i8/i16 arguments to i32.
  CCIfType<[i1, i8, i16], CCPromoteToType<i32>>,
  // The first 4 integer arguments are passed in integer registers.
  CCIfType<[i32], CCAssignToReg<[A0,A1,A2,A3]>>,
  // Integer values get stored in stack slots that are 4 bytes in
  // size and 4-byte aligned.
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

def CC_Save : CalleeSavedRegs<(add S0,S1,S2,S3,RA)>;

