//===-- RISCVInstrInfoM.td - UPT 'M' instructions -------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the UPT instructions from the standard 'M', Integer
// Multiplication and Division instruction set extension.
//
//===----------------------------------------------------------------------===//

class MulDivInst<bits<6> op, string asmstr, SDNode OpNode>:
    UPTInstFormReg3<op,
    (outs GPR:$rd), (ins GPR:$rs, GPR:$rt),
    !strconcat(asmstr, "\t$rd, $rs, $rt"),
    [(set GPR:$rd, (OpNode GPR:$rs, GPR:$rt))]>{
  let Predicates =[HasStdExtM];
}

//===----------------------------------------------------------------------===//
// Mul/Div Instructions
//===----------------------------------------------------------------------===//

def MUL     : MulDivInst<0x30, "MUL",mul>;
def MULH    : MulDivInst<0x31, "MULHS",mulhs>;
def MULHU   : MulDivInst<0x32, "MULHU" , mulhu>;
def DIV     : MulDivInst<0x33, "DIV", sdiv>;
def DIVU    : MulDivInst<0x34, "DIVU",udiv>;
def REM     : MulDivInst<0x35, "REM",srem>;
def REMU    : MulDivInst<0x36, "REMU",urem>;



