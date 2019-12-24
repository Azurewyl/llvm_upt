//===-- UPTAsmBackend.cpp - UPT Assembler Backend -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#define  DEBUG_TYPE "asm-backend"

#include "MCTargetDesc/UPTFixupKinds.h"
#include "MCTargetDesc/UPTMCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCMachObjectWriter.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

static unsigned adjustFixupValue(const MCFixup &Fixup, uint64_t Value) {
  unsigned Kind = Fixup.getKind();
  switch (Kind) {
  default:
    break;
  case UPT::fixup_upt_mov_hi16_pcrel:
    Value >>= 16;
    // Intentional fall-through
  case UPT::fixup_upt_mov_lo16_pcrel:
    unsigned Hi4 = (Value & 0xF000) >> 12;
    unsigned Lo12 = Value & 0x0FFF;
    // inst{19-16} = Hi4;
    // inst{11-0} = Lo12;
    Value = (Hi4 << 16) | (Lo12);
  }
  return Value;
}

namespace llvm{
class UPTELFObjectWriter : public MCELFObjectTargetWriter {
public:
  UPTELFObjectWriter(uint8_t OSABI)
      : MCELFObjectTargetWriter(/*Is64Bit*/ false, OSABI,
      /*ELF::EM_UPT*/ ELF::EM_ARM,
      /*HasRelocationAddend*/ false) {}
};

class UPTAsmBackend : public MCAsmBackend {
  Triple::OSType OSType;

public:
  UPTAsmBackend(Triple::OSType OSType)
  : MCAsmBackend(support::little), OSType(OSType) {}
  ~UPTAsmBackend() {}

  unsigned getNumFixupKinds() const override {
    return UPT::NumTargetFixupKinds;
  }

  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override {
    const static MCFixupKindInfo Infos[UPT::NumTargetFixupKinds] = {
        // This table *must* be in the order that the fixup_* kinds are defined
        // in
        // UPTFixupKinds.h.
        //
        // Name                      Offset (bits) Size (bits)     Flags
        {"fixup_upt_mov_hi16_pcrel", 0, 32, MCFixupKindInfo::FKF_IsPCRel},
        {"fixup_upt_mov_lo16_pcrel", 0, 32, MCFixupKindInfo::FKF_IsPCRel},
    };

    if (Kind < FirstTargetFixupKind) {
      return MCAsmBackend::getFixupKindInfo(Kind);
    }

    assert(unsigned(Kind
               -FirstTargetFixupKind) < getNumFixupKinds() &&
               "Invalid kind!");
    return Infos[Kind - FirstTargetFixupKind];
  }

  /// Hook to check if a relocation is needed for some target specific reason.
  virtual bool shouldForceRelocation(const MCAssembler &Asm,
                                     const MCFixup &Fixup,
                                     const MCValue &Target) override {
    // At this point we'll ignore the value returned by adjustFixupValue as
    // we are only checking if the fixup can be applied correctly.
    (void) adjustFixupValue(Fixup, Target.getConstant());
    // We always have resolved fixups for now.
    // TODO: JKB: when to force relocations?
    return false;
  }

  void applyFixup(const MCAssembler &Asm,
                  const MCFixup &Fixup,
                  const MCValue &Target,
                  MutableArrayRef<char> Data,
                  uint64_t Value,
                  bool IsResolved,
                  const MCSubtargetInfo *STI) const override {
    unsigned NumBytes = 4;
    Value = adjustFixupValue(Fixup, Value);
    if (!Value) {
      return; // Doesn't change encoding.
    }

    unsigned Offset = Fixup.getOffset();

    // For each byte of the fragment that the fixup touches, mask in the bits from
    // the fixup value. The Value has been "split up" into the appropriate
    // bitfields above.
    for (unsigned i = 0; i != NumBytes; ++i) {
      Data[Offset + i] |= uint8_t((Value >> (i * 8)) & 0xff);
    }
  }
  bool mayNeedRelaxation(const MCInst &Inst, const MCSubtargetInfo &STI) const override {
    return false;
  }
  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const override {
    return false;
  }

  void relaxInstruction(const MCInst &Inst, const MCSubtargetInfo &STI,
                        MCInst &Res) const override {}

  bool writeNopData(raw_ostream &OS, uint64_t Count) const override {
    if (Count == 0) {
      return true;
    }
    return false;
  }
  unsigned getPointerSize() const { return 4; }
  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(OSType);
    return createUPTELFObjectWriter(OSABI);
  }
};
}

MCAsmBackend *llvm::createUPTAsmBackend(const Target &T,
                                        const MCSubtargetInfo &STI,
                                        const MCRegisterInfo &MRI,
                                        const MCTargetOptions &Options) {
  return new UPTAsmBackend(STI.getTargetTriple().getOS());
}