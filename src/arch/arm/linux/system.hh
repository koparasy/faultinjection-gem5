/*
 * Copyright (c) 2010 ARM Limited
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2002-2005 The Regents of The University of Michigan
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
 * Authors: Ali Saidi
 */

#ifndef __ARCH_ARM_LINUX_SYSTEM_HH__
#define __ARCH_ARM_LINUX_SYSTEM_HH__

#include <string>
#include <vector>

#include "arch/arm/system.hh"
#include "kern/linux/events.hh"
#include "params/LinuxArmSystem.hh"

class LinuxArmSystem : public ArmSystem
{
  protected:
    static const int ParamsList =  0x100;

  public:
    /** Boilerplate params code */
    typedef LinuxArmSystemParams Params;
    const Params *
    params() const
    {
        return dynamic_cast<const Params *>(_params);
    }

    LinuxArmSystem(Params *p);
    ~LinuxArmSystem();

    void initState();

  private:
#ifndef NDEBUG
    /** Event to halt the simulator if the kernel calls panic()  */
    BreakPCEvent *kernelPanicEvent;
#endif
    /**
     * PC based event to skip udelay(<time>) calls and quiesce the
     * processor for the appropriate amount of time. This is not functionally
     * required but does speed up simulation.
     */
    Linux::UDelayEvent *uDelaySkipEvent;

    /** Another PC based skip event for const_udelay(). Similar to the udelay
     * skip, but this function precomputes the first multiply that is done
     * in the generic case since the parameter is known at compile time.
     * Thus we need to do some division to get back to us.
     */
    Linux::UDelayEvent *constUDelaySkipEvent;
};

#endif // __ARCH_ARM_LINUX_SYSTEM_HH__

