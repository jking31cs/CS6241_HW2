#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Analysis/CFG.h"
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
            //Sanity Check function, used to ensure my output made sense with personal test case.
            std::string genString() {
                std::string ret = "Gen Set of BB:" + bb->getName().str() + '\n';
                for (std::set<StoreInst *>::iterator it = gen.begin(); it != gen.end(); it++) {
                    StoreInst *inst = *it;
                    ret += '\t' + inst->getPointerOperand()->getName().str() + '\n';
                }
                return ret;
            }
    };

    class BBInAndOut {
        public:
            BasicBlock *bb;
            std::set<StoreInst *> in;
            std::set<StoreInst *> out;
            BBInAndOut(BasicBlock *val) { bb = val; }
    };

    struct NaivePass : public FunctionPass {
        static char ID;
        NaivePass() : FunctionPass(ID) {}
        
        virtual bool runOnFunction(Function &F) {
            std::list<BBGenAndKillSets> listOfSets;
            std::list<BBInAndOut> output;
            //First, Generate the Gen And Kill Sets And initialize the output 
            for (Function::iterator f_it = F.begin(); f_it != F.end(); ++f_it) {
                listOfSets.push_back(createBBGenAndKillSets(F, f_it)); 
            }
            for (Function::iterator bb = F.begin(); bb != F.end(); bb++) {
               BBInAndOut out(bb);
               output.push_back(out);
            }

            //Now go through a while loop to generate ins and outs.
            bool change = true;
            while (change) {
                //Set this to false for now, change if changes are made.
                change = false;
                
                //Abusing the fact that I pushed the blocks in order. Doesn't include entry.
                for (BBGenAndKillSets sets : listOfSets) {
                    if (sets.bb == &(F.getEntryBlock())) {
                        continue;
                    }
                    std::set<StoreInst *> new_in;
                    std::set<StoreInst *> new_out;
                    
                    //First get out the in and out for this basic block.
                    BBInAndOut myout = NULL;
                    for (BBInAndOut out : output) {
                        if (out.bb == sets.bb)
                            myout = out;  
                    }
                    
                    //Go through predecessors and get new In
                    for (pred_iterator pi = pred_begin(sets.bb); pi != pred_end(sets.bb); pi++) {
                        BasicBlock *pred = *pi;
                        for (BBInAndOut pred_out : output) {
                            if (pred_out.bb == pred) {
                                for (StoreInst * inst  : pred_out.out) {
                                    new_in.insert(inst);
                                }
                            }
                        }
                    }

                    //Early check on ins, if they don't change, outs can't.
                    if (new_in == myout.in) continue;


                    //Now get new Out from gen set union (IN - Kill)
                    for (StoreInst *inst : sets.gen) {
                        new_out.insert(inst);
                    }
                    for (StoreInst *inst : myout.in) {
                        if (sets.kill.find(inst) == sets.kill.end()) {
                            new_out.insert(inst);
                        }
                    }
                    
                    //Now Check for Changes
                    myout.in.swap(new_in);
                    if (new_out != myout.out) {
                        change = true;
                        errs() << "Size of old" << myout.out.size() << " Size of New" << new_out.size() << '\n';
                        myout.out.swap(new_out);
                    }

                }
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
