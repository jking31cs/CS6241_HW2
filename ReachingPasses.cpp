#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
using namespace llvm;

namespace {
    
    struct NaivePass : public FunctionPass {
        static char ID;
        NaivePass() : FunctionPass(ID) {}
        
        virtual bool runOnFunction(Function &F) {
            Function::iterator it = F.begin();
            for (; it != F.end(); ++it) { //Skips the entry block
                BasicBlock::iterator instruction = it->begin();
                for (; instruction != it->end(); instruction++) {
                    if (isa<PHINode>(instruction)) {
                        continue;
                    }
                    errs() << "Instruction found: " << instruction->getOpcodeName() << '\n';
                } 
            } 
            return false;
        }
    };
    
    char NaivePass::ID = 0;
    static RegisterPass<NaivePass> X("naive-reaching", "Naive Reaching Analysis", false, false);
}
