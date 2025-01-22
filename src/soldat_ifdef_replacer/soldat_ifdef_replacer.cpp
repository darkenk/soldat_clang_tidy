#include "clang-tidy/ClangTidy.h"
#include "clang-tidy/ClangTidyCheck.h"
#include "clang-tidy/ClangTidyModule.h"
#include "clang-tidy/ClangTidyModuleRegistry.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/TypeLoc.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"

using namespace clang;
using namespace clang::tidy;
using namespace clang::ast_matchers;

struct IfdefLocation
{
  SourceLocation Start;
  SourceLocation Else;
  SourceLocation End;
  SourceLocation MacroEnd;
  bool isIfndef;
};

struct IfdefPasser
{
  std::vector<IfdefLocation> &IfdefLocations;
};

namespace
{
// gets the location of the # character in a #define or #ifdef directive
SourceLocation GetHashLoc(SourceLocation Loc) { return Loc.getLocWithOffset(-1); }

AST_MATCHER_P(FunctionDecl, hasIfdefs, IfdefPasser, Locations)
{

  SourceRange Range = Node.getSourceRange();

  auto inRange = std::ranges::any_of(Locations.IfdefLocations, [&](const auto &Ifdef) {
    return Range.fullyContains(SourceRange(Ifdef.Start, Ifdef.End));
  });

  static constexpr auto debug = false;
  if constexpr (debug)
  {
    const SourceManager &SM = Finder->getASTContext().getSourceManager();
    llvm::errs() << "FunctionDecl SourceRange: " << Range.getBegin().printToString(SM) << " - "
                 << Range.getEnd().printToString(SM) << " hasIfdefs "
                 << (inRange ? "SUCCESS" : "FAIL") << "\n";
  }
  return inRange;
}
} // namespace

class IfdefPPCallbacks : public PPCallbacks
{
public:
  IfdefPPCallbacks(ClangTidyCheck &Check, const SourceManager &SM,
                   std::vector<IfdefLocation> &IfdefLocations)
    : Check(Check), SM(SM), IfdefLocations(IfdefLocations)
  {
  }

  void If(SourceLocation Loc, SourceRange ConditionRange,
          ConditionValueKind ConditionValue) override
  {
    DirectiveStack.emplace_back();
  }

  void Ifdef(SourceLocation Loc, const Token &MacroNameTok, const MacroDefinition &MD) override
  {
    std::string MacroName = MacroNameTok.getIdentifierInfo()->getName().str();
    DirectiveStack.emplace_back();
    if (MacroName == "SERVER")
    {
      DirectiveStack.back().Loc = Loc;
      DirectiveStack.back().MacroEndLoc = MacroNameTok.getEndLoc();
    }
  }

  void Ifndef(SourceLocation Loc, const Token &MacroNameTok, const MacroDefinition &MD) override
  {
    std::string MacroName = MacroNameTok.getIdentifierInfo()->getName().str();
    DirectiveStack.emplace_back();
    if (MacroName == "SERVER")
    {
      DirectiveStack.back().Loc = Loc;
      DirectiveStack.back().MacroEndLoc = MacroNameTok.getEndLoc();
      DirectiveStack.back().isIfndef = true;
    }
  }

  void Else(SourceLocation Loc, SourceLocation IfLoc) override
  {
    auto &current = DirectiveStack.back();
    if (current.Loc.isValid() && !current.ElseLoc.isValid())
    {
      current.ElseLoc = Loc;
    }
  }

  void Endif(SourceLocation Loc, SourceLocation IfLoc) override
  {
    if (DirectiveStack.back().Loc.isValid())
    { // if it's a SERVER macro
      IfdefLocations.push_back({IfLoc, DirectiveStack.back().ElseLoc, Loc,
                                DirectiveStack.back().MacroEndLoc, DirectiveStack.back().isIfndef});
    }
    DirectiveStack.pop_back();
  }

private:
  struct DirectiveInfo
  {
    SourceLocation Loc;
    SourceLocation ElseLoc;
    SourceLocation MacroEndLoc;
    bool isIfndef = false;
  };
  [[maybe_unused]] ClangTidyCheck &Check;
  [[maybe_unused]] const SourceManager &SM;
  std::vector<DirectiveInfo> DirectiveStack; // Tracks ifdef/ifndef locations and if they're SERVER
  std::vector<IfdefLocation> &IfdefLocations;
};

class SoldatMatchCallback : public MatchFinder::MatchCallback
{
public:
  explicit SoldatMatchCallback(ClangTidyCheck &Check, std::vector<IfdefLocation> &IfdefLocations)
    : Check(Check), IfdefLocations(IfdefLocations)
  {
  }

  void run(const MatchFinder::MatchResult &Result) override
  {
    if (const auto *MatchedDecl = Result.Nodes.getNodeAs<FunctionDecl>("function_with_ifdef"))
    {
      MatchedDecl->dump();
      bool isTemplate =
        MatchedDecl->getDescribedTemplate() != nullptr ||
        MatchedDecl->getTemplateSpecializationInfo() != nullptr ||
        MatchedDecl->getMemberSpecializationInfo() != nullptr ||
        ((dyn_cast<CXXRecordDecl>(MatchedDecl->getParent()) != nullptr) &&
         dyn_cast<CXXRecordDecl>(MatchedDecl->getParent())->getDescribedClassTemplate() != nullptr);
      if (!isTemplate)
      {
        std::string TemplatePrefix = "template<Config::Module M>\n";
        Check.diag(MatchedDecl->getLocation(), "converting to template function")
          << FixItHint::CreateInsertion(MatchedDecl->getBeginLoc(), TemplatePrefix);
      }
      for (const auto &Ifdef : IfdefLocations)
      {
        if (!MatchedDecl->getSourceRange().fullyContains(SourceRange(Ifdef.Start, Ifdef.End)))
        {
          continue;
        }
        if (Ifdef.isIfndef)
        {
          std::string ReplacementText = "if constexpr (!Config::IsServer(M)) {";
          Check.diag(Ifdef.Start, "replacing #ifndef with if constexpr")
            << FixItHint::CreateReplacement(SourceRange(GetHashLoc(Ifdef.Start), Ifdef.MacroEnd),
                                            ReplacementText);
        }
        else
        {
          std::string ReplacementText = "if constexpr (Config::IsServer(M)) {";
          Check.diag(Ifdef.Start, "replacing #ifdef with if constexpr")
            << FixItHint::CreateReplacement(SourceRange(GetHashLoc(Ifdef.Start), Ifdef.MacroEnd),
                                            ReplacementText);
        }
        if (Ifdef.Else.isValid())
        {
          Check.diag(Ifdef.Else, "replacing #else with } else {") << FixItHint::CreateReplacement(
            SourceRange(GetHashLoc(Ifdef.Else), Ifdef.Else), "} else {");
        }

        Check.diag(Ifdef.End, "replacing #endif with closing brace")
          << FixItHint::CreateReplacement(SourceRange(GetHashLoc(Ifdef.End), Ifdef.End), "}");
      }
    }
  }

private:
  ClangTidyCheck &Check;
  std::vector<IfdefLocation> &IfdefLocations;
};

class SoldatIfdefReplacer : public ClangTidyCheck
{
public:
  SoldatIfdefReplacer(StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context), Callback(*this, IfdefLocations)
  {
  }

  void registerPPCallbacks(const SourceManager &SM, Preprocessor *PP,
                           Preprocessor *ModuleExpanderPP) override
  {
    PP->addPPCallbacks(std::make_unique<IfdefPPCallbacks>(*this, SM, IfdefLocations));
  }

  void registerMatchers(MatchFinder *Finder) override
  {
    auto HasIfdefMatcher =
      functionDecl(hasIfdefs(IfdefPasser{IfdefLocations})).bind("function_with_ifdef");

    Finder->addMatcher(HasIfdefMatcher, &Callback);
  }

private:
  SoldatMatchCallback Callback;
  std::vector<IfdefLocation> IfdefLocations;
};

namespace
{

class SoldatIfdefReplacerModule : public ClangTidyModule
{
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override
  {
    CheckFactories.registerCheck<SoldatIfdefReplacer>("soldat-ifdef-replacer");
  }
};

} // namespace

namespace clang::tidy
{

// Register the module using this statically initialized variable.
static ClangTidyModuleRegistry::Add<::SoldatIfdefReplacerModule> soldatIfdefReplacerInit(
  "soldat-ifdef-replacer-module", "Adds 'soldat-ifdef-replacer' checks.");

// This anchor is used to force the linker to link in the generated object file and thus register
// the module.
volatile int awesomePrefixCheckAnchorSource = 0;

} // namespace clang::tidy
