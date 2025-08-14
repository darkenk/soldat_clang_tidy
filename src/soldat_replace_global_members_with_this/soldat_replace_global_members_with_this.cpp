#include "pch.hpp"

using namespace clang;
using namespace clang::tidy;
using namespace clang::ast_matchers;
using namespace clang::transformer;
using namespace clang::tidy::utils;

namespace {

class SoldatReplaceGlobalMembersWithThis : public TransformerClangTidyCheck {
    static auto rule() {
        // clang-format off
        auto matcher = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            cxxMemberCallExpr()
        ).bind("bind_whole_expr");
        // clang-format on

        return makeRule(matcher,
            {change(node("bind_whole_expr"), cat("SampleReplacement"))},
            cat("Describe what has changed")
        );
    }

public:
  SoldatReplaceGlobalMembersWithThis(StringRef Name, ClangTidyContext *Context)
      : TransformerClangTidyCheck(rule(), Name, Context) {}
};

class SoldatReplaceGlobalMembersWithThisModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override {
    Factories.registerCheck<SoldatReplaceGlobalMembersWithThis>("soldat-replace-global-members-with-this");
  }
};

} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<SoldatReplaceGlobalMembersWithThisModule>
    X("soldat-replace-global-members-with-this", "Adds the new replacer check.");