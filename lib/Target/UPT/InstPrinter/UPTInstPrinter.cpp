//===-- UPTInstPrinter.cpp - Convert UPT MCInst to assembly syntax ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an UPT MCInst to a .s file.
//
//===----------------------------------------------------------------------===//


#include "UPTInstPrinter.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer:to .s file"
#include "UPTGenAsmWriter.inc"

void UPTInstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const {
  OS << StringRef(getRegisterName(RegNo));
}

void UPTInstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                               StringRef Annot, const MCSubtargetInfo &STI) {
  printInstruction(MI, O);
  printAnnotation(O, Annot);
}

static void printExpr(const MCExpr *Expr, raw_ostream &OS) {
  int64_t Offset = 0;
  const MCSymbolRefExpr *SRE;

  if (const MCBinaryExpr *BE = dyn_cast<MCBinaryExpr>(Expr)) {
    SRE = dyn_cast<MCSymbolRefExpr>(BE->getLHS());
    const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(BE->getRHS());
    assert(SRE && CE && "Binary expression must be sym+const.");
    Offset = CE->getValue();
  } else {
    SRE = dyn_cast<MCSymbolRefExpr>(Expr);
    assert(SRE && "Unexpected MCExpr type.");
  }
  const MCSymbolRefExpr::VariantKind Kind = SRE->getKind();
  assert(Kind == MCSymbolRefExpr::VK_None ||
      Kind == MCSymbolRefExpr::VK_UPT_LO ||
      Kind == MCSymbolRefExpr::VK_UPT_HI);

  OS << SRE->getSymbol();

  if (Offset) {
    if (Offset > 0) {
      OS << '+';
    }
    OS << Offset;
  }
}

const char *condCodeToString(ISD::CondCode CC) {
  switch (CC) {
  case ISD::SETCC_INVALID:
  case ISD::SETFALSE:      //    0 0 0 0       Always false (always folded)
  case ISD::SETFALSE2:     //  1 X 0 0 0       Always false (always folded)
  case ISD::SETOEQ:        //    0 0 0 1       True if ordered and equal
  case ISD::SETOGT:        //    0 0 1 0       True if ordered and greater than
  case ISD::SETOGE:        //    0 0 1 1       True if ordered and greater than or equal
  case ISD::SETOLT:        //    0 1 0 0       True if ordered and less than
  case ISD::SETOLE:        //    0 1 0 1       True if ordered and less than or equal
  case ISD::SETONE:        //    0 1 1 0       True if ordered and operands are unequal
  case ISD::SETO:          //    0 1 1 1       True if ordered (no nans)
  case ISD::SETUO:         //    1 0 0 0       True if unordered: isnan(X) | isnan(Y)
    llvm_unreachable("Invalid or unsupported condition code");
    return nullptr;
  case ISD::SETTRUE:       //    1 1 1 1       Always true (always folded)
  case ISD::SETTRUE2:      //  1 X 1 1 1       Always true (always folded)
    return "";
  case ISD::SETUEQ:        //    1 0 0 1       True if unordered or equal
    return "ueq";
  case ISD::SETUGT:        //    1 0 1 0       True if unordered or greater than
    return "ugt";
  case ISD::SETUGE:        //    1 0 1 1       True if unordered, greater than, or equal
    return "uge";
  case ISD::SETULT:        //    1 1 0 0       True if unordered or less than
    return "ult";
  case ISD::SETULE:        //    1 1 0 1       True if unordered, less than, or equal
    return "ule";
  case ISD::SETUNE:        //    1 1 1 0       True if unordered or not equal
    return "une";
    // Don't care operations: undefined if the input is a nan.
  case ISD::SETEQ:         //  1 X 0 0 1       True if equal
    return "EQ";
  case ISD::SETGT:         //  1 X 0 1 0       True if greater than
    return "GT";
  case ISD::SETGE:         //  1 X 0 1 1       True if greater than or equal
    return "GE";
  case ISD::SETLT:         //  1 X 1 0 0       True if less than
    return "LT";
  case ISD::SETLE:         //  1 X 1 0 1       True if less than or equal
    return "LE";
  case ISD::SETNE:         //  1 X 1 1 0       True if not equal
    return "NE";
  default:llvm_unreachable("Invalid or unsupported condition code");
  }
}

// Print a condition code (e.g. for predication).
void UPTInstPrinter::printCondCode(const MCInst *MI, unsigned OpNum,
                                   raw_ostream &O) {
  auto Op = MI->getOperand(OpNum);
  auto CC = (ISD::CondCode) Op.getImm();
  const char *Str = condCodeToString(CC);
  O << Str;
}

// Print a 'memsrc' operand which is a (Register, Offset) pair.
void UPTInstPrinter::printAddrModeMemSrc(const MCInst *MI, unsigned OpNum,
                                         raw_ostream &O) {
  auto Op1 = MI->getOperand(OpNum);
  auto Op2 = MI->getOperand(OpNum + 1);
  O << "[";
  printRegName(O, Op1.getReg());

  auto Offset = Op2.getImm();
  if (Offset) {
    O << ", " << Offset;
  }
  O << "]";
}

// Print the operand of an MI given operand Number
// If the operand is a register, print the name/alias
// If the opearnd is an imm, print it directly
void UPTInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                  raw_ostream &O) {
  auto Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    printRegName(O, Op.getReg());
    return;
  }

  if (Op.isImm()) {
    O << Op.getImm();
    return;
  }

  assert(Op.isExpr() && "unknown operand kind in printOperand");
  printExpr(Op.getExpr(), O);
}
