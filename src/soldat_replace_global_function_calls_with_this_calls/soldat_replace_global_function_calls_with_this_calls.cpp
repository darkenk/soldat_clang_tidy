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

class SoldatReplaceGlobalFunctionCallsWithThisCalls : public TransformerClangTidyCheck {
    static auto rule() {
        // clang-format off
        auto matcher = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            cxxMemberCallExpr(
                on(declRefExpr(
                    to(varDecl(
                        hasGlobalStorage(),
                        matchesName("gGlobalState.*"),
                        hasType(cxxRecordDecl().bind("class"))
                    ))
                ).bind("global_var")),
                hasAncestor(
                    cxxMethodDecl(ofClass(
                        recordDecl(equalsBoundNode("class"))
                    ))
                ),
                callee(
                    cxxMethodDecl(ofClass(
                        cxxRecordDecl(equalsBoundNode("class"))
                    ))
                )
            ).bind("call")
        );
        // clang-format on

        return makeRule(matcher,
            { remove(declWithDot("global_var")) },
            cat("Remove call on global object")
        );
    }

public:
  SoldatReplaceGlobalFunctionCallsWithThisCalls(StringRef Name, ClangTidyContext *Context)
      : TransformerClangTidyCheck(rule(), Name, Context) {}
};

class SoldatReplaceGlobalFunctionCallsWithThisCallsModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override {
    Factories.registerCheck<SoldatReplaceGlobalFunctionCallsWithThisCalls>("soldat-replace-global-function-calls-with-this-calls");
  }
};

} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<SoldatReplaceGlobalFunctionCallsWithThisCallsModule>
    X("soldat-replace-global-function-calls-with-this-calls", "Adds the new replacer check.");