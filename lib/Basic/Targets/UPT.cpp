//===--- UPT.cpp - Implement UPT target feature support ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements UPT TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "UPT.h"
#include "clang/Basic/MacroBuilder.h"
#include "llvm/ADT/StringSwitch.h"

using namespace clang;
using namespace clang::targets;

ArrayRef<const char *> UPTTargetInfo::getGCCRegNames() const {
  return None;
}

ArrayRef<TargetInfo::GCCRegAlias> UPTTargetInfo::getGCCRegAliases() const {
  return None;
}



void UPTTargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
}

/// Return true if has this feature, need to sync with handleTargetFeatures.
bool UPTTargetInfo::hasFeature(StringRef Feature) const {
  return llvm::StringSwitch<bool>(Feature)
      .Case("upt", true)
      .Case("m", HasM)
      .Default(false);
}

/// Perform initialization based on the user configured set of features.
bool UPTTargetInfo::handleTargetFeatures(std::vector<std::string> &Features,
                                           DiagnosticsEngine &Diags) {
  for (const auto &Feature : Features) {
    if (Feature == "+m")
      HasM = true;
  }
  return true;
}
