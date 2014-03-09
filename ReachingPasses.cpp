#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstVisitor.h"
#include <set>
#include <list>
using namespace llvm;

namespace {

    class GenAndKillSets {
        public:
            std::set<StoreInst *> gen;
            std::set<StoreInst *> kill;
    };

    class BBGenAndKillSets {
        public:
            BasicBlock *bb;
            std::set<StoreInst *> gen;
            std::set<StoreInst *> kill;
            BBGenAndKillSets(BasicBlock *val) { bb = val; }
            std::string genString() {
                std::string ret = "Gen Set of BB:" + bb->getName().str() + '\n';
                for (std::set<StoreInst *>::iterator it = gen.begin(); it != gen.end(); it++) {
                    StoreInst *inst = *it;
                    ret += '\t' + inst->getPointerOperand()->getName().str() + '\n';
                }
                return ret;
            }
    };

    struct NaivePass : public FunctionPass {
        static char ID;
        NaivePass() : FunctionPass(ID) {}
        
        virtual bool runOnFunction(Function &F) {
            std::list<BBGenAndKillSets> listOfSets;
            for (Function::iterator f_it = F.begin(); f_it != F.end(); ++f_it) {
                BBGenAndKillSets bbSets = createBBGenAndKillSets(F, f_it); 
                errs() << bbSets.genString();
            }
            return false;
        }

        BBGenAndKillSets createBBGenAndKillSets(Function &F, BasicBlock *f_it) {
            BBGenAndKillSets bbSets(f_it);
            for (BasicBlock::iterator bb_it = f_it->begin(); bb_it != f_it->end(); bb_it++) {
                if (StoreInst *inst = dyn_cast<StoreInst>(bb_it)) {
                    Value *var = inst->getPointerOperand();
                    GenAndKillSets sets;
                    sets.gen.insert(inst);
                    for (Function::iterator f2_it = F.begin(); f2_it != F.end(); ++f2_it) { 
                        for (BasicBlock::iterator bb2_it = f2_it->begin(); 
                                bb2_it != f2_it->end(); bb2_it++) {
                            if (bb2_it == bb_it) {
                                continue;
                            }

                            if (StoreInst *inst2 = dyn_cast<StoreInst>(bb2_it)) {
                                if (inst2->getPointerOperand() == var) {
                                    sets.kill.insert(inst2);
                                }
                            }
                        } 
                    }
                    for (std::set<StoreInst *>::iterator gen = sets.gen.begin(); gen != sets.gen.end(); gen++) {
                        bbSets.gen.insert(*gen);
                    }
                    for (std::set<StoreInst *>::iterator kill = sets.kill.begin(); kill != sets.kill.end(); kill++) {
                        bbSets.kill.insert(*kill);
                    }
                }
            }
            return bbSets;
        }
        
    };
    
    char NaivePass::ID = 0;
    static RegisterPass<NaivePass> X("naive-reaching", "Naive Reaching Analysis", false, false);
}
