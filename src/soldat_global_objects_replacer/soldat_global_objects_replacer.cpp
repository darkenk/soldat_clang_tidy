#include "clang-tidy/ClangTidy.h"
#include "clang-tidy/ClangTidyCheck.h"
#include "clang-tidy/ClangTidyModule.h"
#include "clang-tidy/ClangTidyModuleRegistry.h"
#include "clang-tidy/utils/TransformerClangTidyCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/Transformer/RewriteRule.h"
#include "clang/Tooling/Transformer/Stencil.h"
#include "clang/Tooling/Transformer/Transformer.h"

using namespace clang;
using namespace clang::tidy;
using namespace clang::ast_matchers;
using namespace clang::transformer;
using namespace clang::tidy::utils;

namespace
{

static std::string getBaseName(const StringRef &path)
{
  llvm::SmallString<128> file(path);
  llvm::sys::path::replace_extension(file, "");
  return llvm::sys::path::filename(file).str();
}

class SoldatGlobalObjectsReplacer : public ClangTidyCheck
{
public:
  SoldatGlobalObjectsReplacer(StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context)
  {
  }

  void registerMatchers(MatchFinder *Finder) override
  {
    // clang-format off
    Finder->addMatcher(traverse(TK_IgnoreUnlessSpelledInSource,
      declRefExpr(
        to(
          varDecl(
            hasGlobalStorage(),
            unless(isStaticLocal()),
            unless(isStaticStorageClass())
          )
        )
      )).bind("global_var_usage"), this);
    Finder->addMatcher(traverse(TK_IgnoreUnlessSpelledInSource,
      varDecl(
        hasGlobalStorage(),
        unless(isStaticLocal()),
        unless(isStaticStorageClass()),
        unless(isDefinition())
      )).bind("global_var_decl"), this
    );
    Finder->addMatcher(traverse(TK_IgnoreUnlessSpelledInSource,
      varDecl(
        hasGlobalStorage(),
        unless(isStaticLocal()),
        unless(isStaticStorageClass()),
        isDefinition()
      )
    ).bind("global_var_def"), this);
    // clang-format on
  }

  void check(const MatchFinder::MatchResult &Result) override
  {
    if (const auto *VD = Result.Nodes.getNodeAs<clang::VarDecl>("global_var_decl"))
    {
      const clang::SourceManager &SM = VD->getASTContext().getSourceManager();
      clang::SourceLocation Loc = VD->getLocation();
      std::string Filename = SM.getFilename(Loc).str();
      if (!Filename.ends_with("Cvar.hpp"))
      {
        mGlobalVars[Filename].push_back(VD);
        if (Filename.ends_with(".hpp"))
        {
          mGlobalVarsLocation[VD->getNameAsString()] = Filename;
        }
      }
    }

    if (const auto *Usage = Result.Nodes.getNodeAs<clang::DeclRefExpr>("global_var_usage"))
    {
      mGlobalVarsUsage.push_back(Usage);
    }

    if (const auto *VD = Result.Nodes.getNodeAs<clang::VarDecl>("global_var_def"))
    {
      mGlobalVarsDefinition.push_back(VD);
    }

  }

  void onEndOfTranslationUnit() override
  {
    if (mGlobalVars.empty())
      return;
    
    for (auto& [filename, GlobalVars] : mGlobalVars)
    {
      if (filename.ends_with(".cpp"))
      {
        continue;
      }
      if (getBaseName(filename) != getBaseName(getCurrentMainFile()))
      {
        continue;
      }
      clang::ASTContext &Ctx = GlobalVars.front()->getASTContext();
      clang::PrintingPolicy Policy(Ctx.getLangOpts());
      clang::SourceManager &SM = Ctx.getSourceManager();
      auto StructName = "GlobalState" + getBaseName(filename);

      std::string StructText = "struct " + StructName + " {\n";
      for (const auto *VD : GlobalVars)
      {
        auto InitText = VD->hasInit()
                          ? clang::Lexer::getSourceText(
                              clang::CharSourceRange::getTokenRange(VD->getInit()->getSourceRange()),
                              SM, Ctx.getLangOpts())
                          : "";

        StructText += "  " + VD->getType().getAsString(Policy) + " " + VD->getNameAsString();
        if (!InitText.empty())
          StructText += " = " + InitText.str();
        StructText += ";\n";
      }

      StructText += "};\n\nextern " + StructName + " g" + StructName + ";\n";

      bool create_global_object = true;
      for (const auto *VD : GlobalVars)
      {
        // Get the end location after the semicolon
        SourceLocation EndLoc =
          clang::Lexer::getLocForEndOfToken(VD->getEndLoc(), 0, SM, Ctx.getLangOpts()).getLocWithOffset(1);
        clang::CharSourceRange FullRange =
          clang::CharSourceRange::getCharRange(VD->getBeginLoc(), EndLoc);
        if (create_global_object)
        {
          diag(VD->getBeginLoc(), "Remove original global variable")
            << clang::FixItHint::CreateRemoval(FullRange)
            << clang::FixItHint::CreateInsertion(VD->getBeginLoc(), StructText, true);
          create_global_object = false;
          continue;
        }
        diag(VD->getBeginLoc(), "Remove original global variable")
            << clang::FixItHint::CreateRemoval(FullRange);
      }
    }

    for (const auto *Usage : mGlobalVarsUsage)
    {
      auto it = mGlobalVarsLocation.find(Usage->getNameInfo().getAsString());
      if (it == mGlobalVarsLocation.end())
      {
        continue;
      }
      auto StructName = "gGlobalState" + getBaseName(StringRef(it->second));
      diag(Usage->getBeginLoc(), "Replace with global variable object")
        << clang::FixItHint::CreateReplacement(
            Usage->getBeginLoc(), StructName + "." + Usage->getNameInfo().getAsString());
    }

    bool put_global_state_definition = true;
    SourceLocation LastEntry = {};
    std::string StructText;
    for (const auto *Usage : mGlobalVarsDefinition)
    {
      auto it = mGlobalVarsLocation.find(Usage->getNameAsString());
      if (it == mGlobalVarsLocation.end())
      {
        continue;
      }
      const clang::SourceManager &SM = Usage->getASTContext().getSourceManager();
      const clang::LangOptions &LangOpts = Usage->getASTContext().getLangOpts();
      auto StructName = "gGlobalState" + getBaseName(StringRef(it->second));
      SourceLocation EndLoc =
          clang::Lexer::getLocForEndOfToken(Usage->getEndLoc(), 0, SM, LangOpts).getLocWithOffset(1);
      SourceRange FullRange = {Usage->getBeginLoc(), EndLoc};
      auto d = diag(Usage->getBeginLoc(), "Replace definition with global variable object");
      if (put_global_state_definition)
      {
        put_global_state_definition = false;
        StructText = "GlobalState" + getBaseName(getCurrentMainFile()) + " gGlobalState" + getBaseName(getCurrentMainFile()) + "{\n";
        LastEntry = EndLoc;
      }
      bool DefaultInitializer = true;
      if (Usage->hasInit())
      {
        auto InitText = clang::Lexer::getSourceText(
          clang::CharSourceRange::getTokenRange(Usage->getInit()->getSourceRange()),
          SM, LangOpts
        );
        if (InitText != Usage->getNameAsString())
        {
          DefaultInitializer = false;
          StructText += "." + Usage->getNameAsString() + " = " + InitText.str() + ",\n";
          LastEntry = FullRange.getEnd();
        }
      }
      if (DefaultInitializer)
      {
        StructText += "." + Usage->getNameAsString() + "{},\n";
      }
      d << clang::FixItHint::CreateRemoval(FullRange);

    }
    diag(LastEntry, "Replace definition with global variable object")
      << clang::FixItHint::CreateInsertion(LastEntry, StructText + "};\n");

    mGlobalVars.clear();
  }

private:
  std::unordered_map<std::string, std::vector<const clang::VarDecl *>> mGlobalVars;
  std::unordered_map<std::string, std::string> mGlobalVarsLocation;
  std::vector<const clang::DeclRefExpr *> mGlobalVarsUsage;
  std::vector<const clang::VarDecl *> mGlobalVarsDefinition;
};

class SoldatGlobalObjectsReplacerModule : public ClangTidyModule
{
public:
  void addCheckFactories(ClangTidyCheckFactories &Factories) override
  {
    Factories.registerCheck<SoldatGlobalObjectsReplacer>("soldat-global-objects-replacer");
  }
};

} // end anonymous namespace

// Register the module using this statically initialized variable
static ClangTidyModuleRegistry::Add<SoldatGlobalObjectsReplacerModule> X(
  "soldat-global-objects-replacer", "Adds the new replacer check.");