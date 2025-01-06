#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang-tidy/ClangTidy.h"
#include "clang-tidy/ClangTidyCheck.h"
#include "clang-tidy/ClangTidyModule.h"
#include "clang-tidy/ClangTidyModuleRegistry.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/CompilerInstance.h"

using namespace clang;
using namespace clang::tidy;
using namespace clang::ast_matchers;

class SoldatIfdefReplacer : public ClangTidyCheck
{
public:
    SoldatIfdefReplacer(StringRef Name, ClangTidyContext* Context) : ClangTidyCheck(Name, Context)
    {
    }
    void registerPPCallbacks(const SourceManager &SM, Preprocessor *PP, Preprocessor *ModuleExpanderPP) override;
};

class IfdefPPCallbacks : public PPCallbacks
{
public:
    IfdefPPCallbacks(ClangTidyCheck &Check, const SourceManager &SM) : Check(Check), SM(SM) {}

    void Ifdef(SourceLocation Loc, const Token &MacroNameTok, const MacroDefinition &MD) override {
        std::string MacroName = MacroNameTok.getIdentifierInfo()->getName().str();
        if (MacroName == "SERVER") {
            DirectiveStack.push_back({Loc, true, false}); // isServer=true, hasElse=false
            std::string ReplacementText = "if constexpr (Config::IsServer(M)) {";
            Check.diag(Loc, "replacing #ifdef with if constexpr")
                << FixItHint::CreateReplacement(SourceRange(GetHashLoc(Loc), MacroNameTok.getEndLoc()), ReplacementText);
        } else {
            DirectiveStack.push_back({Loc, false, false});
        }
    }

    void Ifndef(SourceLocation Loc, const Token &MacroNameTok, const MacroDefinition &MD) override
    {
        std::string MacroName = MacroNameTok.getIdentifierInfo()->getName().str();
        if (MacroName == "SERVER") {
            DirectiveStack.push_back({Loc, true, false}); // isServer=true, hasElse=false
            std::string ReplacementText = "if constexpr (Config::IsServer(M)) {";
            Check.diag(Loc, "replacing #ifdef with if constexpr")
                << FixItHint::CreateReplacement(SourceRange(GetHashLoc(Loc), MacroNameTok.getEndLoc()), ReplacementText);
        } else {
            DirectiveStack.push_back({Loc, false, false});
        }
    }

    void Else(SourceLocation Loc, SourceLocation IfLoc) override {
        if (!DirectiveStack.empty()) {
            auto &current = DirectiveStack.back();
            if (current.isServer && !current.hasElse) {
                current.hasElse = true;
                Check.diag(Loc, "replacing #else with } else {")
                    << FixItHint::CreateReplacement(SourceRange(GetHashLoc(Loc), Loc), "} else {");
            }
        }
    }

    void Endif(SourceLocation Loc, SourceLocation IfLoc) override
    {
        if (!DirectiveStack.empty()) {
            if (DirectiveStack.back().isServer) { // if it's a SERVER macro
                // // Find start of #endif directive
                // SourceLocation HashLoc = Loc.getLocWithOffset(-1);
                
                // // Get the full extent of #endif directive
                // Token EndifTok;
                // Lexer::getRawToken(Loc, EndifTok, SM, LangOptions());
                // SourceLocation EndifEnd = EndifTok.getEndLoc();
                Check.diag(Loc, "replacing #endif with closing brace") 
                    << FixItHint::CreateReplacement(SourceRange(GetHashLoc(Loc), Loc), "}");
            }
            DirectiveStack.pop_back();
        }
    }

private:
    SourceLocation GetHashLoc(SourceLocation Loc) {
        return Loc.getLocWithOffset(-1);
    }

    struct DirectiveInfo {
        SourceLocation Loc;
        bool isServer;
        bool hasElse;
    };
    ClangTidyCheck &Check;
    [[maybe_unused]] const SourceManager &SM;
    std::vector<DirectiveInfo> DirectiveStack; // Tracks ifdef/ifndef locations and if they're SERVER
};


void SoldatIfdefReplacer::registerPPCallbacks(const SourceManager &SM, Preprocessor *PP, Preprocessor *ModuleExpanderPP)
{
    PP->addPPCallbacks(std::make_unique<IfdefPPCallbacks>(*this, SM));
}

namespace {

class SoldatIfdefReplacerModule : public ClangTidyModule
{
public:
    void addCheckFactories(ClangTidyCheckFactories& CheckFactories) override
    {
        CheckFactories.registerCheck<SoldatIfdefReplacer>("soldat-ifdef-replacer");
    }
};

}  // namespace

namespace clang::tidy {

// Register the module using this statically initialized variable.
static ClangTidyModuleRegistry::Add<::SoldatIfdefReplacerModule> soldatIfdefReplacerInit("soldat-ifdef-replacer-module",
                                                                                       "Adds 'soldat-ifdef-replacer' checks.");

// This anchor is used to force the linker to link in the generated object file and thus register the module.
volatile int awesomePrefixCheckAnchorSource = 0;

}  // namespace clang::tidy
