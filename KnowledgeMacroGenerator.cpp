//===- PrintFunctionNames.cpp ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Example clang plugin which simply prints the names of all the top-level decls
// in the input file.
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;

namespace {
	const std::vector<std::string> endsWith = {
		"iterator",
		"iplist",
		"analysis",
		"analysisusage",
		"ref",
		"Impl",
		"Traits",
		"Pass",
		"Set",
		"iterator_range",
		"Builder",
		"Pair",
		"Map",
		"Info",
		"Entry",
		"resolver",
		"tracking",
	};
	const std::vector<std::string> startswith = {
		"__", 
		"std", 
		"llvm::PointerUnion",
		"llvm::sys",
		"knowledge::",
		"llvm::raw",
		"llvm::pass",
		"llvm::pm",
		"llvm::prettystack",
		"llvm::dominatortree",
	};
	const std::vector<std::string> findCalls = {
		"Allocator",
		"vector", 
		"Vector",
		"Iterator",
		"iterator",
		"ilist",
		"Node",
		"Base",
		"PassManager",
		"llvm::is_integral_or_enum",
		"llvm::Optional",
		"Flags",
		"FlagIt",
	};
	const std::vector<std::string> specialValueTypes = {
		"class llvm::StringRef",
		"class std::basic_string<char>",
		"const class std::basic_string<char>",
		"const class std::basic_string<char>&",
		"class std::basic_string<char>&",
		"class std::basic_string<char>&",
		"class llvm::APInt",
		"const unsigned char*",
		"const char*",
		"char*",
		"unsigned char*",
	};

	class PrintFunctionsConsumer : public ASTConsumer {
		public:
			virtual void HandleTagDeclDefinition(TagDecl *D) {
				if (D->hasNameForLinkage() && D->isClass()) {
					//llvm::errs() << "Class " << D->getNameAsString() << "\n";
					if (const clang::CXXRecordDecl* x = dyn_cast<CXXRecordDecl>(D)) {
						if (!x->isInStdNamespace()) {
							// eliminate all of types taht I will _NEVER_ need
							llvm::StringRef z = x->getQualifiedNameAsString();
							for (std::vector<std::string>::const_iterator it = endsWith.cbegin(); it != endsWith.cend(); ++it) {
								if (z.endswith_lower(*it)) {
									return;
								}
							}

							for (std::vector<std::string>::const_iterator it = startswith.cbegin(); it != startswith.cend(); ++it) {
								if (z.startswith_lower(*it)) {
									return;
								}
							}

							for (std::vector<std::string>::const_iterator it = findCalls.cbegin(); it != findCalls.cend(); ++it) {
								if (z.find(*it) != llvm::StringRef::npos) {
									return;
								}
							}
							llvm::errs() << "Begin(" << x->getQualifiedNameAsString() << ")\n";
							for (clang::CXXRecordDecl::base_class_const_iterator it = x->bases_begin(); it != x->bases_end(); ++it) {
								if (!it->getType()->isInstantiationDependentType()) {
									llvm::errs() << "\tsuper(" << it->getType().getAsString() << ")\n";
								}
							}
							std::vector<std::string> multifields;
							for (clang::CXXRecordDecl::method_iterator it = x->method_begin(); it != x->method_end(); ++it) {
								if (it->getNumParams() == 0 && 
										it->isInstance() && 
										!it->getReturnType()->isVoidType() && 
										!it->isTemplateInstantiation() &&
										!it->isCopyAssignmentOperator() &&
										//!it->isTemplateDecl() &&
										!it->isInAnonymousNamespace() &&
										!it->isOverloadedOperator() && it->isConst()) {
									std::string target,
										slotName,
										ret = it->getReturnType().getUnqualifiedType().getAsString(), 
										name = it->getNameInfo().getAsString();
									StringRef sr = name;
									if (sr.startswith_lower("getContext") || sr.startswith_lower("convert") || sr.startswith_lower("clone") || sr.startswith_lower("Parse") || sr.startswith_lower("strip")) {
										continue;
									} else if (sr == "rbegin" || sr == "rend" || sr.endswith("rbegin") || sr.endswith("rend") || sr == "operands") {
										continue;
#define ignore(str) \
									} else if (sr == str) { \
										continue
#define X(begin, end, size, tbeg, tend, tsize, field) \
									} else if (sr == begin) { \
										std::string tmp; \
										llvm::raw_string_ostream q(tmp); \
										q << "X(" << ret << ", \"" << field << "\", 0, Multifield, " << tbeg << ", " << tend << ", " << tsize << ", 0) // " << it->getParent()->getQualifiedNameAsString() << "\n"; \
										multifields.push_back(q.str()); \
										continue; \
										ignore(end); \
										ignore(size); \
										ignore(field)
#define Y(begin, end, size, tbeg, tend, field) X(begin, end, size, tbeg, tend, "std::distance(" << tbeg << ", " << tend << ")", field)
									X("begin", "end", "size", "t->begin()", "t->end()", "t->size()", "children");
									X("op_begin", "op_end", "getNumOperands", "t->op_begin()", "t->op_end()", "t->getNumOperands()", "operands");
									X("alias_begin", "alias_end", "alias_size", "t->alias_begin()", "t->alias_end()", "t->alias_size()", "aliases");
									X("named_metadata_begin", "named_metadata_end", "named_metadata_size", "t->named_metadata_begin()", "t->named_metadata_end()", "t->named_metadata_size()", "named_metadata");
									Y("global_begin", "global_end", "getNumGlobals", "t->global_begin()", "t->global_end()", "globals");
									X("subtype_begin", "subtype_end", "getNumContainedTypes", "t->subtype_begin()", "t->subtype_end()", "t->getNumContainedTypes()", "subtypes");
									X("param_begin", "param_end", "getNumParams", "t->param_begin()", "t->param_end()", "t->getNumParams()", "params");
									X("element_begin", "element_end", "getNumElements", "t->element_begin()", "t->element_end()", "t->getNumElements()", "elements");
									X("use_begin", "use_end", "getNumUses", "t->use_begin()", "t->use_end()", "t->getNumUses()", "uses");
									Y("user_begin", "user_end", "getNumUsers", "t->user_begin()", "t->user_end()", "users");
									X("idx_begin", "idx_end", "getNumIndices", "t->idx_begin()", "t->idx_end()", "t->getnumIndices()", "idxes");
									ignore("global_empty");
									ignore("empty");
									ignore("alias_empty");
									ignore("named_metadata_empty");
#undef Y
#undef X
#undef ignore
									} else if (sr.endswith("begin")) {
										std::string tmp, prefix = name.substr(0, name.size() - 5);
										llvm::raw_string_ostream q(tmp);
										q << "X(" << ret << ", \"" << prefix << "s\", 0, Multifield, t->" << name << "(), t->" << prefix << "end(), t->getNum" << prefix << "(), 0) // " << it->getParent()->getQualifiedNameAsString() << "\n";
										multifields.push_back(q.str());
										continue;
									} else {
										//sr = it->getNameInfo().getAsString();
										if (sr.startswith("get")) {
											slotName = name.substr(3);
										} else {
											slotName = name;
										}
									}
									bool foundMatch = false;
									for (std::vector<std::string>::const_iterator it = specialValueTypes.cbegin(); it != specialValueTypes.cend();
											++it) {
										if (ret == *it) {
											target = "Value";
											foundMatch = true;
											break;
										}
									}
									if (!foundMatch) {
										if ((it->getReturnType()->isFundamentalType() && !it->getReturnType()->isPointerType()) || 
												it->getReturnType()->isEnumeralType()) {
											target = "Value";
										} else if (it->getReturnType()->isPointerType() || it->getReturnType()->isReferenceType()) {
											target = "Reference";
										} else {
											target = "Unknown_FIXME";
										}
									}
									llvm::errs() << "\tX(" << 
										(it->getReturnType().getCanonicalType()->isBooleanType() ? "bool" : ret) << ", "
										<< "\"" << slotName << "\", " << 
										"t->" << it->getNameInfo().getAsString() << "(), " << 
										target << " ,0) // " << it->getParent()->getQualifiedNameAsString() << "\n";
								}
							}
							for (std::vector<std::string>::const_iterator it = multifields.cbegin(); it != multifields.cend(); ++it) {
								llvm::errs() << "\t" << *it;
							}
							llvm::errs() << "End\n";
						}
					}
				}
			}
			//virtual bool HandleTopLevelDecl(DeclGroupRef DG) {
			//  for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; ++i) {
			//    const Decl *D = *i;
			//    if (D->isInStdNamespace()) {
			//  	  continue;
			//    }
			//    if (D->isInAnonymousNamespace()) {
			//  	  continue;
			//    }
			//    if (D->isFromASTFile()) {
			//  	  continue;
			//    }
			//    if (dyn_cast<CXXConversionDecl>(D)) {
			//  	  continue;
			//    } else if (dyn_cast<CXXConstructorDecl>(D)) {
			//  	  continue;
			//    } else if (dyn_cast<CXXDestructorDecl>(D)) {
			//  	  continue;
			//    } else if (const CXXMethodDecl *ND = dyn_cast<CXXMethodDecl>(D)) {
			//  	  if (!ND->isExternallyVisible()) {
			//  		  continue;
			//  	  }
			//  	  //if (!ND->isOutOfLine()) {
			//  	  //    continue;
			//  	  //}

			//  	  if (ND->isTemplateInstantiation()) {
			//  		  continue;
			//  	  }
			//  	  if (ND->getNumParams() == 0 && ND->isInstance() && !ND->isOverloadedOperator() && !ND->getReturnType()->isVoidType()) {
			//  		  std::string target;
			//  		  if (ND->getReturnType()->isFundamentalType() && !ND->getReturnType()->isPointerType()) {
			//  			  target = "Value";
			//  		  } else if (ND->getReturnType()->isCompoundType()) {
			//  			  target = "CompoundType";
			//  		  } else {
			//  			  target = "Unknown_FIXME!";
			//  		  }
			//  		  llvm::errs() << "X(" << ND->getReturnType().getCanonicalType().getAsString() << ", "
			//  			           << "\"" << ND->getNameInfo().getAsString() << "\", " << 
			//  					   "t->" << ND->getNameInfo().getAsString() << "(), " << target << 
			//  					   " ,0) // " << ND->getParent()->getQualifiedNameAsString() << "\n";
			//  			      
			//  			  //<< "              (class-name \"" << ND->getParent()->getNameAsString() << "\")\n"
			//  			  //<< "              (type " <<  ND->getParent()->getQualifiedNameAsString() << ")\n"
			//  			  //<< "              (function " << ND->getNameInfo().getAsString() << "))\n";
			//  	  }
			//  	  //llvm::errs() << "top-level-decl: \"" << ND->getNameAsString() << "\"\n";
			//     } else if (const NamedDecl *ND = dyn_cast<NamedDecl>(D)){
			//  	   //llvm::errs() << ND->getNameAsString() << "\n";
			//     }
			//  }

			//  return true;
			//}
	};

	class PrintFunctionNamesAction : public PluginASTAction {
		protected:
			std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
					llvm::StringRef) {
				return llvm::make_unique<PrintFunctionsConsumer>();
			}

			bool ParseArgs(const CompilerInstance &CI,
					const std::vector<std::string>& args) {
				for (unsigned i = 0, e = args.size(); i != e; ++i) {
					llvm::errs() << "PrintFunctionNames arg = " << args[i] << "\n";

					// Example error handling.
					if (args[i] == "-an-error") {
						DiagnosticsEngine &D = CI.getDiagnostics();
						unsigned DiagID = D.getCustomDiagID(DiagnosticsEngine::Error,
								"invalid argument '%0'");
						D.Report(DiagID) << args[i];
						return false;
					}
				}
				if (args.size() && args[0] == "help")
					PrintHelp(llvm::errs());

				return true;
			}
			void PrintHelp(llvm::raw_ostream& ros) {
				ros << "Help for PrintFunctionNames plugin goes here\n";
			}

	};

}

static FrontendPluginRegistry::Add<PrintFunctionNamesAction>
X("print-fns", "print function names");
