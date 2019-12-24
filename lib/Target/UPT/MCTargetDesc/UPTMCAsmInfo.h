//===-- UPTMCAsmInfo.h - UPT asm properties --------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the UPTMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef UPTTARGETASMINFO_H
#define UPTTARGETASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
class Triple;

class UPTMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit UPTMCAsmInfo(const Triple &TT);
};

} // namespace llvm

#endif
