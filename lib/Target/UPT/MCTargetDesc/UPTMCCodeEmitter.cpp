//===-- UPT/UPTMCCodeEmitter.cpp - Convert UPT code to machine code -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the UPTMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "mccodeemitter"
#include "MCTargetDesc/UPTMCTargetDesc.h"
#include "MCTargetDesc/UPTFixupKinds.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "UPTMCTargetDesc.h"

using namespace llvm;

STATISTIC(MCNumEmitted, "Number of MC instructions emitted.");

namespace {
class UPTMCCodeEmitter : public MCCodeEmitter {
public:
  UPTMCCodeEmitter(const UPTMCCodeEmitter &) = delete;
  void operator=(const UPTMCCodeEmitter &) = delete;
  const MCInstrInfo &MCII;
  const MCContext &CTX;

  UPTMCCodeEmitter(const MCInstrInfo &mcii, MCContext &ctx)
      : MCII(mcii), CTX(ctx) {}

  ~UPTMCCodeEmitter() = default;

  // getBinaryCodeForInstr - TableGen created function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// getMachineOpValue - Return binary encoding of operand. If the machine
  /// operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  // 在TableGen的DecoderMethod中指定 ,获取加载/存储操作数的二进制编码
  unsigned getMemSrcValue(const MCInst &MI, unsigned OpIdx,
                          SmallVectorImpl<MCFixup> &Fixups,
                          const MCSubtargetInfo &STI) const;

  void EmitByte(unsigned char C, raw_ostream &OS) const { OS << (char)C; }

  void EmitConstant(uint64_t Val, unsigned Size, raw_ostream &OS) const {
    // Output the constant in little endian byte order.
    for (unsigned i = 0; i != Size; ++i) {
      EmitByte(Val & 255, OS);
      Val >>= 8;
    }
  }
  
  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;
};

} // end anonymous namespace

MCCodeEmitter *llvm::createUPTMCCodeEmitter(const MCInstrInfo &MCII,
                                            const MCRegisterInfo &MRI,
                                            MCContext &Ctx) {
  return new UPTMCCodeEmitter(MCII, Ctx);
}

/// getMachineOpValue - Return binary encoding of operand. If the machine
/// operand requires relocation, record the relocation and return zero.
unsigned UPTMCCodeEmitter::getMachineOpValue(const MCInst &MI,
                                             const MCOperand &MO,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  if (MO.isReg()) {
    return CTX.getRegisterInfo()->getEncodingValue(MO.getReg());
  }

  if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  }

  assert(MO.isExpr() && "unknown operand kind in printOperand");

  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();

  if (Kind == MCExpr::Binary) {
    Expr = static_cast<const MCBinaryExpr*>(Expr)->getLHS();
    Kind = Expr->getKind();
  }

  assert (Kind == MCExpr::SymbolRef);

  unsigned FixupKind;
  switch (cast<MCSymbolRefExpr>(Expr)->getKind()) {
  default:
    llvm_unreachable("Unknown fixup kind!");
  case MCSymbolRefExpr::VK_UPT_LO: {
    FixupKind = UPT::fixup_upt_mov_lo16_pcrel;
    break;
  }
  case MCSymbolRefExpr::VK_UPT_HI: {
    FixupKind = UPT::fixup_upt_mov_hi16_pcrel;
    break;
  }
  }

  Fixups.push_back(MCFixup::create(0, MO.getExpr(), MCFixupKind(FixupKind)));
  return 0;
}

unsigned UPTMCCodeEmitter::getMemSrcValue(const MCInst &MI, unsigned OpIdx,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  unsigned Bits = 0;
  const MCOperand &RegMO = MI.getOperand(OpIdx);
  const MCOperand &ImmMO = MI.getOperand(OpIdx + 1);
  assert(ImmMO.getImm() >= 0);
  Bits |= (getMachineOpValue(MI, RegMO, Fixups, STI) << 12);
  Bits |= (unsigned)ImmMO.getImm() & 0xfff;
  return Bits;
}



void UPTMCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  if (Desc.getSize() != 4) {
    llvm_unreachable("Unexpected instruction size!");
  }

  const uint32_t Binary = getBinaryCodeForInstr(MI, Fixups, STI);

  EmitConstant(Binary, Desc.getSize(), OS);
  ++MCNumEmitted;
}

#include "UPTGenMCCodeEmitter.inc"
