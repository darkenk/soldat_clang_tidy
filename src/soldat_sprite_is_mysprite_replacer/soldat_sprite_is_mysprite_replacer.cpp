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

class SoldatSpriteIsMyspriteReplacer : public TransformerClangTidyCheck {
    static auto rule() {
        auto mysprite = declRefExpr(to(varDecl(hasName("mysprite"), hasGlobalStorage(), hasType(isUnsignedInteger()))));
        auto other_sprite = declRefExpr(to(varDecl(hasType(isInteger())))).bind("other_sprite");
        auto other_member_sprite = memberExpr(member(hasType(isInteger()))).bind("other_sprite");
        
        auto matcher_is_player = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            binaryOperator(hasOperands(mysprite, other_sprite), hasOperatorName("==")).bind("is_player")
        );
        auto matcher_is_not_player = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            binaryOperator(hasOperands(mysprite, other_sprite), hasOperatorName("!=")).bind("is_not_player")
        );
        auto member_matcher_is_player = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            binaryOperator(hasOperands(mysprite, other_member_sprite), hasOperatorName("==")).bind("is_player")
        );
        auto member_matcher_is_not_player = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            binaryOperator(hasOperands(mysprite, other_member_sprite), hasOperatorName("!=")).bind("is_not_player")
        );


        auto r = applyFirst({
            makeRule(matcher_is_player,
                {change(node("is_player"), cat("SpriteSystem::Get().IsPlayerSprite(", node("other_sprite"), ")"))},
                cat("replace", node("is_player"), " with SpriteSystem::Get().IsPlayerSprite(", node("other_sprite"), ")")
            ),
            makeRule(matcher_is_not_player,
                {change(node("is_not_player"), cat("!SpriteSystem::Get().IsPlayerSprite(", node("other_sprite"), ")"))},
                cat("replace", node("is_not_player"), " with !SpriteSystem::Get().IsPlayerSprite(", node("other_sprite"), ")")
            ),
            makeRule(member_matcher_is_player,
                {change(node("is_player"), cat("SpriteSystem::Get().IsPlayerSprite(", node("other_sprite"), ")"))},
                cat("replace", node("is_player"), " with SpriteSystem::Get().IsPlayerSprite(", node("other_sprite"), ")")
            ),
            makeRule(member_matcher_is_not_player,
                {change(node("is_not_player"), cat("!SpriteSystem::Get().IsPlayerSprite(", node("other_sprite"), ")"))},
                cat("replace", node("is_not_player"), " with !SpriteSystem::Get().IsPlayerSprite(", node("other_sprite"), ")")
            ),
        });
        addInclude(r, "shared/mechanics/SpriteSystem.hpp");

        return r;
    }

public:
  SoldatSpriteIsMyspriteReplacer(StringRef Name, ClangTidyContext *Context)
      : TransformerClangTidyCheck(rule(), Name, Context) {}
};

class SoldatSpriteIsMyspriteReplacerModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override {
    Factories.registerCheck<SoldatSpriteIsMyspriteReplacer>("soldat-sprite-is-mysprite-replacer");
  }
};

} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<SoldatSpriteIsMyspriteReplacerModule>
    X("soldat-sprite-is-mysprite-replacer", "Adds the new replacer check.");