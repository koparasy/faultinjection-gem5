# Copyright (c) 2010 ARM Limited
# All rights reserved.
#
# The license below extends only to copyright in the software and shall
# not be construed as granting a license to any other intellectual
# property including but not limited to intellectual property relating
# to a hardware implementation of the functionality of the software
# licensed hereunder.  You may use the software subject to the license
# terms below provided that you ensure that this notice is replicated
# unmodified and in its entirety in all distributions of the software,
# modified or unmodified, in source code or in binary form.
#
# Copyright (c) 2006-2007 The Regents of The University of Michigan
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Authors: Ali Saidi

import optparse
import os
import sys

import m5
from m5.defines import buildEnv
from m5.objects import *
from m5.util import addToPath, fatal

if not buildEnv['FULL_SYSTEM']:
    fatal("This script requires full-system mode (*_FS).")

addToPath('../common')

from FSConfig import *
from SysPaths import *
from Benchmarks import *
import Simulation
import CacheConfig
from Caches import *

# Get paths we might need.  It's expected this file is in m5/configs/example.
config_path = os.path.dirname(os.path.abspath(__file__))
config_root = os.path.dirname(config_path)

parser = optparse.OptionParser()

# Simulation options
parser.add_option("--timesync", action="store_true",
        help="Prevent simulated time from getting ahead of real time")

# System options
parser.add_option("--kernel", action="store", type="string")
parser.add_option("--script", action="store", type="string")
if buildEnv['TARGET_ISA'] == "arm":
    parser.add_option("--bare-metal", action="store_true",
               help="Provide the raw system without the linux specific bits")
    parser.add_option("--machine-type", action="store", type="choice",
            choices=ArmMachineType.map.keys(), default="RealView_PBX")
# Benchmark options
parser.add_option("--dual", action="store_true",
                  help="Simulate two systems attached with an ethernet link")
parser.add_option("-b", "--benchmark", action="store", type="string",
                  dest="benchmark",
                  help="Specify the benchmark to run. Available benchmarks: %s"\
                  % DefinedBenchmarks)

# Metafile options
parser.add_option("--etherdump", action="store", type="string", dest="etherdump",
                  help="Specify the filename to dump a pcap capture of the" \
                  "ethernet traffic")

#fault injection option
parser.add_option("--where", action="store", type="string", dest="where",
                  help="in which core to insert faults")
parser.add_option("--firun", action="store", type="int", dest="firun",
                  help="number of fi campaign run: Used to enable a fault")
parser.add_option("--regnum", action="store", type="int", dest="regnum",
                  help="reg number to inject")
parser.add_option("--regtype", action="store", type="string", dest="regtype",
                  help="reg number to inject")
parser.add_option("--whatval", action="store", type="int", dest="whatval",
                  help="what value to use from the array")
parser.add_option("--timeval", action="store", type="int", dest="timeval",
                  help="what value to use from the array")
parser.add_option("--ftype", action="store", type="string", dest="ftype",
                  help="what value to use from the array")
parser.add_option("--threadId", action="store", type="string", dest="thread_Id",
                  help="which thread will be injected with this fault")

execfile(os.path.join(config_root, "common", "Options.py"))

(options, args) = parser.parse_args()

if args:
    print "Error: script doesn't take any positional arguments"
    sys.exit(1)

# driver system CPU is always simple... note this is an assignment of
# a class, not an instance.
DriveCPUClass = AtomicSimpleCPU
drive_mem_mode = 'atomic'

# system under test can be any CPU
(TestCPUClass, test_mem_mode, FutureClass) = Simulation.setCPUClass(options)

TestCPUClass.clock = '2GHz'
DriveCPUClass.clock = '2GHz'

if options.benchmark:
    try:
        bm = Benchmarks[options.benchmark]
    except KeyError:
        print "Error benchmark %s has not been defined." % options.benchmark
        print "Valid benchmarks are: %s" % DefinedBenchmarks
        sys.exit(1)
else:
    if options.dual:
        bm = [SysConfig(), SysConfig()]
    else:
        bm = [SysConfig()]

np = options.num_cpus

if buildEnv['TARGET_ISA'] == "alpha":
    test_sys = makeLinuxAlphaSystem(test_mem_mode, bm[0])
elif buildEnv['TARGET_ISA'] == "mips":
    test_sys = makeLinuxMipsSystem(test_mem_mode, bm[0])
elif buildEnv['TARGET_ISA'] == "sparc":
    test_sys = makeSparcSystem(test_mem_mode, bm[0])
elif buildEnv['TARGET_ISA'] == "x86":
    test_sys = makeLinuxX86System(test_mem_mode, options.num_cpus, bm[0])
    setWorkCountOptions(test_sys, options)
elif buildEnv['TARGET_ISA'] == "arm":
    test_sys = makeArmSystem(test_mem_mode,
            options.machine_type, bm[0],
            bare_metal=options.bare_metal)
else:
    fatal("incapable of building non-alpha or non-sparc full system!")

if options.kernel is not None:
    test_sys.kernel = binary(options.kernel)

if options.script is not None:
    test_sys.readfile = options.script

test_sys.cpu = [TestCPUClass(cpu_id=i) for i in xrange(np)]

CacheConfig.config_cache(options, test_sys)

if options.caches or options.l2cache:
    if bm[0]:
        mem_size = bm[0].mem()
    else:
        mem_size = SysConfig().mem()
    # For x86, we need to poke a hole for interrupt messages to get back to the
    # CPU. These use a portion of the physical address space which has a
    # non-zero prefix in the top nibble. Normal memory accesses have a 0
    # prefix.
    if buildEnv['TARGET_ISA'] == 'x86':
        test_sys.bridge.filter_ranges_a=[AddrRange(0, Addr.max >> 4)]
    else:
        test_sys.bridge.filter_ranges_a=[AddrRange(0, Addr.max)]
    test_sys.bridge.filter_ranges_b=[AddrRange(mem_size)]
    test_sys.iocache = IOCache(addr_range=mem_size)
    test_sys.iocache.cpu_side = test_sys.iobus.port
    test_sys.iocache.mem_side = test_sys.membus.port

for i in xrange(np):
    if options.fastmem:
        test_sys.cpu[i].physmem_port = test_sys.physmem.port

if buildEnv['TARGET_ISA'] == 'mips':
    setMipsOptions(TestCPUClass)

if len(bm) == 2:
    if buildEnv['TARGET_ISA'] == 'alpha':
        drive_sys = makeLinuxAlphaSystem(drive_mem_mode, bm[1])
    elif buildEnv['TARGET_ISA'] == 'mips':
        drive_sys = makeLinuxMipsSystem(drive_mem_mode, bm[1])
    elif buildEnv['TARGET_ISA'] == 'sparc':
        drive_sys = makeSparcSystem(drive_mem_mode, bm[1])
    elif buildEnv['TARGET_ISA'] == 'x86':
        drive_sys = makeX86System(drive_mem_mode, np, bm[1])
    elif buildEnv['TARGET_ISA'] == 'arm':
        drive_sys = makeArmSystem(drive_mem_mode,
                machine_options.machine_type, bm[1])
    drive_sys.cpu = DriveCPUClass(cpu_id=0)
    drive_sys.cpu.connectAllPorts(drive_sys.membus)
    if options.fastmem:
        drive_sys.cpu.physmem_port = drive_sys.physmem.port
    if options.kernel is not None:
        drive_sys.kernel = binary(options.kernel)

    root = makeDualRoot(test_sys, drive_sys, options.etherdump)
elif len(bm) == 1:
    root = Root(system=test_sys)
else:
    print "Error I don't know how to create more than 2 systems."
    sys.exit(1)

if options.timesync:
    root.time_sync_enable = True
    
what_val_num = options.whatval

if (what_val_num == 65):
    what_type = "All0"
elif  (what_val_num == 64):
    what_type = "All1"
else:
    what_type = "Flip"



#time_type = "Tick"



where_str = options.where
reg_num1 = 1
reg_num2 = 2
reg_num3 = 3
reg_num4 = 4
reg_num5 = 5
reg_num6 = 6 
reg_num7 = 7
reg_num8 = 8

reg_num9 = 1
reg_num10 = 2
reg_num11 = 3
reg_num12 = 4
reg_num13 = 5
reg_num14 = 6 
reg_num15 = 7
reg_num16 = 8

time_type = "Tick"
#time_val1 = options.timeval
#time_val2 = options.timeval
time_val1 = 4831842680
time_val2 = 4831843416


test_sys.f_a = RegisterInjectedFault( RegType = "int", Register = reg_num1, where = "all", when = ''.join([time_type, ":", str(time_val1)]), what = ''.join(["Flip", ":", str(1)]),threadId ="all", relative = True, occurrence = 0 , cores = np)
#test_sys.f_b = RegisterInjectedFault( RegType = "int", Register = reg_num2, where = "system.cpu1", when = ''.join([time_type, ":", str(time_val1)]), what = ''.join(["Flip", ":", str(2)]),threadId ="0", relative = True, occurrence = 0)
#test_sys.f_c = RegisterInjectedFault( RegType = "int", Register = reg_num3, where = "system.cpu1", when = ''.join([time_type, ":", str(time_val1)]), what = ''.join(["Flip", ":", str(3)]),threadId ="0", relative = True, occurrence = 0)
#test_sys.f_d = RegisterInjectedFault( RegType = "int", Register = reg_num4, where = "system.cpu1", when = ''.join([time_type, ":", str(time_val1)]), what = ''.join(["Flip", ":", str(4)]),threadId ="0", relative = True, occurrence = 0)
#test_sys.f_e = RegisterInjectedFault( RegType = "int", Register = reg_num5, where = "system.cpu1", when = ''.join([time_type, ":", str(time_val1)]), what = ''.join(["Flip", ":", str(5)]),threadId ="0", relative = True, occurrence = 0)
#test_sys.f_f = RegisterInjectedFault( RegType = "int", Register = reg_num6, where = "system.cpu1", when = ''.join([time_type, ":", str(time_val1)]), what = ''.join(["Flip", ":", str(6)]),threadId ="0", relative = True, occurrence = 0)
#test_sys.f_g = RegisterInjectedFault( RegType = "int", Register = reg_num7, where = "system.cpu1", when = ''.join([time_type, ":", str(time_val1)]), what = ''.join(["Flip", ":", str(7)]),threadId ="0", relative = True, occurrence = 0)
#test_sys.f_h = RegisterInjectedFault( RegType = "int", Register = reg_num8, where = "system.cpu1", when = ''.join([time_type, ":", str(time_val1)]), what = ''.join(["Flip", ":", str(8)]),threadId ="0", relative = True, occurrence = 0)

test_sys.f_i = RegisterInjectedFault( RegType = "int", Register = reg_num9, where = "system.cpu2", when = ''.join([time_type, ":", str(time_val2)]), what = ''.join(["Flip", ":", str(1)]),threadId ="all", relative = True, occurrence = 0 , cores = np)
#test_sys.f_k = RegisterInjectedFault( RegType = "int", Register = reg_num10, where = "system.cpu2", when = ''.join([time_type, ":", str(time_val2)]), what = ''.join(["Flip", ":", str(2)]),threadId ="1", relative = True, occurrence = 0)
#test_sys.f_l = RegisterInjectedFault( RegType = "int", Register = reg_num11, where = "system.cpu2", when = ''.join([time_type, ":", str(time_val2)]), what = ''.join(["Flip", ":", str(3)]),threadId ="1", relative = True, occurrence = 0)
#test_sys.f_m = RegisterInjectedFault( RegType = "int", Register = reg_num12, where = "system.cpu2", when = ''.join([time_type, ":", str(time_val2)]), what = ''.join(["Flip", ":", str(4)]),threadId ="1", relative = True, occurrence = 0)
#test_sys.f_o = RegisterInjectedFault( RegType = "int", Register = reg_num14, where = "system.cpu2", when = ''.join([time_type, ":", str(time_val2)]), what = ''.join(["Flip", ":", str(6)]),threadId ="1", relative = True, occurrence = 0)
#test_sys.f_n = RegisterInjectedFault( RegType = "int", Register = reg_num13, where = "system.cpu2", when = ''.join([time_type, ":", str(time_val2)]), what = ''.join(["Flip", ":", str(5)]),threadId ="1", relative = True, occurrence = 0)
#test_sys.f_p = RegisterInjectedFault( RegType = "int", Register = reg_num15, where = "system.cpu2", when = ''.join([time_type, ":", str(time_val2)]), what = ''.join(["Flip", ":", str(7)]),threadId ="1", relative = True, occurrence = 0)
#test_sys.f_q = RegisterInjectedFault( RegType = "int", Register = reg_num16, where = "system.cpu2", when = ''.join([time_type, ":", str(time_val2)]), what = ''.join(["Flip", ":", str(8)]),threadId ="1", relative = True, occurrence = 0)





#if options.firun == 1 :
#    test_sys.f1  = f
    

Simulation.run(options, root, test_sys, FutureClass)
