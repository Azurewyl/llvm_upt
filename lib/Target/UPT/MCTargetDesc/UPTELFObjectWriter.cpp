//===-- UPTELFObjectWriter.cpp - UPT ELF Writer ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/UPTFixupKinds.h"
#include "MCTargetDesc/UPTMCTargetDesc.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"


namespace llvm{
class UPTELFObjectWriter : public MCELFObjectTargetWriter {
public:
  UPTELFObjectWriter(uint8_t OSABI)
      : MCELFObjectTargetWriter(/*Is64Bit*/ false, OSABI, /*ELF::EM_UPT*/ ELF::EM_ARM,
      /*HasRelocationAddend*/ false) {}

  virtual ~UPTELFObjectWriter() = default;

  unsigned int getRelocType(MCContext &Ctx,
                            const MCValue &Target,
                            const MCFixup &Fixup,
                            bool IsPCRel) const override {
    if (!IsPCRel) {
      llvm_unreachable("Only dealying with PC-relative fixups for now");
    }

    unsigned Type = 0;
    switch ((unsigned) Fixup.getKind()) {
    default:llvm_unreachable("Unimplemented");
    case UPT::fixup_upt_mov_hi16_pcrel:Type = ELF::R_ARM_MOVT_PREL;
      break;
    case UPT::fixup_upt_mov_lo16_pcrel:Type = ELF::R_ARM_MOVW_PREL_NC;
      break;
    }
    return Type;
  }
};

std::unique_ptr<MCObjectTargetWriter> createUPTELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<UPTELFObjectWriter>(OSABI);
}
}
