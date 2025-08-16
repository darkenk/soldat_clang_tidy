#include "pch.hpp"

using namespace clang;
using namespace clang::tidy;
using namespace clang::ast_matchers;
using namespace clang::transformer;
using namespace clang::tidy::utils;

namespace {

class SoldatMoveGlobalVariablesToPrivateMembers : public TransformerClangTidyCheck {
    static auto rule() {
        // clang-format off
        auto matcher = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
          varDecl(
            hasAncestor(
                translationUnitDecl(
                    hasDescendant(
                        varDecl(
                            matchesName("gGlobalState.*"),
                            isDefinition(),
                            hasType(cxxRecordDecl(
                              has(accessSpecDecl(isPrivate()).bind("private"))
                            ).bind("class"))
                        )
                    )
                )
            ),
            unless(matchesName("gGlobalState.*")),
            unless(isConstexpr()),
            isDefinition(),
            isExpansionInMainFile(),
            hasTypeLoc(typeLoc().bind("var_type")),
            hasGlobalStorage(),
            optionally(hasInitializer(expr().bind("initializer")))
          ).bind("global_var")
        );

        return applyFirst(
          {
            makeRule(matcher,
              flatten(
                remove(node("global_var")),
                ifBound("initializer",
                    insertAfter(node("private"), cat(enclose(node("var_type"), after(node("global_var"))))),
                    insertAfter(node("private"), cat(enclose(node("var_type"), after(name("global_var"))), "{};"))
                )
              ),
              cat("Move variable ", name("global_var"), " into ", name("class"))
            )
          }
        );
    }

public:
  SoldatMoveGlobalVariablesToPrivateMembers(StringRef Name, ClangTidyContext *Context)
      : TransformerClangTidyCheck(rule(), Name, Context) {}
};

class SoldatMoveGlobalVariablesToPrivateMembersModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override {
    Factories.registerCheck<SoldatMoveGlobalVariablesToPrivateMembers>("soldat-move-global-variables-to-private-members");
  }
};

} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<SoldatMoveGlobalVariablesToPrivateMembersModule>
    X("soldat-move-global-variables-to-private-members", "Adds the new replacer check.");