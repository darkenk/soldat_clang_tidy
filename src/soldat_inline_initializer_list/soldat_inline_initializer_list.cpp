#include "pch.hpp"

using namespace clang;
using namespace clang::tidy;
using namespace clang::ast_matchers;
using namespace clang::transformer;
using namespace clang::tidy::utils;

namespace {

AST_MATCHER_P(DesignatedInitExpr, hasDesignedType, ast_matchers::internal::Matcher<FieldDecl>, InnerMatcher) {
    for (unsigned i = 0; i < Node.size(); ++i) {
        auto Designator = Node.getDesignator(i);
        if (!Designator->isFieldDesignator())
        {
            continue;
        }
        if (InnerMatcher.matches(*Designator->getFieldDecl(), Finder, Builder))
        {
            return true;
        }
    }
    return false;
}

RangeSelector designatedInitExprWithComma(std::string BeginID)
{
    return [=](const ast_matchers::MatchFinder::MatchResult &Result) {
        const auto *Var = Result.Nodes.getNodeAs<DesignatedInitExpr>(BeginID);

        auto &SM = *Result.SourceManager;
        const LangOptions &LangOpts = Result.Context->getLangOpts();

        auto Begin = Var->getBeginLoc();
        auto End = Var->getEndLoc();

        End = Lexer::getLocForEndOfToken(End, 0, SM, LangOpts);
        return CharSourceRange::getTokenRange(Begin, End);
    };
}

class SoldatInlineInitializerList : public TransformerClangTidyCheck {
    static auto rule() {
        // clang-format off
        auto matcher = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            designatedInitExpr(
                hasDesignedType(
                    fieldDecl().bind("class_member")
                ),
                has(
                    expr().bind("init")
                ),
                hasParent(
                    initListExpr(
                        hasParent(
                            varDecl(
                                hasType(
                                    cxxRecordDecl(
                                        matchesName("GlobalState.*"),
                                        has(
                                            fieldDecl(
                                                equalsBoundNode("class_member")
                                            )
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
            )
        ).bind("initializer");
        // clang-format on

        return makeRule(matcher,
            { remove(designatedInitExprWithComma("initializer")),
              changeTo(after(name("class_member")), cat(" = ", node("init"))),
            },
            cat("Inline initializer list")
        );
    }

public:
  SoldatInlineInitializerList(StringRef Name, ClangTidyContext *Context)
      : TransformerClangTidyCheck(rule(), Name, Context) {}
};

class SoldatInlineInitializerListModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override {
    Factories.registerCheck<SoldatInlineInitializerList>("soldat-inline-initializer-list");
  }
};

} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<SoldatInlineInitializerListModule>
    X("soldat-inline-initializer-list", "Adds the new replacer check.");