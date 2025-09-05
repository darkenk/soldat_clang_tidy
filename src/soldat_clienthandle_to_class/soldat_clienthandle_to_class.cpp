#include "pch.hpp"

using namespace clang;
using namespace clang::tidy;
using namespace clang::ast_matchers;
using namespace clang::transformer;
using namespace clang::tidy::utils;

namespace {

class SoldatClienthandleToClass : public TransformerClangTidyCheck {

    static auto rule() {
        // clang-format off
        auto matcher = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            functionDecl(
              isDefinition(),
              matchesName("clienthandle.*")
            ).bind("definition")
        );
        auto matcher2 = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            cxxMemberCallExpr(
                callee(cxxMethodDecl(hasName("RegisterMsgHandler"))),
                thisPointerType(
                    cxxRecordDecl(
                        hasName("NetworkClientImpl")
                    )
                ),
                hasArgument(1, declRefExpr().bind("arg"))
            ).bind("callexpr")
        );
        auto matcher3 = traverse(TraversalKind::TK_IgnoreUnlessSpelledInSource,
            functionDecl(
                matchesName("clienthandle.*"),
                unless(isDefinition())
            ).bind("declaration")
        );
        // clang-format on
        auto rule1 = makeRule(matcher, {change(name("definition"), cat(name("definition"), "::Handle"))}, cat("Replace definition"));
        auto rule2 = makeRule(matcher3, {change(node("declaration"), cat("class ", name("declaration"), " : public INetMessageHandler"
                " {public: void Handle(NetworkContext* nc) override; };"))}, cat("Replace declaration"));
        addInclude(rule2, "common/network/Net.hpp");
        auto rule3 = makeRule(matcher2, {change(node("arg"), cat("std::make_unique<", node("arg"), ">()"))}, cat("Replace call"));
        return applyFirst({rule1, rule2, rule3});
    }

public:
  SoldatClienthandleToClass(StringRef Name, ClangTidyContext *Context)
      : TransformerClangTidyCheck(rule(), Name, Context) {}
};

class SoldatClienthandleToClassModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override {
    Factories.registerCheck<SoldatClienthandleToClass>("soldat-clienthandle-to-class");
  }
};

} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<SoldatClienthandleToClassModule>
    X("soldat-clienthandle-to-class", "Adds the new replacer check.");
