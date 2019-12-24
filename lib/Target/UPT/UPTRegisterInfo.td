//===-- UPTRegisterInfo.td - UPT Register defs ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the UPT register file
//===----------------------------------------------------------------------===//
// General register definition
class UPTReg<bits<5> Enc, string n> : Register<n> {
  let HWEncoding{4-0} = Enc;
  let Namespace = "UPT";
}
// General Purpose Registers
def ZERO : UPTReg< 0,  "ZERO">, DwarfRegNum<[0]>;
def V0   : UPTReg< 1,  "V0">,   DwarfRegNum<[1]>;
def A0   : UPTReg< 2,  "A0">,   DwarfRegNum<[2]>;
def A1   : UPTReg< 3,  "A1">,   DwarfRegNum<[3]>;
def A2   : UPTReg< 4,  "A2">,   DwarfRegNum<[4]>;
def A3   : UPTReg< 5,  "A3">,   DwarfRegNum<[5]>;
def T0   : UPTReg< 6,  "T0">,   DwarfRegNum<[6]>;
def T1   : UPTReg< 7,  "T1">,   DwarfRegNum<[7]>;
def T2   : UPTReg< 8,  "T2">,   DwarfRegNum<[8]>;
def T3   : UPTReg< 9,  "T3">,   DwarfRegNum<[9]>;
def S0   : UPTReg< 10, "S0">,   DwarfRegNum<[10]>;
def S1   : UPTReg< 11, "S1">,   DwarfRegNum<[11]>;
def S2   : UPTReg< 12, "S2">,   DwarfRegNum<[12]>;
def S3   : UPTReg< 13, "S3">,   DwarfRegNum<[13]>;
def SP   : UPTReg< 14, "SP">,   DwarfRegNum<[14]>;
def RA   : UPTReg< 15, "RA">,   DwarfRegNum<[15]>;
def SR   : UPTReg< 16, "SR">,   DwarfRegNum<[16]>;


//===----------------------------------------------------------------------===//
// Register Classes
//===----------------------------------------------------------------------===//
//class RegisterClass<string namespace, list<ValueType> regTypes, int alignment,
//                    dag regList, RegAltNameIndex idx = NoRegAltName>
// Also define the default allocation order ofregisters
def GRRegs : RegisterClass<"UPT", [i32], 32, (add
  // (Return values and Arguments registers)
  V0, A0, A1, A2, A3,
  //  (Not preserved across procedure calls)
  T0, T1, T2, T3,
  //  (Callee saved registers)
  S0, S1, S2, S3,
  // (Reserved registers)
  ZERO, SP, RA)>;
