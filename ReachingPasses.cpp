#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InstVisitor.h"
using namespace llvm;

namespace {

    struct CustomInstVisitor : public InstVisitor<CustomInstVisitor> {
        CustomInstVisitor() : InstVisitor() {}
        
        void visitStoreInst(StoreInst &i) {
            errs() << "Store Inst Found: " << i.getName() << '\n';
        }

        void visitAllocaInst(AllocaInst &i) {
            errs() << "Allocate Inst Found: " << i.getName() << '\n';
        }

        void visitInstruction(Instruction &i) {
            errs() << "Analyzing Instruction: " << i.getName() << '\n';
        }
    };
    
    struct NaivePass : public FunctionPass {
        static char ID;
        NaivePass() : FunctionPass(ID) {}
        
        virtual bool runOnFunction(Function &F) {
            CustomInstVisitor visitor;
            visitor.visit(F); 
            return false;
        }
        
    };
    
    char NaivePass::ID = 0;
    static RegisterPass<NaivePass> X("naive-reaching", "Naive Reaching Analysis", false, false);
}
