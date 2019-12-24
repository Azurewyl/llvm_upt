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
    (outs GRRegs:$rd), (ins GRRegs:$rs, GRRegs:$rt),
    !strconcat(asmstr, "\t$rd, $rs, $rt"),
    [(set GRRegs:$rd, (OpNode GRRegs:$rs, GRRegs:$rt))]>{
  let Predicates =[HasStdExtM];
}

//===----------------------------------------------------------------------===//
// Mul/Div Instructions
//===----------------------------------------------------------------------===//

def MUL     : MulDivInst<0b010101, "MUL",mul>;
def MULH    : MulDivInst<0b010111, "MULHS",mulhs>;
def MULHU   : MulDivInst<0b011001, "MULHU" , mulhu>;
def DIV     : MulDivInst<0b011010, "DIV", sdiv>;
def DIVU    : MulDivInst<0b011011, "DIVU",udiv>;
def REM     : MulDivInst<0b011100, "REM",srem>;
def REMU    : MulDivInst<0b011101, "REMU",urem>;



