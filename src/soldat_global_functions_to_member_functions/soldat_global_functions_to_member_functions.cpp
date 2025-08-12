#include "pch.hpp"

using namespace clang;
using namespace clang::tidy;
using namespace clang::ast_matchers;
using namespace clang::transformer;
using namespace clang::tidy::utils;

namespace {

class SoldatGlobalFunctionsToMemberFunctions : public ClangTidyCheck {
public:
  SoldatGlobalFunctionsToMemberFunctions(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}

  void registerMatchers(MatchFinder *Finder) override {
    Finder->addMatcher(varDecl(isExpansionInMainFile()).bind("var"), this);
  }

  void check(const MatchFinder::MatchResult &Result) override {
    if (const auto *VD = Result.Nodes.getNodeAs<VarDecl>("var")) {
      diag(VD->getLocation(), "Found variable declaration: ")
        << VD->getNameAsString();
    }
  }
};

class SoldatGlobalFunctionsToMemberFunctionsModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override {
    Factories.registerCheck<SoldatGlobalFunctionsToMemberFunctions>("soldat-global-functions-to-member-functions");
  }
};

} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<SoldatGlobalFunctionsToMemberFunctionsModule>
    X("soldat-global-functions-to-member-functions", "Adds the new replacer check.");