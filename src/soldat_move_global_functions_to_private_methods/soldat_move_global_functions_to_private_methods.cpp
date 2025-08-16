#include "pch.hpp"

using namespace clang;
using namespace clang::tidy;
using namespace clang::ast_matchers;
using namespace clang::transformer;
using namespace clang::tidy::utils;

namespace {

RangeSelector endOfClass(std::string BeginID, int Offset)
{
    return [=](const ast_matchers::MatchFinder::MatchResult &Result) {
        const auto *CRD = Result.Nodes.getNodeAs<CXXRecordDecl>(BeginID);

        auto InsertLoc = Lexer::getLocForEndOfToken(CRD->getEndLoc().getLocWithOffset(Offset), 0,
                                           *Result.SourceManager,
                                           Result.Context->getLangOpts());
        return CharSourceRange::getTokenRange(InsertLoc);
    };
}

class SoldatMoveGlobalFunctionsToPrivateMethods : public TransformerClangTidyCheck {
    static auto rule() {
        // clang-format off
        auto MatcherToFindClassWithoutPrivate = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
          varDecl(
              matchesName("gGlobalState.*"),
              isDefinition(),
              hasType(
                cxxRecordDecl(
                  unless(has(accessSpecDecl(isPrivate())))
                ).bind("class")
              )
          )
        );
        auto matcher = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
          functionDecl(
            hasAncestor(
                translationUnitDecl(
                    hasDescendant(
                        varDecl(
                            matchesName("gGlobalState.*"),
                            isDefinition(),
                            hasType(cxxRecordDecl(
                              optionally(has(accessSpecDecl(isPrivate()).bind("private")))
                            ).bind("class"))
                        )
                    )
                )
            ),
            unless(cxxMethodDecl()),
            unless(hasName("main")),
            isDefinition(),
            isExpansionInMainFile(),
            has(compoundStmt().bind("function_body"))
          ).bind("global_function")
        );

        return applyFirst(
          {
            makeRule(MatcherToFindClassWithoutPrivate, 
              { insertBefore(endOfClass("class", -2), cat("private:"))},
              cat("Insert private section into ", name("class"))
            ),
            makeRule(matcher,
              flatten(
                insertBefore(name("global_function"), cat(name("class"), "::")),
                insertAfter(endOfClass("class", -1), cat(enclose(node("global_function"), before(node("function_body"))), ";"))
              ),
              cat("Move function ", name("global_function"), " into ", name("class"))
            )
          }
        );
    }

public:
  SoldatMoveGlobalFunctionsToPrivateMethods(StringRef Name, ClangTidyContext *Context)
      : TransformerClangTidyCheck(rule(), Name, Context) {}
};

class SoldatMoveGlobalFunctionsToPrivateMethodsModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override {
    Factories.registerCheck<SoldatMoveGlobalFunctionsToPrivateMethods>("soldat-move-global-functions-to-private-methods");
  }
};

} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<SoldatMoveGlobalFunctionsToPrivateMethodsModule>
    X("soldat-move-global-functions-to-private-methods", "Adds the new replacer check.");