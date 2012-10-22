/*
 * Copyright (c) 2007 MIPS Technologies, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Korey Sewell
 *
 */

#include <list>
#include <vector>

#include "cpu/inorder/resources/execution_unit.hh"
#include "cpu/inorder/cpu.hh"
#include "cpu/inorder/resource_pool.hh"
#include "debug/InOrderExecute.hh"
#include "debug/InOrderStall.hh"

using namespace std;
using namespace ThePipeline;

ExecutionUnit::ExecutionUnit(string res_name, int res_id, int res_width,
                             int res_latency, InOrderCPU *_cpu,
                             ThePipeline::Params *params)
    : Resource(res_name, res_id, res_width, res_latency, _cpu),
      lastExecuteTick(0), lastControlTick(0), serializeTick(0)
{ }

void
ExecutionUnit::regStats()
{
    predictedTakenIncorrect
        .name(name() + ".predictedTakenIncorrect")
        .desc("Number of Branches Incorrectly Predicted As Taken.");

    predictedNotTakenIncorrect
        .name(name() + ".predictedNotTakenIncorrect")
        .desc("Number of Branches Incorrectly Predicted As Not Taken).");

    executions
        .name(name() + ".executions")
        .desc("Number of Instructions Executed.");

 
    predictedIncorrect
        .name(name() + ".mispredicted")
        .desc("Number of Branches Incorrectly Predicted");

    predictedCorrect
        .name(name() + ".predicted")
        .desc("Number of Branches Incorrectly Predicted");

    mispredictPct
        .name(name() + ".mispredictPct")
        .desc("Percentage of Incorrect Branches Predicts")
        .precision(6);
    mispredictPct = (predictedIncorrect / 
                     (predictedCorrect + predictedIncorrect)) * 100;

    Resource::regStats();
}

void
ExecutionUnit::execute(int slot_num)
{
    ResourceRequest* exec_req = reqs[slot_num];
    DynInstPtr inst = reqs[slot_num]->inst;
    Fault fault = NoFault;
    InstSeqNum seq_num = inst->seqNum;
    Tick cur_tick = curTick();

    if (cur_tick == serializeTick) {
        DPRINTF(InOrderExecute, "Can not execute [tid:%i][sn:%i][PC:%s] %s. "
                "All instructions are being serialized this cycle\n",
                inst->readTid(), seq_num, inst->pcState(), inst->instName());
        exec_req->done(false);
        return;
    }


    switch (exec_req->cmd)
    {
      case ExecuteInst:
        {
            if (inst->isNop()) {
                DPRINTF(InOrderExecute, "[tid:%i] [sn:%i] [PC:%s] Ignoring execution"
                        "of %s.\n", inst->readTid(), seq_num, inst->pcState(),
                        inst->instName());
                inst->setExecuted();
                exec_req->done();
                return;
            } else {
                DPRINTF(InOrderExecute, "[tid:%i] Executing [sn:%i] [PC:%s] %s.\n",
                        inst->readTid(), seq_num, inst->pcState(), inst->instName());
            }

            if (cur_tick != lastExecuteTick) {
                lastExecuteTick = cur_tick;
            }

            assert(!inst->isMemRef());

            if (inst->isSerializeAfter()) {
                serializeTick = cur_tick;
                DPRINTF(InOrderExecute, "Serializing execution after [tid:%i] "
                        "[sn:%i] [PC:%s] %s.\n", inst->readTid(), seq_num,
                        inst->pcState(), inst->instName());
            }

            if (inst->isControl()) {
                if (lastControlTick == cur_tick) {
                    DPRINTF(InOrderExecute, "Can not Execute More than One Control "
                            "Inst Per Cycle. Blocking Request.\n");
                    exec_req->done(false);
                    return;
                }
                lastControlTick = curTick();

                // Evaluate Branch
                fault = inst->execute();
                executions++;

                inst->setExecuted();

                if (fault == NoFault) {
                    // If branch is mispredicted, then signal squash
                    // throughout all stages behind the pipeline stage
                    // that got squashed.
                    if (inst->mispredicted()) {
                        int stage_num = exec_req->getStageNum();
                        ThreadID tid = inst->readTid();
                        // If it's a branch ...
                        if (inst->isDirectCtrl()) {
                            assert(!inst->isIndirectCtrl());

                            TheISA::PCState pc = inst->pcState();
                            TheISA::advancePC(pc, inst->staticInst);
                            inst->setPredTarg(pc);

                            if (inst->predTaken() && inst->isCondDelaySlot()) {
                                inst->bdelaySeqNum = seq_num;

                                DPRINTF(InOrderExecute, "[tid:%i]: Conditional"
                                        " branch inst [sn:%i] PC %s mis"
                                        "predicted as taken.\n", tid,
                                        seq_num, inst->pcState());
                            } else if (!inst->predTaken() &&
                                       inst->isCondDelaySlot()) {
                                inst->bdelaySeqNum = seq_num;
                                inst->procDelaySlotOnMispred = true;

                                DPRINTF(InOrderExecute, "[tid:%i]: Conditional"
                                        " branch inst [sn:%i] PC %s mis"
                                        "predicted as not taken.\n", tid,
                                        seq_num, inst->pcState());
                            } else {
#if ISA_HAS_DELAY_SLOT
                                inst->bdelaySeqNum = seq_num + 1;
#else
                                inst->bdelaySeqNum = seq_num;
#endif
                                DPRINTF(InOrderExecute, "[tid:%i]: "
                                        "Misprediction detected at "
                                        "[sn:%i] PC %s,\n\t squashing after "
                                        "delay slot instruction [sn:%i].\n",
                                        tid, seq_num, inst->pcState(),
                                        inst->bdelaySeqNum);
                                DPRINTF(InOrderStall, "STALL: [tid:%i]: Branch"
                                        " misprediction at %s\n",
                                        tid, inst->pcState());
                            }

                            DPRINTF(InOrderExecute, "[tid:%i] Redirecting "
                                    "fetch to %s.\n", tid,
                                    inst->readPredTarg());

                        } else if (inst->isIndirectCtrl()){
                            TheISA::PCState pc = inst->pcState();
                            TheISA::advancePC(pc, inst->staticInst);
                            inst->seqNum = seq_num;
                            inst->setPredTarg(pc);

#if ISA_HAS_DELAY_SLOT
                            inst->bdelaySeqNum = seq_num + 1;
#else
                            inst->bdelaySeqNum = seq_num;
#endif

                            DPRINTF(InOrderExecute, "[tid:%i] Redirecting"
                                    " fetch to %s.\n", tid,
                                    inst->readPredTarg());
                        } else {
                            panic("Non-control instruction (%s) mispredict"
                                  "ing?!!", inst->staticInst->getName());
                        }

                        DPRINTF(InOrderExecute, "[tid:%i] Squashing will "
                                "start from stage %i.\n", tid, stage_num);

                        cpu->pipelineStage[stage_num]->squashDueToBranch(inst,
                                                                         tid);

                        inst->squashingStage = stage_num;

                        // Squash throughout other resources
                        cpu->resPool->scheduleEvent((InOrderCPU::CPUEventType)
                                                    ResourcePool::SquashAll,
                                                    inst, 0, 0, tid);

                        if (inst->predTaken()) {
                            predictedTakenIncorrect++;
                            DPRINTF(InOrderExecute, "[tid:%i] [sn:%i] %s ..."
                                    "PC %s ... Mispredicts! (Taken)\n",
                                    tid, inst->seqNum,
                                    inst->staticInst->disassemble(
                                        inst->instAddr()),
                                    inst->pcState());
                        } else {
                            predictedNotTakenIncorrect++;
                            DPRINTF(InOrderExecute, "[tid:%i] [sn:%i] %s ..."
                                    "PC %s ... Mispredicts! (Not Taken)\n",
                                    tid, inst->seqNum,
                                    inst->staticInst->disassemble(
                                        inst->instAddr()),
                                    inst->pcState());
                        }
                        predictedIncorrect++;
                    } else {
                        DPRINTF(InOrderExecute, "[tid:%i]: [sn:%i]: Prediction"
                                "Correct.\n", inst->readTid(), seq_num);
                        predictedCorrect++;
                    }

                    exec_req->done();
                } else {
                    warn("inst [sn:%i] had a %s fault",
                         seq_num, fault->name());
                }
            } else {
                // Regular ALU instruction
                fault = inst->execute();
                executions++;

                if (fault == NoFault) {
                    inst->setExecuted();

                    DPRINTF(InOrderExecute, "[tid:%i]: [sn:%i]: The result "
                            "of execution is 0x%x.\n", inst->readTid(),
                            seq_num,
                            (inst->resultType(0) == InOrderDynInst::Float) ?
                            inst->readFloatResult(0) : inst->readIntResult(0));
                } else {
                    DPRINTF(InOrderExecute, "[tid:%i]: [sn:%i]: had a %s "
                            "fault.\n", inst->readTid(), seq_num, fault->name());
                    inst->fault = fault;
                }

                exec_req->done();
            }
        }
        break;

      default:
        fatal("Unrecognized command to %s", resName);
    }
}


