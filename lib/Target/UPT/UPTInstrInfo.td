//===-- UPTInstrInfo.td - Target Description for UPT ---*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the UPT instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

include "UPTInstrFormats.td"
include "UPTOperators.td"

//===----------------------------------------------------------------------===//
// Logic Instructions
//===----------------------------------------------------------------------===//

multiclass LogicInst<bits<6> op/*b位操作码*/, string asmstr/*指令汇编名*/, SDNode OpNode/* DAG节点*/>{
  def R : UPTInstFormReg3<op,  // 操作码
                      (outs GPR:$rd), (ins GPR:$rs, GPR:$rt), // 输入输出类型和个数
                      !strconcat(asmstr, "R $rd, $rs, $rt"),   //汇编输出完整形式
                      [(set GPR:$rd, (OpNode GPR:$rs, GPR:$rt))]>;  // 匹配模式

  def I : UPTInstFormReg2I<{op{5}, op{4}, op{3}, op{2}, op{1}, 1},
                      (outs GPR:$rd), (ins GPR:$rs, i32imm:$imm),
                      !strconcat(asmstr, "I $rd, $rs, $imm"),
                      [(set GPR:$rd, (OpNode GPR:$rs, immZExt16:$imm))]>;
}

//5bit imm zext to 32bit
//===----------------------------------------------------------------------===//
// SHift Left/Right Arthimetic/Logical Instructions
//===----------------------------------------------------------------------===//

multiclass ShiftInst<bits<6> op, string asmstr, SDNode OpNode>{
  def R : UPTInstFormReg3<op,
                      (outs GPR:$rd), (ins GPR:$rs, GPR:$rt),
                      !strconcat(asmstr, "R $rd, $rs, $rt"),
                      [(set GPR:$rd, (OpNode GPR:$rs, GPR:$rt))]>;
  def I : UPTInstFormReg2I<{op{5}, op{4}, op{3}, op{2}, op{1}, 1},
                      (outs GPR:$rd), (ins GPR:$rs, i32imm:$imm),
                      !strconcat(asmstr, "I $rd, $rs, $imm"),
                      [(set GPR:$rd, (OpNode GPR:$rs, immZExt5:$imm))]>;
}

//16bit imm zext to 32bit
//===----------------------------------------------------------------------===//
// Arithmetic Instructions
//===----------------------------------------------------------------------===//
multiclass ArithInst<bits<6> op/*b位操作码*/, string asmstr/*指令汇编名*/, SDNode OpNode/* DAG节点*/>{
  def R : UPTInstFormReg3<op,  // 操作码
                      (outs GPR:$rd), (ins GPR:$rs, GPR:$rt), // 输入输出类型和个数
                      !strconcat(asmstr, "R $rd, $rs, $rt"),   //汇编输出完整形式
                      [(set GPR:$rd, (OpNode GPR:$rs, GPR:$rt))]>; // 匹配模式
  def I : UPTInstFormReg2I<{op{5}, op{4}, op{3}, op{2}, op{1}, 1},
                      (outs GPR:$rd), (ins GPR:$rs, i32imm:$imm),
                      !strconcat(asmstr, "I $rd, $rs, $imm"),
                      [(set GPR:$rd, (OpNode GPR:$rs, immSExt16:$imm))]>;
}



defm AND : LogicInst<0x00, "AND", and>;
defm OR  : LogicInst<0x02, "OR",  or>;
defm XOR : LogicInst<0x04, "XOR", xor>;
defm ADD : ArithInst<0x06, "ADD", add>;
defm SUB : ArithInst<0x08, "SUB", sub>;
defm SRL : ShiftInst<0x0A, "SRL", srl>;
defm SLL : ShiftInst<0x0C, "SLL", shl>;
defm SRA : ShiftInst<0x0D, "SRA", sra>;

//===----------------------------------------------------------------------===//
// Load/Store Instructions
//===----------------------------------------------------------------------===//



def LDR : UPTInstFormReg2I<0x0E, (outs GPR:$rd), (ins memsrc:$imm),
  !strconcat("LDR", "  $rd, $imm"),[(set GPR:$rd, (load addr:$imm))]>{
}

def STR : UPTInstFormReg2I<0x0F, (outs), (ins GPR:$rd, memsrc:$imm),
  !strconcat("STR", "  $rd, $imm"),[(store GPR:$rd, addr:$imm)]>{
}




//===----------------------------------------------------------------------===//
// Mov Immediate Instructions
//===----------------------------------------------------------------------===//
multiclass MovInst<bits<6> op, string asmstr> {
  def LOi16 : UPTInstFormReg2I<op,
                      (outs GPR:$rd), (ins i32imm:$imm),
                      !strconcat(asmstr,"L $rd, $imm"),  /* Mov Lower Part*/
                      [(set i32:$rd, immZExt16:$imm)]>;
  def HIi16 : UPTInstFormReg2I<{op{5}, op{4}, op{3}, op{2}, op{1}, 1},
                      (outs GPR:$rd), (ins GPR:$src1, i32imm:$imm),
                      !strconcat(asmstr,"U $rd, $imm"),  /* Mov Upper Part*/
                      [/* No pattern */]>;
}
defm MOV : MovInst<0x10,"MOV">;
def MOVi32 : UPTPseudo<(outs GPR:$rd), (ins i32imm:$imm), "",[(set i32:$rd, (movei32 imm:$imm))]> {}

//// Transformation function: get the low 16 bits.
//def LO16 : SDNodeXForm<imm, [{return getI32Imm(N->getZExtValue(), SDLoc(N));}]>;
//// Transformation function: shift the immediate value down into the low bits.
//def HI16 : SDNodeXForm<imm, [{return getI32Imm(N->getZExtValue() >> 16, SDLoc(N));


// Small immediates
def : Pat<(i32 immSExt16:$imm),(ADDI ZERO, imm:$imm)>;
def : Pat<(i32 immZExt16:$imm),(ORI  ZERO, imm:$imm)>;

// Arbitrary immediates
//def : Pat<(i32 imm:$imm),(ORI (ORI ZERO, (HI16 imm:$imm)), (LO16 imm:$imm))>;
def : Pat<(i32 imm:$imm),(MOVi32 $imm)>;

def : InstAlias<"MOV  $rd,$rs",   (ADDR GPR:$rd, GPR:$rs, ZERO)>;
def : InstAlias<"MOV  $rd,$rs",   (ADDR GPR:$rd, ZERO, GPR:$rs)>;
def : InstAlias<"MOV  $rd,$imm",  (ADDI GPR:$rd, ZERO, i32imm:$imm)>;
def : InstAlias<"MOV  $rd,$imm",  (ORI  GPR:$rd, ZERO, i32imm:$imm)>;


//===----------------------------------------------------------------------===//
// Return and Call Instructions
//===----------------------------------------------------------------------===//


let isCall=1, Defs=[RA],Uses=[SP] in {
  def CALL: UPTInstFormReg1<0x12,(outs), (ins GPR:$rd),
          "CALL $rd", [(upt_call i32:$rd)]> {}
}
def : Pat<(i32 (load_sym tglobaladdr:$addr)),  (MOVi32 $addr)>;

let isBranch=1 ,isTerminator=1, isReturn=1, Uses= [RA] in {
  def RET:UPTInstFormReg1<0x13, (outs), (ins variable_ops),"RET", [(UPTRetFlag)]> {
    let Inst{25-21}= 0xF;
  }
}

//===----------------------------------------------------------------------===//
// Comparison Instructions
//===----------------------------------------------------------------------===//

def CMP : UPTInstFormReg2<0x14, (outs), (ins GPR:$rd, GPR:$rs),
  !strconcat("CMP","  $rd, $rs"), []>{
  let Defs = [SR];
}

//===----------------------------------------------------------------------===//
// Branch Instructions
//===----------------------------------------------------------------------===//

let isBranch=1, isTerminator=1 in {
  def JMP : UPTInstFormReg0<0x20, (outs), (ins b_target:$addr),
          "JMP $addr", [(br bb:$addr)]> {}
}

//===----------------------------------------------------------------------===//
// Pseudo Instructions
//===----------------------------------------------------------------------===//
let Defs = [SP], Uses = [SP] in {
  def ADJCALLSTACKDOWN : UPTPseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
      "# ADJCALLSTACKDOWN $amt1 $amt2", [(callseq_start timm:$amt1, timm:$amt2)]>;
  def ADJCALLSTACKUP   : UPTPseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
      "# ADJCALLSTACKUP $amt1 $amt2", [(callseq_end timm:$amt1, timm:$amt2)]>;
}




def Bcc : UPTInstFormReg0<0x21, (outs), (ins cc_val:$cc, b_target:$addr),
    "B$cc $addr", []> {
  bits<6> cc_val;
  let Opcode= cc_val;
  let Uses = [SR];
  let isBranch=1;
  let isTerminator=1;
}



//===----------------------------------------------------------------------===//
// Standard extensions
//===----------------------------------------------------------------------===//

include "UPTInstrInfoIntegerMul.td"
include "UPTInstrInfoFloatPoint.td"