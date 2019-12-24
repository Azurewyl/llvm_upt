//===-- UPTMachineFuctionInfo.h - UPT machine function info -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares UPT-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef UPTMACHINEFUNCTIONINFO_H
#define UPTMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

/// UPTFunctionInfo - This class is derived from MachineFunctionInfo
/// and contains private UPT target-specific information for each MachineFunction.
//  Mytodo support varargs here
class UPTFunctionInfo : public MachineFunctionInfo {
public:
  UPTFunctionInfo() = default;
};
} // End llvm namespace

#endif // UPTMACHINEFUNCTIONINFO_H

