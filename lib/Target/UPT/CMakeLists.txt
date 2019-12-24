set(LLVM_TARGET_DEFINITIONS UPT.td)

tablegen(LLVM UPTGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM UPTGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM UPTGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM UPTGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM UPTGenCallingConv.inc -gen-callingconv)
tablegen(LLVM UPTGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM UPTGenMCCodeEmitter.inc -gen-emitter)
add_public_tablegen_target(UPTCommonTableGen)

add_llvm_target(UPTCodeGen
  UPTSubtarget.cpp
  UPTTargetMachine.cpp
  UPTRegisterInfo.cpp
  UPTInstrInfo.cpp
  UPTFrameLowering.cpp
  UPTISelLowering.cpp
  UPTISelDAGToDAG.cpp
  UPTAsmPrinter.cpp
  UPTMCInstLower.cpp
  )

add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
