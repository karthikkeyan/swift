//===--- DominanceAnalysis.h - SIL Dominance Analysis -*- C++ -*-----------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2015 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SILANALYSIS_DOMINANCEANALYSIS_H
#define SWIFT_SILANALYSIS_DOMINANCEANALYSIS_H

#include "swift/SIL/SILInstruction.h"
#include "swift/SIL/Dominance.h"
#include "swift/SILAnalysis/Analysis.h"
#include "llvm/ADT/DenseMap.h"

namespace swift {
class SILModule;
class SILValue;
class SILInstruction;

  class DominanceAnalysis : public SILAnalysis {
    llvm::DenseMap<SILFunction *, DominanceInfo*> DomInfo;
    llvm::DenseMap<SILFunction *, PostDominanceInfo*> PostDomInfo;

  public:
    virtual ~DominanceAnalysis() {
      // Delete Dominance Info.
      for (auto D : DomInfo)
        delete D.second;

      // Delete PostDominanceInfo.
      for (auto P : PostDomInfo)
        delete P.second;
    }

    DominanceAnalysis(SILModule *) : SILAnalysis(AnalysisKind::Dominance) {}

    DominanceInfo* getDomInfo(SILFunction *F) {
      if (!DomInfo.count(F))
        DomInfo[F] = new DominanceInfo(F);
      return DomInfo[F];
    }

    PostDominanceInfo* getPostDomInfo(SILFunction *F) {
      if (!PostDomInfo.count(F))
        PostDomInfo[F] = new PostDominanceInfo(F);
      return PostDomInfo[F];
    }

    static bool classof(const SILAnalysis *S) {
      return S->getKind() == AnalysisKind::Dominance;
    }

    virtual void invalidate(InvalidationKind K) {
      // FIXME: Invalidating the call graph should not invalidate the domtrees
      // of all functions.
      if (K >= InvalidationKind::CFG) {
        // Delete Dominance Info.
        for (auto D : DomInfo)
          delete D.second;

        // Delete PostDominanceInfo.
        for (auto P : PostDomInfo)
          delete P.second;

        // Clear the maps.
        DomInfo.clear();
        PostDomInfo.clear();
      }
    }

    virtual void invalidate(SILFunction* F, InvalidationKind K) {
      if (K >= InvalidationKind::CFG) {
        if (DomInfo.count(F)) {
          delete DomInfo[F];
          DomInfo.erase(F);
        }

        if (PostDomInfo.count(F)) {
          delete PostDomInfo[F];
          PostDomInfo.erase(F);
        }
      }
    }

    /// Update the dominance information with the passed analysis info.
    /// Takes ownership of the analysis info.
    void updateAnalysis(SILFunction *F,
                        std::unique_ptr<DominanceInfo> Info) {
      if (DomInfo.count(F)) {
        assert(DomInfo[F] != Info.get());
        delete DomInfo[F];
      }
      DomInfo[F] = Info.release();
    }

    /// Update the post dominance information with the passed analysis info.
    /// Takes ownership of the analysis info.
    void updateAnalysis(SILFunction *F,
                        std::unique_ptr<PostDominanceInfo> Info) {
      if (PostDomInfo.count(F)) {
        assert(PostDomInfo[F] != Info.get());
        delete PostDomInfo[F];
      }
      PostDomInfo[F] = Info.release();
    }

    /// Release ownership of the dominance information for the function. The
    /// returned unique_ptr takes ownership of the object.
    std::unique_ptr<DominanceInfo> preserveDomAnalysis(SILFunction *F) {
      assert(DomInfo.count(F));
      std::unique_ptr<DominanceInfo> Info(DomInfo[F]);
      DomInfo.erase(F);
      return Info;
    }

    /// Release ownership of the post-dominance information for the function.
    /// The returned unique_ptr takes ownership of the object.
    std::unique_ptr<PostDominanceInfo> preservePostDomAnalysis(SILFunction *F) {
      assert(PostDomInfo.count(F));
      std::unique_ptr<PostDominanceInfo> Info(PostDomInfo[F]);
      PostDomInfo.erase(F);
      return Info;
    }
  };
} // end namespace swift



#endif
