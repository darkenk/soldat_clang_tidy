#include "clang-tidy/ClangTidy.h"
#include "clang-tidy/ClangTidyCheck.h"
#include "clang-tidy/ClangTidyModule.h"
#include "clang-tidy/ClangTidyModuleRegistry.h"
#include "clang-tidy/utils/TransformerClangTidyCheck.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Transformer/RewriteRule.h"
#include "clang/Tooling/Transformer/Stencil.h"
#include "clang/Tooling/Transformer/Transformer.h"

using namespace clang;
using namespace clang::tidy;
using namespace clang::ast_matchers;
using namespace clang::transformer;
using namespace clang::tidy::utils;

namespace {

class SoldatMySpriteIsValidReplacer : public TransformerClangTidyCheck {
    static auto rule() {
        auto mysprite_expr = declRefExpr(to(varDecl(hasName("mysprite"), hasGlobalStorage(), hasType(isUnsignedInteger()))));
        auto matcher_less = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            binaryOperator(hasLHS(mysprite_expr), hasOperatorName("<"), hasRHS(integerLiteral(equals(1)))).bind("invalid_player")
        );
        auto matcher_greater = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            binaryOperator(hasLHS(mysprite_expr), hasOperatorName(">"), hasRHS(integerLiteral(equals(0)))).bind("valid_player")
        );
        auto matcher_equal = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            binaryOperator(hasLHS(mysprite_expr), hasOperatorName("=="), hasRHS(integerLiteral(equals(0)))).bind("invalid_player")
        );
        auto r = applyFirst({
            makeRule(matcher_less,
                {change(node("invalid_player"), cat("!SpriteSystem::Get().IsPlayerSpriteValid()"))},
                cat("replace", node("invalid_player"), " with !SpriteSystem::Get().IsPlayerSpriteValid()")
            ),
            makeRule(matcher_greater,
                {change(node("valid_player"), cat("SpriteSystem::Get().IsPlayerSpriteValid()"))},
                cat("replace", node("valid_player"), " with SpriteSystem::Get().IsPlayerSpriteValid()")
            ),
            makeRule(matcher_equal,
                {change(node("invalid_player"), cat("!SpriteSystem::Get().IsPlayerSpriteValid()"))},
                cat("replace", node("invalid_player"), " with !SpriteSystem::Get().IsPlayerSpriteValid()")
            )
        });
        addInclude(r, "shared/mechanics/SpriteSystem.hpp");
        return r;
    }

public:
  SoldatMySpriteIsValidReplacer(StringRef Name, ClangTidyContext *Context)
      : TransformerClangTidyCheck(rule(), Name, Context) {}
};

class SoldatMySpriteIsValidReplacerModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override {
    Factories.registerCheck<SoldatMySpriteIsValidReplacer>("soldat-mysprite-is-valid-replacer");
  }
};

} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<SoldatMySpriteIsValidReplacerModule>
    X("soldat-mysprite-is-valid-replacer", "Replaces calls mysprite > 0 with IsPlayerSpriteValid()");