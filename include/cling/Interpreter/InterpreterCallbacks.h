//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_INTERPRETER_CALLBACKS_H
#define CLING_INTERPRETER_CALLBACKS_H

#include "clang/AST/DeclarationName.h"

#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/ArrayRef.h"

namespace clang {
  class ASTDeserializationListener;
  class Decl;
  class DeclContext;
  class DeclarationName;
  class ExternalSemaSource;
  class LookupResult;
  class NamedDecl;
  class Scope;
  class Type;
}

namespace cling {
  class Interpreter;
  class InterpreterCallbacks;
  class InterpreterDeserializationListener;
  class InterpreterExternalSemaSource;
  class Transaction;

  /// \brief  This interface provides a way to observe the actions of the
  /// interpreter as it does its thing.  Clients can define their hooks here to
  /// implement interpreter level tools.
  class InterpreterCallbacks {
  protected:

    ///\brief Our interpreter instance.
    ///
    Interpreter* m_Interpreter; // we don't own

    ///\brief Our custom SemaExternalSource, translating interesting events into
    /// callbacks.
    ///
    llvm::OwningPtr<InterpreterExternalSemaSource> m_ExternalSemaSource;

    ///\brief Our custom ASTDeserializationListener, translating interesting 
    /// events into callbacks.
    ///
    llvm::
    OwningPtr<InterpreterDeserializationListener> m_DeserializationListener;

    ///\brief DynamicScopes only! Set to true only when evaluating dynamic expr.
    ///
    bool m_IsRuntime;
  protected:
    void UpdateWithNewDecls(const clang::DeclContext *DC, 
                            clang::DeclarationName Name, 
                            llvm::ArrayRef<clang::NamedDecl*> Decls);
  public:
    ///\brief Constructs the callbacks.
    ///
    ///\param[in] interp - an interpreter.
    ///\param[in] IESS - an InterpreterExternalSemaSource (takes the ownership)
    ///\param[in] IDL - an InterpreterDeserializationListener (owned)
    ///
    InterpreterCallbacks(Interpreter* interp,
                         InterpreterExternalSemaSource* IESS,
                         InterpreterDeserializationListener* IDL);

    ///\brief Constructs the callbacks with default callback adaptors.
    ///
    ///\param[in] interp - an interpreter.
    ///\param[in] enableExternalSemaSourceCallbacks  - creates a default 
    ///           InterpreterExternalSemaSource and attaches it to Sema.
    ///
    ///\param[in] enableInterpreterDeserializationListener - creates a default
    ///           InterpreterDeserializationListener and attaches it to the 
    ///           ModuleManager if it is set.
    ///
    InterpreterCallbacks(Interpreter* interp,
                         bool enableExternalSemaSourceCallbacks = false,
                         bool enableDeserializationListenerCallbacks = false);

    virtual ~InterpreterCallbacks();

    clang::ExternalSemaSource* getInterpreterExternalSemaSource() const;

    clang::ASTDeserializationListener* 
    getInterpreterDeserializationListener() const;

    /// \brief This callback is invoked whenever the interpreter needs to
    /// resolve the type and the adress of an object, which has been marked for
    /// delayed evaluation from the interpreter's dynamic lookup extension.
    ///
    /// \returns true if lookup result is found and should be used.
    ///
    virtual bool LookupObject(clang::LookupResult&, clang::Scope*);
    virtual bool LookupObject(const clang::DeclContext*, clang::DeclarationName);

    ///\brief This callback is invoked whenever interpreter has committed new
    /// portion of declarations.
    ///
    ///\param[in] - The transaction that was committed.
    ///
    virtual void TransactionCommitted(const Transaction&) {}

    ///\brief This callback is invoked whenever interpreter has reverted a
    /// portion of declarations.
    ///
    ///\param[in] - The transaction that was reverted.
    ///
    virtual void TransactionUnloaded(const Transaction&) {}

    /// \brief Used to inform client about a new decl read by the ASTReader.
    ///
    ///\param[in] - The Decl read by the ASTReader.
    virtual void DeclDeserialized(const clang::Decl*) {}

    /// \brief Used to inform client about a new type read by the ASTReader.
    ///
    ///\param[in] - The Type read by the ASTReader.
    virtual void TypeDeserialized(const clang::Type*) {}

    ///\brief DynamicScopes only! Set to true if it is currently evaluating a
    /// dynamic expr.
    ///
    void SetIsRuntime(bool val) { m_IsRuntime = val; }
  };
} // end namespace cling

// TODO: Make the build system in the testsuite aware how to build that class
// and extract it out there again.
namespace cling {
  namespace test {
    class TestProxy {
    public:
      TestProxy();
      int Draw();
      const char* getVersion();

      int Add(int a, int b);
      int Add10(int num);
      void PrintString(std::string s);
      bool PrintArray(int a[], size_t size);

      void PrintArray(float a[][5], size_t size);

      void PrintArray(int a[][4][5], size_t size);
    };

    extern TestProxy* Tester;

    class SymbolResolverCallback: public cling::InterpreterCallbacks {
    private:
      clang::NamedDecl* m_TesterDecl;
    public:
      SymbolResolverCallback(Interpreter* interp);
      ~SymbolResolverCallback();

      bool LookupObject(clang::LookupResult& R, clang::Scope* S);
      bool LookupObject(const clang::DeclContext*, clang::DeclarationName) {
        return false;
      }
      bool ShouldResolveAtRuntime(clang::LookupResult& R, clang::Scope* S);
    };
  } // end test
} // end cling

#endif // CLING_INTERPRETER_CALLBACKS_H
