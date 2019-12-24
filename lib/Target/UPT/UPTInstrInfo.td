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
                      (outs GRRegs:$rd), (ins GRRegs:$rs, GRRegs:$rt), // 输入输出类型和个数
                      !strconcat(asmstr, "R $rd, $rs, $rt"),   //汇编输出完整形式
                      [(set GRRegs:$rd, (OpNode GRRegs:$rs, GRRegs:$rt))]>;  // 匹配模式

  def I : UPTInstFormReg2I<{op{5}, op{4}, op{3}, op{2}, op{1}, 1},
                      (outs GRRegs:$rd), (ins GRRegs:$rs, i32imm:$imm),
                      !strconcat(asmstr, "I $rd, $rs, $imm"),
                      [(set GRRegs:$rd, (OpNode GRRegs:$rs, immZExt16:$imm))]>;
}

//5bit imm zext to 32bit
//===----------------------------------------------------------------------===//
// SHift Left/Right Arthimetic/Logical Instructions
//===----------------------------------------------------------------------===//

multiclass ShiftInst<bits<6> op, string asmstr, SDNode OpNode>{
  def R : UPTInstFormReg3<op,
                      (outs GRRegs:$rd), (ins GRRegs:$rs, GRRegs:$rt),
                      !strconcat(asmstr, "R $rd, $rs, $rt"),
                      [(set GRRegs:$rd, (OpNode GRRegs:$rs, GRRegs:$rt))]>;
  def I : UPTInstFormReg2I<{op{5}, op{4}, op{3}, op{2}, op{1}, 1},
                      (outs GRRegs:$rd), (ins GRRegs:$rs, i32imm:$imm),
                      !strconcat(asmstr, "I $rd, $rs, $imm"),
                      [(set GRRegs:$rd, (OpNode GRRegs:$rs, immZExt5:$imm))]>;
}

//16bit imm zext to 32bit
//===----------------------------------------------------------------------===//
// Arithmetic Instructions
//===----------------------------------------------------------------------===//
multiclass ArithInst<bits<6> op/*b位操作码*/, string asmstr/*指令汇编名*/, SDNode OpNode/* DAG节点*/>{
  def R : UPTInstFormReg3<op,  // 操作码
                      (outs GRRegs:$rd), (ins GRRegs:$rs, GRRegs:$rt), // 输入输出类型和个数
                      !strconcat(asmstr, "R $rd, $rs, $rt"),   //汇编输出完整形式
                      [(set GRRegs:$rd, (OpNode GRRegs:$rs, GRRegs:$rt))]>; // 匹配模式
  def I : UPTInstFormReg2I<{op{5}, op{4}, op{3}, op{2}, op{1}, 1},
                      (outs GRRegs:$rd), (ins GRRegs:$rs, i32imm:$imm),
                      !strconcat(asmstr, "I $rd, $rs, $imm"),
                      [(set GRRegs:$rd, (OpNode GRRegs:$rs, immZExt16:$imm))]>;
}



defm AND : LogicInst<0b000000, "AND", and>;
defm OR  : LogicInst<0b000010, "OR", or>;
defm XOR : LogicInst<0b000100, "XOR", xor>;
defm ADD : ArithInst<0b000110, "ADD", add>;
defm SUB : ArithInst<0b001000, "SUB", sub>;
defm SRL : ShiftInst<0b001010, "SRL", srl>;
defm SLL : ShiftInst<0b001100, "SLL", shl>;
defm SRA : ShiftInst<0b001110, "SRA", sra>;

//===----------------------------------------------------------------------===//
// Comparison Instructions
//===----------------------------------------------------------------------===//

def CMP : UPTInstFormReg2<0b010000, (outs), (ins GRRegs:$rd, GRRegs:$rs),
    !strconcat("CMP"," $rd, $rs"), []>{
  let Defs = [SR];
}

//===----------------------------------------------------------------------===//
// Load/Store Instructions
//===----------------------------------------------------------------------===//

def LDR : UPTInstFormReg1I<0b010110, (outs GRRegs:$rd), (ins memsrc:$imm),
  !strconcat("LDR", " $rd, $imm"),[(set GRRegs:$rd, (load addr:$imm))]>{
}

// Store
def STR : UPTInstFormReg1I<0b010110, (outs), (ins GRRegs:$rd, memsrc:$imm),
  !strconcat("STR", " $rd, $imm"),[(store GRRegs:$rd, addr:$imm)]>{
}



//===----------------------------------------------------------------------===//
// Mov Immediate Instructions
//===----------------------------------------------------------------------===//

multiclass MovInst<bits<6> op, string asmstr> {
  def LOi16 : UPTInstFormReg1I<op,
                      (outs GRRegs:$rd), (ins i32imm:$imm),
                      !strconcat(asmstr,"L $rd, $imm"),  /* Mov Lower Part*/
                      [(set i32:$rd, i32imm_lo:$imm)]>;
  def HIi16 : UPTInstFormReg1I<{op{5}, op{4}, op{3}, op{2}, op{1}, 1},
                      (outs GRRegs:$rd), (ins GRRegs:$src1, i32imm:$imm),
                      !strconcat(asmstr,"U $rd, $imm"),  /* Mov Upper Part*/
                      [/* No pattern */]>;
}


def MOVi32 : UPTPseudo<(outs GRRegs:$rd), (ins i32imm:$imm), "",[(set i32:$rd, (movei32 imm:$imm))]> {}
def : Pattern<(i32 (load_sym tglobaladdr:$addr)),  [(MOVi32 $addr)]>;
defm MOV : MovInst<0b011000,"MOV">;

//===----------------------------------------------------------------------===//
// Return and Call Instructions
//===----------------------------------------------------------------------===//


let isCall=1, Defs=[RA],Uses=[SP] in {
  def CALL: UPTInstFormReg1<0b011011,(outs), (ins GRRegs:$rd),
          "CALL $rd", [(upt_call i32:$rd)]> {}
}

let isBranch=1 ,isTerminator=1, isBarrier=1, isReturn=1, Uses= [RA] in {
  def RET:UPTInstFormReg1<0b011010, (outs), (ins variable_ops),"RET RA", [(UPTRetFlag)]> {
    let Inst{25-21}= 0xF;
  }
}
//===----------------------------------------------------------------------===//
// Branch Instructions
//===----------------------------------------------------------------------===//

let isBranch=1,isBarrier=1,isTerminator=1 in {
  def JMP : UPTInstFormReg0<0b010010, (outs), (ins b_target:$addr),
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




def Bcc : UPTInstFormReg0<0b100000, (outs), (ins cc_val:$cc, b_target:$addr),
    "B$cc $addr", []> {
  let Uses = [SR];
  let isBranch=1;
  let isTerminator=1;
}

//let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
//class BranchCC_rri<op, string asmstr>
//    : UPTInstFormReg2I<op,
//              (outs GRRegs:$rd), (ins GRRegs:$rs, i32imm:$imm),
//              !strconcat(asmstr, "$rd, $rs, $imm"),
//              [(set GRRegs:$rd, (OpNode GRRegs:$rs, immZExt16:$imm))]>; {
//  let isBranch = 1;
//  let isTerminator = 1;
//}
//
//def BEQ  : BranchCC_rri<0b101110, "beq">;
//def BNE  : BranchCC_rri<0b100001, "bne">;
//def BLT  : BranchCC_rri<0b100000, "blt">;
//def BGE  : BranchCC_rri<0b100101, "bge">;
//def BLTU : BranchCC_rri<0b100110, "bltu">;
//def BGEU : BranchCC_rri<0b100111, "bgeu">;
//
///// Branches and jumps
//
//// Match `(brcond (CondOp ..), ..)` and lower to the appropriate RISC-V branch
//// instruction.
//class BccPat<PatFrag CondOp, RVInstB Inst>
//    : Pat<(brcond (XLenVT (CondOp GPR:$rs1, GPR:$rs2)), bb:$imm12),
//          (Inst GPR:$rs1, GPR:$rs2, simm13_lsb0:$imm12)>;
//
//def : BccPat<seteq, BEQ>;
//def : BccPat<setne, BNE>;
//def : BccPat<setlt, BLT>;
//def : BccPat<setge, BGE>;
//def : BccPat<setult, BLTU>;
//def : BccPat<setuge, BGEU>;
//
//class BccSwapPat<PatFrag CondOp, RVInst InstBcc>
//    : Pat<(brcond (XLenVT (CondOp GPR:$rs1, GPR:$rs2)), bb:$imm12),
//          (InstBcc GPR:$rs2, GPR:$rs1, bb:$imm12)>;
//
//// Condition codes that don't have matching RISC-V branch instructions, but
//// are trivially supported by swapping the two input operands
//def : BccSwapPat<setgt, BLT>;
//def : BccSwapPat<setle, BGE>;
//def : BccSwapPat<setugt, BLTU>;
//def : BccSwapPat<setule, BGEU>;
//
//// An extra pattern is needed for a brcond without a setcc (i.e. where the
//// condition was calculated elsewhere).
//def : Pat<(brcond GPR:$cond, bb:$imm12), (BNE GPR:$cond, X0, bb:$imm12)>;
//

//===----------------------------------------------------------------------===//
// Standard extensions
//===----------------------------------------------------------------------===//

include "UPTInstrInfoIntegerMul.td"
include "UPTInstrInfoFloatPoint.td"