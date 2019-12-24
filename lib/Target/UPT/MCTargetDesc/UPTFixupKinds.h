//===-- UPTFixupKinds.h - UPT-Specific Fixup Entries ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_UPTFIXUPKINDS_H
#define LLVM_UPTFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace UPT {
enum Fixups {

    // Although most of the current fixup types reflect a unique relocation
    // one can have multiple fixup types for a given relocation and thus need
    // to be uniquely named.
    //
    // This table *must* be in the save order of
    // MCFixupKindInfo Infos[Sample::NumTargetFixupKinds]
    // in SampleAsmBackend.cpp.
  fixup_upt_mov_hi16_pcrel = FirstTargetFixupKind,
  fixup_upt_mov_lo16_pcrel,
  // Marker
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};
}
}

#endif

