//===-- UPTISelLowering.cpp - UPT DAG Lowering Implementation ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that UPT uses to lower LLVM code into a
// selection DAG.
//===----------------------------------------------------------------------===//
#include "UPTISelLowering.h"
#include "UPT.h"
#include "UPTMachineFunctionInfo.h"
#include "UPTSubtarget.h"
#include "UPTTargetMachine.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineJumpTableInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#define DEBUG_TYPE "upt-lower"

using namespace llvm;

const char *UPTTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  default:return nullptr;
  case UPTISD::RET_FLAG: return "RET_FLAG";
  case UPTISD::LOAD_SYM: return "LOAD_SYM";
  case UPTISD::MOVEi32: return "MOVEi32";
  case UPTISD::CALL: return "CALL";
  }
}

UPTTargetLowering::UPTTargetLowering(UPTTargetMachine &UPTTM)
    : TargetLowering(UPTTM), Subtarget(*UPTTM.getSubtargetImpl()) {
  LLVM_DEBUG(dbgs() << ">>UPT TargetLowering::constructor <<\n");
  // Set up the register classes.
  addRegisterClass(MVT::i32, &UPT::GRRegsRegClass);
  // Compute derived properties from the register classes
  computeRegisterProperties(Subtarget.getRegisterInfo());
  setStackPointerRegisterToSaveRestore(UPT::SP);
  /// Specify the target scheduling preference.
  setSchedulingPreference(Sched::Source);
  // Enum that describes how the target represents true/false values.
  setBooleanContents(ZeroOrOneBooleanContent);
  // Nodes that require custom lowering
  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);

  // Type of load instruction is not supported
  for (MVT VT : MVT::integer_valuetypes()) {
    for (auto N : {ISD::EXTLOAD, ISD::SEXTLOAD, ISD::ZEXTLOAD}) {
      setLoadExtAction(N, VT, MVT::i1, Promote);
      setLoadExtAction(N, VT, MVT::i8, Promote);
      setLoadExtAction(N, VT, MVT::i16, Promote);
    }
  }

  // Nodes that require custom lowering
  if (Subtarget.hasStdExtM()) {
    setOperationAction(ISD::MUL, MVT::i32, Expand);
    setOperationAction(ISD::MULHS, MVT::i32, Expand);
    setOperationAction(ISD::MULHU, MVT::i32, Expand);
    setOperationAction(ISD::SDIV, MVT::i32, Expand);
    setOperationAction(ISD::UDIV, MVT::i32, Expand);
    setOperationAction(ISD::SREM, MVT::i32, Expand);
    setOperationAction(ISD::UREM, MVT::i32, Expand);
  }
  if (Subtarget.useSoftMulDiv()) {
    // .umul works for both signed and unsigned
    setOperationAction(ISD::SMUL_LOHI, MVT::i32, Expand);
    setOperationAction(ISD::UMUL_LOHI, MVT::i32, Expand);
    setLibcallName(RTLIB::MUL_I32, ".umul");

    setOperationAction(ISD::SDIV, MVT::i32, Expand);
    setLibcallName(RTLIB::SDIV_I32, ".div");

    setOperationAction(ISD::UDIV, MVT::i32, Expand);
    setLibcallName(RTLIB::UDIV_I32, ".udiv");

    setLibcallName(RTLIB::SREM_I32, ".rem");
    setLibcallName(RTLIB::UREM_I32, ".urem");
  }

}

SDValue UPTTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  default:llvm_unreachable("Unimplemented operand");
  case ISD::GlobalAddress:return LowerGlobalAddress(Op, DAG);
  }
}

SDValue UPTTargetLowering::LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const {
  EVT VT = Op.getValueType();
  const GlobalAddressSDNode *GlobalAddr = cast<GlobalAddressSDNode>(Op.getNode());
  SDValue TargetAddr =
      DAG.getTargetGlobalAddress(GlobalAddr->getGlobal(), Op, MVT::i32);
  return DAG.getNode(UPTISD::LOAD_SYM, Op, VT, TargetAddr);
}

//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "UPTGenCallingConv.inc"

/// Implemented to lower calls into the specified DAG.
/// The outgoing arguments to the call are described by the Outs array,
/// and the values to be returned by the call are described by the Ins
/// array. The implementation should fill in the InVals array with legal-type
/// return values from the call, and return the resulting token chain value.
/// Here - functions arguments are copied from virtual regs to
/// physical regs or/and stack frame, CALLSEQ_START and CALLSEQ_END are emitted.
SDValue UPTTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                     SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &Loc = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  CallingConv::ID CallConv = CLI.CallConv;
  const bool isVarArg = CLI.IsVarArg;

  CLI.IsTailCall = false;

  if (isVarArg) {
    llvm_unreachable("Unimplemented");
  }

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  // CCInfo is a state,it captures which registers are already assigned and which stack slots are used.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs, *DAG.getContext());
  // for each operand ,if it follow CC_UPT,assign a location(stack/register,see CC.td) to it
  CCInfo.AnalyzeCallOperands(Outs, CC_UPT);

  // Get the size of the outgoing arguments stack space requirement.
  const unsigned NumBytes = CCInfo.getNextStackOffset();

  Chain = DAG.getCALLSEQ_START(Chain, NumBytes,0 , Loc);

  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;

  // Walk the register/memloc assignments, inserting sotres.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];

    // We only handle fully promoted arguments.
    assert(VA.getLocInfo() == CCValAssign::Full && "UPT Unhandled loc info");

    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
      continue;
    }

    assert(VA.isMemLoc() && "UPT Only support passing arguments through registers or via the stack");

    SDValue StackPtr = DAG.getRegister(UPT::SP, MVT::i32);
    SDValue PtrOff = DAG.getIntPtrConstant(VA.getLocMemOffset(), Loc);
    PtrOff = DAG.getNode(ISD::ADD, Loc, MVT::i32, StackPtr, PtrOff);
    SDValue Store =
        DAG.getStore(Chain, Loc, Arg, PtrOff, MachinePointerInfo());
    MemOpChains.push_back(Store);
  }

  // Emit all stores, make sure they occur before the call.
  if (!MemOpChains.empty()) {
    Chain = DAG.getNode(ISD::TokenFactor, Loc, MVT::Other, MemOpChains);
  }

  // Build a sequence of copy-to-reg nodes chained together with token chain
  // and flag operands which copy the outgoing args into the appropriate regs.
  SDValue InFlag;
  for (auto &Reg : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, Loc, Reg.first, Reg.second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // We only support calling global addresses.
  GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee);
  assert(G && "UPT only support the calling of global addresses");

  EVT PtrVT = getPointerTy(DAG.getDataLayout());
  Callee = DAG.getGlobalAddress(G->getGlobal(), Loc, PtrVT, 0);

  std::vector<SDValue> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are known live
  // into the call.
  for (auto &Reg : RegsToPass) {
    Ops.push_back(DAG.getRegister(Reg.first, Reg.second.getValueType()));
  }

  // Add a register mask operand representing the call-preserved registers.
  const uint32_t *Mask;
  const TargetRegisterInfo *TRI = DAG.getSubtarget().getRegisterInfo();
  Mask = TRI->getCallPreservedMask(DAG.getMachineFunction(), CallConv);

  assert(Mask && "UPT Missing call preserved mask for calling convention");
  Ops.push_back(DAG.getRegisterMask(Mask));

  if (InFlag.getNode()) {
    Ops.push_back(InFlag);
  }

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);

  // Returns a chain and a flag for retval copy to use.
  Chain = DAG.getNode(UPTISD::CALL, Loc, NodeTys, Ops);
  InFlag = Chain.getValue(1);

  Chain = DAG.getCALLSEQ_END(Chain, DAG.getIntPtrConstant(NumBytes, Loc, true),
                             DAG.getIntPtrConstant(0, Loc, true), InFlag, Loc);
  if (!Ins.empty()) {
    InFlag = Chain.getValue(1);
  }

  // Handle result values, copying them out of physregs into vregs that we return.
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg, Ins, Loc, DAG,
                         InVals);
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
SDValue UPTTargetLowering::LowerCallResult(
    SDValue Chain, SDValue InGlue, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl, SelectionDAG &DAG,
    SmallVectorImpl<SDValue> &InVals) const {
  assert(!isVarArg && "Unsupported");

  // Assign locations to each value returned by this call.Seems like at most 16 return values
  SmallVector<CCValAssign, 16> RVLocs;
  //It captures which registers are already assigned and which stack slots are used.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs, *DAG.getContext());
  // for each operand ,if it follow RetCC_UPT,assign a location(stack/register,see CC.td) to it
  CCInfo.AnalyzeCallResult(Ins, RetCC_UPT);

  // Copy all of the result registers out of their specified physreg.
  for (auto &Loc : RVLocs) {
    Chain = DAG.getCopyFromReg(Chain, dl, Loc.getLocReg(), Loc.getValVT(), InGlue).getValue(1);
    InGlue = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//             Formal Arguments Calling Convention Implementation
//===----------------------------------------------------------------------===//

/// UPT formal arguments implementation
/// This hook must be implemented to lower the incoming (formal) arguments,
/// described by the Ins array, into the specified DAG. The implementation
/// should fill in the InVals array with legal-type argument values, and
/// return the resulting token chain value.
/// Here we lower each i32 arguments to
/// 1. virtual reg if the argument passed as a register
/// 2. stack if the augument passed as memory
SDValue UPTTargetLowering::LowerFormalArguments(SDValue Chain,
                                                CallingConv::ID CallConv,
                                                bool isVarArg,
                                                const SmallVectorImpl<ISD::InputArg> &Ins,
                                                const SDLoc &dl,
                                                SelectionDAG &DAG,
                                                SmallVectorImpl<SDValue> &InVals) const {
  LLVM_DEBUG(dbgs() << ">> UPT TargetLowering::LowerFormalArguments <<\n");
  LLVM_DEBUG(dbgs() << "  Chain: ";
                 Chain->dumpr(););

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  // don't spport var arguments function call
  assert(!isVarArg && "VarArg not supported");

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_UPT);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc()) {
      // if the argument is passed as a register
      EVT RegVT = VA.getLocVT();
      LLVM_DEBUG(dbgs() << "  Reg N" << i
                   << " UPT LowerFormalArguments Unhandled argument type: ");
//                   << RegVT.getSimpleVT().SimpleTy << "\n";);
      assert(RegVT.getSimpleVT().SimpleTy == MVT::i32 &&
          "Only support MVT::i32 register passing");
      const unsigned VReg = RegInfo.createVirtualRegister(&UPT::GRRegsRegClass);
      RegInfo.addLiveIn(VA.getLocReg(), VReg);
      SDValue ArgIn = DAG.getCopyFromReg(Chain, dl, VReg, RegVT);

      InVals.push_back(ArgIn);
      continue;
    }
    // pass by memory
    assert(VA.isMemLoc() &&
        "UPT Can only pass arguments as either registers or via the stack");
    LLVM_DEBUG(dbgs() << "  Mem N" << i
                 << " LowerFormalArguments Unhandled argument type: "
                 << EVT(VA.getLocVT()).getEVTString()
                 << "\n";);
    const unsigned Offset = VA.getLocMemOffset();

    const int FI = MF.getFrameInfo().CreateFixedObject(4, Offset, true);
    EVT PtrTy = getPointerTy(DAG.getDataLayout());
    SDValue FIPtr = DAG.getFrameIndex(FI, PtrTy);

    assert(VA.getValVT() == MVT::i32 &&
        "UPT Only support passing arguments as i32");
    // 创建一个LOAD节点以从堆栈中load参数
    SDValue Load = DAG.getLoad(VA.getValVT(), dl, Chain, FIPtr,MachinePointerInfo());

    InVals.push_back(Load);
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//               Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//
/// true iff the function's return value can be lowered to
/// registers.
bool UPTTargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &MF, bool isVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Context) const {
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, Context);
  if (!CCInfo.CheckReturn(Outs, RetCC_UPT)) {
    return false;
  }
  if (CCInfo.getNextStackOffset() != 0 && isVarArg) {
    return false;
  }
  return true;
}

/// Lower outgoing return values, described by the Outs array, into the specified DAG.
/// The implementation should return the resulting token chain value.
SDValue
UPTTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &OutVals, const SDLoc &dl,
                               SelectionDAG &DAG) const {
  if (isVarArg) {
    report_fatal_error("UPT VarArg not supported");
  }

  // CCValAssign - represent the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());
  // following return calling convention ?
  CCInfo.AnalyzeReturn(Outs, RetCC_UPT);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0, e = RVLocs.size(); i < e; ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "UPT Can only return in registers!");
    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), OutVals[i], Flag);
    // Guarantee that all emitted copies are stuck together
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode()) {
    RetOps.push_back(Flag);
  }

  return DAG.getNode(UPTISD::RET_FLAG, dl, MVT::Other, RetOps);
}

