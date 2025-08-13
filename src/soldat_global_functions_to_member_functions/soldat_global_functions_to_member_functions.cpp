#include "pch.hpp"
#include <string_view>

using namespace clang;
using namespace clang::tidy;
using namespace clang::ast_matchers;
using namespace clang::transformer;
using namespace clang::tidy::utils;
using namespace std::string_view_literals;

namespace {

std::string getBaseName(const StringRef &path)
{
  llvm::SmallString<128> file(path);
  llvm::sys::path::replace_extension(file, "");
  return llvm::sys::path::filename(file).str();
}

std::string getFunctionSignature(const FunctionDecl* FD)
{
  clang::PrintingPolicy Policy(FD->getASTContext().getLangOpts());
  std::string Signature;
  llvm::raw_string_ostream OS(Signature);
  FD->print(OS, Policy);
  OS.flush();
  auto ExternKeyword = "extern "sv;
  if (Signature.starts_with(ExternKeyword))
  {
    Signature.erase(0, ExternKeyword.size());
  }
  return Signature;
}

class SoldatGlobalFunctionsToMemberFunctions : public ClangTidyCheck {
public:
  SoldatGlobalFunctionsToMemberFunctions(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}

  void registerMatchers(MatchFinder *Finder) override
  {
    // clang-format off
    Finder->addMatcher(traverse(TK_IgnoreUnlessSpelledInSource,
      functionDecl(
        unless(cxxMethodDecl()),
        unless(hasName("main"))
      ).bind("global_function")
    ), this);
    Finder->addMatcher(traverse(TK_IgnoreUnlessSpelledInSource,
      cxxRecordDecl(
        matchesName("GlobalState.*")
      ).bind("global_state_class_declaration")
    ), this);
    Finder->addMatcher(traverse(TK_IgnoreUnlessSpelledInSource,
      callExpr(
        callee(
          functionDecl(
            unless(cxxMethodDecl())
          )
        )
      ).bind("function_call")
    ), this);
    // clang-format on
  }

  void check(const MatchFinder::MatchResult &Result) override
  {
    if (const auto *GF = Result.Nodes.getNodeAs<FunctionDecl>("global_function"))
    {
      const clang::SourceManager &SM = GF->getASTContext().getSourceManager();
      clang::SourceLocation Loc = GF->getLocation();
      std::string Filename = SM.getFilename(Loc).str();
      if (!GF->isThisDeclarationADefinition()) 
      {
        mFunctionDeclarations[GF] = Filename;
      }
      else
      {
        mFunctionDefinitions[GF] = Filename;
      }
    }
    if (const auto *GS = Result.Nodes.getNodeAs<CXXRecordDecl>("global_state_class_declaration"))
    {
      mGlobalStateDeclarations[GS->getNameAsString()] = GS;
    }
    if (const auto *FC = Result.Nodes.getNodeAs<CallExpr>("function_call"))
    {
      mFunctionCalls.push_back(FC);
    }
  }

  void onEndOfTranslationUnit() override
  {
    auto GlobalStateName = "GlobalState" + getBaseName(getCurrentMainFile());

    auto it = mGlobalStateDeclarations.find(GlobalStateName);
    if (it != mGlobalStateDeclarations.end())
    {
      auto GlobalStateDecl = it->second;
      for (auto [FD, Filename] : mFunctionDeclarations)
      {
        if (getBaseName(Filename) != getBaseName(getCurrentMainFile()))
        {
          continue;
        }
        auto InsertLoc = GlobalStateDecl->getBraceRange().getBegin().getLocWithOffset(1);
        diag(FD->getLocation(), "add method declaration to " + GlobalStateName)
          << FixItHint::CreateInsertion(InsertLoc, getFunctionSignature(FD) + ";")
          << FixItHint::CreateRemoval(SourceRange(FD->getBeginLoc(), FD->getEndLoc().getLocWithOffset(1)));
      }
      for (auto [FD, Filename] : mFunctionDefinitions)
      {
        if (getBaseName(Filename) != getBaseName(getCurrentMainFile()))
        {
          continue;
        }
        auto it = std::find_if(mFunctionDeclarations.begin(), mFunctionDeclarations.end(), [&FD](const auto& v)
          {
            return v.first->getName() == FD->getName();
          }
        );
        if (it == mFunctionDeclarations.end())
        {
          continue;
        }
        diag(FD->getLocation(), "move global function to " + GlobalStateName)
          << FixItHint::CreateInsertion(FD->getLocation(), GlobalStateName + "::");
      }
    }
    for (auto FC : mFunctionCalls)
    {
      auto FunctionName = FC->getDirectCallee()->getNameAsString();
      auto it = std::find_if(mFunctionDeclarations.begin(), mFunctionDeclarations.end(), [&FunctionName](const auto& v)
        {
          return v.first->getNameAsString() == FunctionName;
        }
      );
      if (it == mFunctionDeclarations.end())
      {
        continue;
      }

      if (mGlobalStateDeclarations.find("GlobalState" + getBaseName(it->second)) == mGlobalStateDeclarations.end())
      {
        continue;
      }
      auto GlobalStateObj = "gGlobalState" + getBaseName(it->second);
      diag(FC->getBeginLoc(), "call method of " + GlobalStateObj)
        << FixItHint::CreateInsertion(FC->getBeginLoc(), GlobalStateObj + ".");
    }


  }
private:
  std::unordered_map<const FunctionDecl*, std::string> mFunctionDeclarations;
  std::unordered_map<const FunctionDecl*, std::string> mFunctionDefinitions;
  std::unordered_map<std::string, const CXXRecordDecl*> mGlobalStateDeclarations;
  std::vector<const CallExpr*> mFunctionCalls;
};

class SoldatGlobalFunctionsToMemberFunctionsModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override {
    Factories.registerCheck<SoldatGlobalFunctionsToMemberFunctions>("soldat-global-functions-to-member-functions");
  }
};

} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<SoldatGlobalFunctionsToMemberFunctionsModule>
    X("soldat-global-functions-to-member-functions", "Adds the new replacer check."); 