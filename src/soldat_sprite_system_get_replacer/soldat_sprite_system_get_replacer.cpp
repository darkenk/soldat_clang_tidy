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

class SoldatSpriteSystemGetReplacer : public TransformerClangTidyCheck {
    static auto rule() {
        auto sprite_system_get = callExpr(
            callee(
                cxxMethodDecl(
                    hasName("Get"),
                    ofClass(
                        hasName("GlobalSubsystem")
                    ),
                    returns(
                        references(
                            cxxRecordDecl(
                                hasName("TSpriteSystem")
                            )
                        )
                    )
                )
            ),
            hasAncestor(
                functionDecl(
                    has(
                        compoundStmt(
                            has(
                                stmt(unless(nullStmt())).bind("insert_sprite_system")
                            )
                        )
                    )
                ).bind("func_body")
            ),
            unless(
                hasAncestor(
                    varDecl(
                        hasType(
                            references(
                                cxxRecordDecl(
                                    hasName("TSpriteSystem")
                                )
                            )
                        )
                    )
                )
            )
        ).bind("sprite_system_get");

        auto local_sprite_system = varDecl(
            hasName("sprite_system"),
            hasAutomaticStorageDuration(),
            hasType(
                references(
                    cxxRecordDecl(
                        hasName("TSpriteSystem")
                    )
                )
            )
        );

        auto local_sprite_system_get = callExpr(
            callee(
                cxxMethodDecl(
                    hasName("Get"),
                    ofClass(
                        hasName("GlobalSubsystem")
                    ),
                    returns(
                        references(
                            cxxRecordDecl(
                                hasName("TSpriteSystem")
                            )
                        )
                    )
                )
            ),
            hasAncestor(functionDecl(hasDescendant(local_sprite_system))),
            unless(
                hasAncestor(
                    varDecl(
                        hasType(
                            references(
                                cxxRecordDecl(
                                    hasName("TSpriteSystem")
                                )
                            )
                        )
                    )
                )
            )
        ).bind("sprite_system_get");

        auto only_sprite_system_get = callExpr(
            callee(
                cxxMethodDecl(
                    hasName("Get"),
                    ofClass(
                        hasName("GlobalSubsystem")
                    ),
                    returns(
                        references(
                            cxxRecordDecl(
                                hasName("TSpriteSystem")
                            )
                        )
                    )
                )
            ),
            unless(
                hasAncestor(
                    varDecl(
                        hasType(
                            references(
                                cxxRecordDecl(
                                    hasName("TSpriteSystem")
                                )
                            )
                        )
                    )
                )
            )
        ).bind("sprite_system_get");

        auto matcher_sprite_system_get = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            sprite_system_get
        );

        auto matcher_local_sprite_system_get = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            local_sprite_system_get
        );

        /// auto InlineX =
///     makeRule(declRefExpr(to(varDecl(hasName("x")))), changeTo(cat("3")));
/// makeRule(functionDecl(hasName("f"), hasBody(stmt().bind("body"))).bind("f"),
///          flatten(
///            changeTo(name("f"), cat("newName")),
///            rewriteDescendants("body", InlineX)));

        auto r = applyFirst({
            makeRule(matcher_local_sprite_system_get,
                {
                    change(node("sprite_system_get"), cat("sprite_system"))
                },
                cat("Changed node ", node("sprite_system_get"), " to sprite_system")
            ),        
            makeRule(matcher_sprite_system_get,
                flatten(
                    insertBefore(node("insert_sprite_system"), cat("auto& sprite_system = SpriteSystem::Get();")),
                    rewriteDescendants("func_body", makeRule(only_sprite_system_get,
                                                        change(node("sprite_system_get"), cat("sprite_system"))
                                                    ))
                ),
                cat("Changed node ", node("sprite_system_get"), " to sprite_system")
            )
        });
        return r;
    }

public:
  SoldatSpriteSystemGetReplacer(StringRef Name, ClangTidyContext *Context)
      : TransformerClangTidyCheck(rule(), Name, Context) {}
};

class SoldatSpriteSystemGetReplacerModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override {
    Factories.registerCheck<SoldatSpriteSystemGetReplacer>("soldat-sprite-system-get-replacer");
  }
};

} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<SoldatSpriteSystemGetReplacerModule>
    X("soldat-sprite-system-get-replacer", "Adds the new replacer check.");