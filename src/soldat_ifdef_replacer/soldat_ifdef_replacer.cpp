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
  SourceLocation mStart;
  SourceLocation mElse;
  SourceLocation mEnd;
  SourceLocation mMacroEnd;
  bool mIsIfndef;
};

struct IfdefPasser
{
  std::vector<IfdefLocation> &mIfdefLocations;
};

namespace
{
// gets the location of the # character in a #define or #ifdef directive
SourceLocation GetHashLoc(SourceLocation Loc) { return Loc.getLocWithOffset(-1); }

AST_MATCHER_P(FunctionDecl, hasIfdefs, IfdefPasser, Locations)
{

  SourceRange range = Node.getSourceRange();

  auto in_range = std::ranges::any_of(Locations.mIfdefLocations, [&](const auto &Ifdef) {
    return range.fullyContains(SourceRange(Ifdef.mStart, Ifdef.mEnd));
  });

  static constexpr auto kDebug = false;
  if constexpr (kDebug)
  {
    const SourceManager &sm = Finder->getASTContext().getSourceManager();
    llvm::errs() << "FunctionDecl SourceRange: " << range.getBegin().printToString(sm) << " - "
                 << range.getEnd().printToString(sm) << " hasIfdefs "
                 << (in_range ? "SUCCESS" : "FAIL") << "\n";
  }
  return in_range;
}
} // namespace

class IfdefPPCallbacks : public PPCallbacks
{
public:
  IfdefPPCallbacks(ClangTidyCheck &Check, const SourceManager &SM,
                   std::vector<IfdefLocation> &IfdefLocations)
    : mCheck(Check), mSm(SM), mIfdefLocations(IfdefLocations)
  {
  }

  void If(SourceLocation Loc, SourceRange ConditionRange,
          ConditionValueKind ConditionValue) override
  {
    mDirectiveStack.emplace_back();
  }

  void Ifdef(SourceLocation Loc, const Token &MacroNameTok, const MacroDefinition &MD) override
  {
    std::string macro_name = MacroNameTok.getIdentifierInfo()->getName().str();
    mDirectiveStack.emplace_back();
    if (macro_name == "SERVER")
    {
      mDirectiveStack.back().mLoc = Loc;
      mDirectiveStack.back().mMacroEndLoc = MacroNameTok.getEndLoc();
    }
  }

  void Ifndef(SourceLocation Loc, const Token &MacroNameTok, const MacroDefinition &MD) override
  {
    std::string macro_name = MacroNameTok.getIdentifierInfo()->getName().str();
    mDirectiveStack.emplace_back();
    if (macro_name == "SERVER")
    {
      mDirectiveStack.back().mLoc = Loc;
      mDirectiveStack.back().mMacroEndLoc = MacroNameTok.getEndLoc();
      mDirectiveStack.back().mIsIfndef = true;
    }
  }

  void Else(SourceLocation Loc, SourceLocation IfLoc) override
  {
    auto &current = mDirectiveStack.back();
    if (current.mLoc.isValid() && !current.mElseLoc.isValid())
    {
      current.mElseLoc = Loc;
    }
  }

  void Endif(SourceLocation Loc, SourceLocation IfLoc) override
  {
    if (mDirectiveStack.back().mLoc.isValid())
    { // if it's a SERVER macro
      mIfdefLocations.push_back({IfLoc, mDirectiveStack.back().mElseLoc, Loc,
                                 mDirectiveStack.back().mMacroEndLoc,
                                 mDirectiveStack.back().mIsIfndef});
    }
    mDirectiveStack.pop_back();
  }

private:
  struct DirectiveInfo
  {
    SourceLocation mLoc;
    SourceLocation mElseLoc;
    SourceLocation mMacroEndLoc;
    bool mIsIfndef = false;
  };
  [[maybe_unused]] ClangTidyCheck &mCheck;
  [[maybe_unused]] const SourceManager &mSm;
  std::vector<DirectiveInfo> mDirectiveStack; // Tracks ifdef/ifndef locations and if they're SERVER
  std::vector<IfdefLocation> &mIfdefLocations;
};

class SoldatMatchCallback : public MatchFinder::MatchCallback
{
public:
  explicit SoldatMatchCallback(ClangTidyCheck &Check, std::vector<IfdefLocation> &IfdefLocations)
    : mCheck(Check), mIfdefLocations(IfdefLocations)
  {
  }

  void run(const MatchFinder::MatchResult &Result) override
  {
    if (const auto *matched_decl = Result.Nodes.getNodeAs<FunctionDecl>("function_with_ifdef"))
    {
      matched_decl->dump();
      bool is_template =
        matched_decl->getDescribedTemplate() != nullptr ||
        matched_decl->getTemplateSpecializationInfo() != nullptr ||
        matched_decl->getMemberSpecializationInfo() != nullptr ||
        ((dyn_cast<CXXRecordDecl>(matched_decl->getParent()) != nullptr) &&
         dyn_cast<CXXRecordDecl>(matched_decl->getParent())->getDescribedClassTemplate() != nullptr);
      if (!is_template)
      {
        std::string template_prefix = "template<Config::Module M>\n";
        mCheck.diag(matched_decl->getLocation(), "converting to template function")
          << FixItHint::CreateInsertion(matched_decl->getBeginLoc(), template_prefix);
      }
      for (const auto &ifdef : mIfdefLocations)
      {
        if (!matched_decl->getSourceRange().fullyContains(SourceRange(ifdef.mStart, ifdef.mEnd)))
        {
          continue;
        }
        if (ifdef.mIsIfndef)
        {
          std::string replacement_text = "if constexpr (!Config::IsServer(M)) {";
          mCheck.diag(ifdef.mStart, "replacing #ifndef with if constexpr")
            << FixItHint::CreateReplacement(SourceRange(GetHashLoc(ifdef.mStart), ifdef.mMacroEnd),
                                            replacement_text);
        }
        else
        {
          std::string replacement_text = "if constexpr (Config::IsServer(M)) {";
          mCheck.diag(ifdef.mStart, "replacing #ifdef with if constexpr")
            << FixItHint::CreateReplacement(SourceRange(GetHashLoc(ifdef.mStart), ifdef.mMacroEnd),
                                            replacement_text);
        }
        if (ifdef.mElse.isValid())
        {
          mCheck.diag(ifdef.mElse, "replacing #else with } else {") << FixItHint::CreateReplacement(
            SourceRange(GetHashLoc(ifdef.mElse), ifdef.mElse), "} else {");
        }

        mCheck.diag(ifdef.mEnd, "replacing #endif with closing brace")
          << FixItHint::CreateReplacement(SourceRange(GetHashLoc(ifdef.mEnd), ifdef.mEnd), "}");
      }
    }
  }

private:
  ClangTidyCheck &mCheck;
  std::vector<IfdefLocation> &mIfdefLocations;
};

class SoldatIfdefReplacer : public ClangTidyCheck
{
public:
  SoldatIfdefReplacer(StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context), mCallback(*this, mIfdefLocations)
  {
  }

  void registerPPCallbacks(const SourceManager &SM, Preprocessor *PP,
                           Preprocessor *ModuleExpanderPP) override
  {
    PP->addPPCallbacks(std::make_unique<IfdefPPCallbacks>(*this, SM, mIfdefLocations));
  }

  void registerMatchers(MatchFinder *Finder) override
  {
    auto has_ifdef_matcher =
      functionDecl(hasIfdefs(IfdefPasser{mIfdefLocations})).bind("function_with_ifdef");

    Finder->addMatcher(has_ifdef_matcher, &mCallback);
  }

private:
  SoldatMatchCallback mCallback;
  std::vector<IfdefLocation> mIfdefLocations;
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
static ClangTidyModuleRegistry::Add<::SoldatIfdefReplacerModule> gSoldatIfdefReplacerInit(
  "soldat-ifdef-replacer-module", "Adds 'soldat-ifdef-replacer' checks.");

// This anchor is used to force the linker to link in the generated object file and thus register
// the module.
volatile int gAwesomePrefixCheckAnchorSource = 0;

} // namespace clang::tidy
