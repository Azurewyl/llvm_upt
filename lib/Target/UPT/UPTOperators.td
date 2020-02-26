//===-- UPTOperators.td - UPT-specific operators ------*- tblgen-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Type profiles and SDNodes
//===----------------------------------------------------------------------===//

// SDTypeProfile定义了SDNode的要求。
//第一个参数：结果节点号
//第二个参数：操作数节点数
//第三个参数：约束条件（SDTypeConstraint）

// SDTCisInt <N>：第N个操作数是Int类型
// SDTCisVT <N，VT>：第N个操作数是VT类型

// SDNode定义了一个新的SDNode。
//第一个参数：操作码
//第二个参数：约束条件（SDTypeProfile）
//第三个参数：SDNodeProperty


// SDNodeProperty
// SDNPCommutative : // 可換
// SDNPAssociative : // 結合法則
// SDNPHasChain    : // R/W chain operand and result
// SDNPOutGlue     : // Write a flag result
// SDNPInGlue      : // Read a flag operand
// SDNPOptInGlue   : // Optionally read a flag operand
// SDNPMayStore    : // May write to memory, sets 'mayStore'.
// SDNPMayLoad     : // May read memory, sets 'mayLoad'.
// SDNPSideEffect  : // Sets 'HasUnmodelledSideEffects'.
// SDNPMemOperand  : // Touches memory, has assoc MemOperand
// SDNPVariadic    : // 可变参数
// SDNPWantRoot    : // ComplexPattern gets the root of match
// SDNPWantParent  : // ComplexPattern gets the parent



def callseq_start : SDNode<"ISD::CALLSEQ_START",
                            SDCallSeqStart<[ SDTCisVT<0, i32> ]>,
                            [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END",
                            SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>,
                            [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def upt_call : SDNode<"UPTISD::CALL",SDTypeProfile<0, -1, [SDTCisPtrTy<0>]>,
                      [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;
def UPTRetFlag    : SDNode<"UPTISD::RET_FLAG", SDTNone,
                           [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def load_sym : SDNode<"UPTISD::LOAD_SYM", SDTIntUnaryOp>;

def movei32 : SDNode<"UPTISD::MOVEi32",
                     SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>, SDTCisInt<0>]>>;




//===----------------------------------------------------------------------===//
// Operand Definitions.
//===----------------------------------------------------------------------===//


def b_target : Operand<OtherVT>;

def cc_val : Operand<i32> {
  let PrintMethod = "printCondCode";
}

def memsrc : Operand<i32> {
  let MIOperandInfo = (ops GPR, i32imm);
  let PrintMethod = "printAddrModeMemSrc";
  let EncoderMethod = "getMemSrcValue";
}


def UPTimm8 : Operand<i32>, ImmLeaf<i32, [{
  return Imm >= 0 && Imm < 256;
}]>;

//===----------------------------------------------------------------------===//
// Pattern about imm
//===----------------------------------------------------------------------===//

// Signed extend 16bit
def immSExt16  : PatLeaf<(imm), [{ return isInt<16>(N->getSExtValue()); }]>;
// Zero extend Inst[4:0] or Inst[15:0] to 32bit
def immZExt5  : PatLeaf<(imm), [{ return isUInt<5>(N->getZExtValue()); }]>;
def immZExt16  : PatLeaf<(imm), [{ return isUInt<16>(N->getZExtValue()); }]>;


//===----------------------------------------------------------------------===//
// Complex Pattern Definitions.
//===----------------------------------------------------------------------===//

def addr : ComplexPattern<iPTR, 2, "SelectAddr", [], []>;



