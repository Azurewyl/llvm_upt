//===-- UPTMCAsmInfo.cpp - UPT asm properties -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "UPTMCAsmInfo.h"
using namespace llvm;

void UPTMCAsmInfo::anchor() {}

/// used to configure asm printer.
UPTMCAsmInfo::UPTMCAsmInfo(const Triple &TT) {
  /// These directives are used to output some unit of integer data to the current section
  Data8bitsDirective = "\t.byte\t";
  Data16bitsDirective = "\t.short\t";
  Data32bitsDirective = "\t.long\t";
  Data64bitsDirective = nullptr;  // 64bit data is not supported for now.
  ZeroDirective = "\t.space\t";
  /// comment character used by the assembler
  CommentString = "#";
  CodePointerSize = CalleeSaveStackSlotSize = 4;

  IsLittleEndian = true;
  /// True if target stack grow up
  StackGrowsUp = false;
  /// The '$' token refers to the current PC when not referencing an identifier
  DollarIsPC = false;
  LabelSuffix =":";

}

