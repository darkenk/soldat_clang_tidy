#include "pch.hpp"

using namespace clang;
using namespace clang::tidy;
using namespace clang::ast_matchers;
using namespace clang::transformer;
using namespace clang::tidy::utils;

namespace {

class SoldatSwitchCaseToCommand : public TransformerClangTidyCheck {
    static auto rule() {
        // clang-format off
        auto matcher = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            caseStmt(
                hasAncestor(
                    cxxMethodDecl(
                        hasName("HandleMessages"),
                        hasAncestor(
                            translationUnitDecl(
                                has(cxxConstructorDecl(
                                    isDefinition(),
                                    hasName("NetworkClientImpl"),
                                    hasBody(compoundStmt().bind("ctor"))
                                ))
                            )
                        )
                    )
                ),
                has(declRefExpr().bind("msgid")),
                has(callExpr(
                  callee(functionDecl().bind("handler"))
                ))
            ).bind("case_statement")
        );
        // clang-format on

        return makeRule(matcher,
            { insertBefore(statements("ctor"), cat("RegisterMsgHandler(", node("msgid"), ", ", name("handler"), ");")),
              remove(node("case_statement"))
            },
            cat("Added register function for ", node("msgid"), " ", name("handler"))
        );
    }

public:
  SoldatSwitchCaseToCommand(StringRef Name, ClangTidyContext *Context)
      : TransformerClangTidyCheck(rule(), Name, Context) {}
};

class SoldatSwitchCaseToCommandModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override {
    Factories.registerCheck<SoldatSwitchCaseToCommand>("soldat-switch-case-to-command");
  }
};

} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<SoldatSwitchCaseToCommandModule>
    X("soldat-switch-case-to-command", "Adds the new replacer check.");
