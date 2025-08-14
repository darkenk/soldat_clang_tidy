#include "pch.hpp"

using namespace clang;
using namespace clang::tidy;
using namespace clang::ast_matchers;
using namespace clang::transformer;
using namespace clang::tidy::utils;

namespace {

RangeSelector declWithDot(std::string BeginID)
{
    return [=](const ast_matchers::MatchFinder::MatchResult &Result) {
        const auto *Var = Result.Nodes.getNodeAs<DeclRefExpr>(BeginID);

        auto &SM = *Result.SourceManager;
        const LangOptions &LangOpts = Result.Context->getLangOpts();

        auto Begin = Var->getBeginLoc();
        auto End = Var->getEndLoc();

        End = Lexer::getLocForEndOfToken(End, 0, SM, LangOpts);
        return CharSourceRange::getTokenRange(Begin, End);
    };
}

class SoldatReplaceGlobalMembersWithThis : public TransformerClangTidyCheck {
    static auto rule() {
        // clang-format off
        auto matcher = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            memberExpr(
              unless(member(cxxMethodDecl())),
              hasObjectExpression(
                  declRefExpr(
                      to(varDecl(
                          hasGlobalStorage(),
                          matchesName("gGlobalState.*"),
                          hasType(cxxRecordDecl().bind("class"))
                      ))
                  ).bind("global_var")
              ),
              hasAncestor(
                  cxxMethodDecl(ofClass(
                      recordDecl(equalsBoundNode("class"))
                  ))
              )
          )
        );
        // clang-format on

        return makeRule(matcher,
            { remove(declWithDot("global_var")) },
            cat("Remove call on global object")
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