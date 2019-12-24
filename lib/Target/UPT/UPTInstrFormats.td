//===-- UPTInstrFormats.td - UPT Instruction Formats ---*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  These instruction format definitions are structured to match the
//  description in the UPT User-Level ISA specification as closely as
//  possible.
//
//  One area of divergence is in the description of immediates. The
//  specification describes immediate encoding in terms of bit-slicing
//  operations on the logical value represented. The immediate argument to
//  these instruction formats instead represents the bit sequence that will be
//  inserted into the instruction. e.g. although JAL's immediate is logically
//  a 21-bit value (where the LSB is always zero), we describe it as an imm20
//  to match how it is encoded.
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// UPT Format Definitions.Use 3bit to rep 7 kinds of format
//===----------------------------------------------------------------------===//

class Format<bits<3> val> {
  bits<3> Value = val;
}

def FormPseudo: Format<0>;
def FormReg0  : Format<1>;
def FormReg1  : Format<2>;
def FormReg2  : Format<3>;
def FormReg3  : Format<4>;
def FormReg1I : Format<5>;
def FormReg2I : Format<6>;

//===----------------------------------------------------------------------===//
// Instruction format superclass： Define Opcode in base class because
// all instrutions have the same bit-size and bit-location for the Opcode
//===----------------------------------------------------------------------===//
class UPTInst<dag outs, dag ins, string asmstr, list<dag> pattern, Format f>
      : Instruction
{
  field bits<32> Inst;
  let Size = 4;

  Format Form = f;
  bits<3> FormBits = Form.Value;

  let Namespace = "UPT";
  let DecoderNamespace = "UPT";

  bits<6> Opcode = 0;
  let Inst{31-26} = Opcode;
  let OutOperandList = outs;
  let InOperandList  = ins;
  let AsmString   = asmstr;
  let Pattern     = pattern;
}

// pseudo class is CodenGen only, have blank asmstr
class UPTPseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
      : UPTInst<outs, ins, asmstr, pattern, FormPseudo>
{
  let isCodeGenOnly = 1;
  let isPseudo = 1;
}

// Format with 0 Register
class UPTInstFormReg0<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern>
      : UPTInst<outs, ins, asmstr, pattern, FormReg0>
{
  bits<26> addr;
  let Opcode = op;
  let Inst{25-0} = addr;
}

// Format with 1 register
class UPTInstFormReg1<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern>
      : UPTInst<outs, ins, asmstr, pattern, FormReg1>
{
  bits<5> rd;

  let Opcode = op;
  let Inst{25-21} = rd;
  let Inst{20-0} = 0; //reversed
}

// Format with 2 registers
class UPTInstFormReg2<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern>
      : UPTInst<outs, ins, asmstr, pattern, FormReg2>
{
  bits<5> rd;
  bits<5> rs;

  let Opcode = op;
  let Inst{25-21} = rd;
  let Inst{20-16} = rs;
  let Inst{15-0} = 0; //reversed
}

// Format with 3 register
class UPTInstFormReg3<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern>
      : UPTInst<outs, ins, asmstr, pattern, FormReg3>
{
  bits<5> rd;
  bits<5> rs;
  bits<5> rt;

  let Opcode = op;
  let Inst{25-21} = rd;
  let Inst{20-16} = rs;
  let Inst{15-11} = rt;
  let Inst{10-0} = 0; //reversed
}

// Format with 1 register && imm
class UPTInstFormReg1I<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern>
      : UPTInst<outs, ins, asmstr, pattern, FormReg1I>
{
  bits<5> rd;
  bits<20> imm;

  let Opcode = op;
  let Inst{25-21} = rd;
  let Inst{19-0} = imm;
}
// FormRegR2I ,2 register & imm
class UPTInstFormReg2I<bits<6> op, dag outs, dag ins, string asmstr, list<dag> pattern>
       : UPTInst<outs, ins, asmstr, pattern, FormReg2I>
 {
   bits<5> rd;
   bits<5> rs;
   bits<16> imm;

   let Opcode = op;
   let Inst{25-21} = rd;
   let Inst{20-16} = rs;
   let Inst{15-0} = imm;
 }

