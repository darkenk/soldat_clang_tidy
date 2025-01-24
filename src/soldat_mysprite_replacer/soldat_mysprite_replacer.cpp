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

class MySpriteReplacer : public TransformerClangTidyCheck {
    static auto rule() {
        // Match calls to GetSprite(mysprite).
        auto MySpriteMatcher = cxxMemberCallExpr(
            on(expr(hasType(namedDecl(hasName("TSpriteSystem"))))),
            callee(functionDecl(hasName("GetSprite"))),
            callee(memberExpr().bind("func_call")),
            argumentCountIs(1),
            hasArgument(0, declRefExpr(to(varDecl(hasName("mysprite"), hasGlobalStorage(), hasType(isInteger())))))
        ).bind("callGetSprite");

        // Rewrite call GetSprite(mysprite) to GetMySprite().
        return makeRule(MySpriteMatcher,
                        {change(member("func_call"), cat("GetMySprite")), 
                         change(callArgs("callGetSprite"), cat(""))},
                        cat("Replace GetSprite(mysprite) with GetMySprite()."));
    }

public:
  MySpriteReplacer(StringRef Name, ClangTidyContext *Context)
      : TransformerClangTidyCheck(rule(), Name, Context) {}
};

class MySpriteReplacerModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override {
    Factories.registerCheck<MySpriteReplacer>("soldat-mysprite-replacer");
  }
};

} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<MySpriteReplacerModule>
    X("mysprite-replacer-module", "Adds the mysprite replacer check.");