//===-- UPT.td - Describe the UPT Target Machine -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This is the top level entry point for the UPT target.
// Target-independent interfaces which we are implementing
// Notice : Take care in the order of your code
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// UPT subtarget features and instruction predicates.
//===----------------------------------------------------------------------===//

// todo: make sure -> or . here
def FeatureStdExtM : SubtargetFeature<"m", "HasStdExtM", "true",
                   "Integer Multiplication and Division">;
def HasStdExtM : Predicate<"Subtarget.hasStdExtM()">,AssemblerPredicate<"FeatureStdExtM">;

// soft integer mul/div support
def FeatureSoftMulDiv : SubtargetFeature<"soft", "UseSoftMulDiv", "true",
                   "Use software emulation for integer multiply and divide">;
def UseSoftMulDiv : Predicate<"Subtarget.useSoftMulDiv()">,AssemblerPredicate<"FeatureSoftMulDiv">;

//===----------------------------------------------------------------------===//
// UPT processors supported.
//===----------------------------------------------------------------------===//

// ProcessorModel allows subtargets to specify the more general
// SchedMachineModel instead if a ProcessorItinerary.
// class ProcessorModel<string n, SchedMachineModel m, list<SubtargetFeature> f>
def : ProcessorModel<"generic-upt", NoSchedModel, []>;


//===----------------------------------------------------------------------===//
// Registers, calling conventions, instruction descriptions.
//===----------------------------------------------------------------------===//
include "UPTRegisterInfo.td"
include "UPTInstrInfo.td"
include "UPTCallingConv.td"

//===----------------------------------------------------------------------===//
// Define the target which we are implementing
//===----------------------------------------------------------------------===//
def UPTInstrInfo : InstrInfo;

def UPT : Target {
  let InstructionSet = UPTInstrInfo;
}


