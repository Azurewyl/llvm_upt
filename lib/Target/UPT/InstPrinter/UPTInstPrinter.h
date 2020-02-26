//== UPTInstPrinter.h - Convert UPT MCInst to assembly syntax -*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file contains the declaration of the UPTInstPrinter class,
/// which is used to print UPT MCInst to a .s file.
///
//===----------------------------------------------------------------------===//

#ifndef UPTINSTPRINTER_H
#define UPTINSTPRINTER_H
#include "llvm/ADT/Triple.h"
#include "llvm/MC/MCInstPrinter.h"

namespace llvm {

class TargetMachine;

class UPTInstPrinter : public MCInstPrinter {
public:
  UPTInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                 const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

  // Autogenerated by tblgen.
  bool printAliasInstr(const MCInst *MI, raw_ostream &OS);
  void printInstruction(const MCInst *MI, raw_ostream &O);
  void printCustomAliasOperand(const MCInst *MI, unsigned OpIdx,unsigned PrintMethodIdx,raw_ostream &OS);
  static const char *getRegisterName(unsigned RegNo);

  void printRegName(raw_ostream &OS, unsigned RegNo) const override;
  void printInst(const MCInst *MI, raw_ostream &OS, StringRef Annot,
                         const MCSubtargetInfo &STI) override;

private:
  void printCondCode(const MCInst *MI, unsigned OpNum, raw_ostream &O);
  void printAddrModeMemSrc(const MCInst *MI, unsigned OpNum, raw_ostream &O);
  void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &O);
};
} // end namespace llvm

#endif
