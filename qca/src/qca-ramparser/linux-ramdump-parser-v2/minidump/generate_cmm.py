#Copyright (c) 2019, The Linux Foundation. All rights reserved.
#
#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License version 2 and
#only version 2 as published by the Free Software Foundation.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#

import glob
import os
from optparse import OptionParser
import fileinput

parser = parser = OptionParser()
parser.add_option('--config',dest='config',help=' CONFIG is set to 32 or 64. Default is 32 bit')
parser.add_option('--arch',dest='arch',help='arch is set to ipq807x or ipq60xx. Default is ipq807x')
(options, args) = parser.parse_args()

module_input_file=open("MOD_INFO.txt")
module_output_file=open("Load_modules.cmm","w")

umac = None
qca_ol = None
wifi_3_0 = None
qdf = None

def print_mod_info(name,line):
    address = line[line.index('=') + 1 : line.index('\0')]
    module_output_file.write("\nd.load.elf "+ name +" /nocode /noclear  /reloc .bss AT 0x" + address)

for line in reversed(module_input_file.readlines()):
    if "umac" in line and umac != True:
         name = "umac.ko"
         umac = True
         print_mod_info(name,line)
    if "qca_ol" in line and qca_ol != True:
         name = "qca_ol.ko"
         qca_ol = True
         print_mod_info(name,line)
    if "wifi_3_0" in line and wifi_3_0 != True:
         name = "wifi_3_0.ko"
         wifi_3_0 = True
         print_mod_info(name,line)
    if "qdf" in line and qdf != True:
         name = "qdf.ko"
         qdf = True
         print_mod_info(name,line)
    if "PGD" in line:
        PGD = line[line.index('=') +1:line.index('\0')]

module_input_file.close()
module_output_file.close()

if options.config == "64":
    t32commands = ["r.s M 0x05",
    "Data.Set SPR:0x30201 %Quad 0x"+PGD,
    "Data.Set SPR:0x30202 %Quad 0x00000012B5193519",
    "Data.Set SPR:0x30A20 %Quad 0x000000FF440C0400",
    "Data.Set SPR:0x30A30 %Quad 0x0000000000000000",
    "Data.Set SPR:0x30100 %Quad 0x0000000034D5D91D",
    "MMU.SCAN PT 0xFFFFFF8000000000--0xFFFFFFFFFFFFFFFF",
    "mmu.on",
    "mmu.scan",
    "task.config c:\\T32\demo\\arm64\kernel\linux\linux-3.x\linux3.t32",
    "menu.reprogram c:\\T32\demo\\arm64\kernel\linux\linux-3.x\linux.men",
    "task.dtask",
    "v.v  %ASCII %STRING linux_banner"]
    if options.arch == "ipq60xx":
        vmlinux = "openwrt-ipq-ipq60xx_64-vmlinux.elf"
    else:
        vmlinux = "openwrt-ipq-ipq807x_64-vmlinux.elf"
else:
    t32commands = ["r.s M 0x13",
	"PER.Set.simple SPR:0x30200 %Quad 0x"+PGD,
	"PER.Set.simple C15:0x1 %Long 0x1025",
	"Data.Set SPR:0x36110 %Quad 0x535",
	"mmu.on",
	"mmu.scan",
	"task.config c:\\T32\demo\\arm\kernel\linux\linux-3.x\linux3.t32",
	"menu.reprogram c:\\T32\demo\\arm\kernel\linux\linux-3.x\linux.men",
	"task.dtask",
	"v.v  %ASCII %STRING linux_banner"]
    if options.arch == "ipq60xx":
        vmlinux = "openwrt-ipq-ipq60xx-vmlinux.elf"
    else:
        vmlinux = "openwrt-ipq-ipq807x-vmlinux.elf"

def file_base_name(file_name):
    if '.' in file_name:
        separator_index = file_name.index('.')
        base_name = file_name[:separator_index]
        return base_name
    else:
        return file_name


onlyfiles = (glob.glob("*.BIN"))
file=open("startup_t32.cmm","w")
file.write("sys.cpu CORTEXA53 " + "\n")

file.write("sys.up" + "\n");

for i in range(len(onlyfiles)):
	base_name = file_base_name(onlyfiles[i])
	file.write("data.load.binary" + " " + onlyfiles[i] + " "+ " 0x" + base_name + "\n")

file.write("data.load.elf" + " " + vmlinux + " "+ "/Nocode" + "\n")

for i in range(len(t32commands)):
	file.write(t32commands[i] +"\n")

file.close()

for line in fileinput.input('MOD_INFO.txt', inplace=True):
    line = line.strip('\n')
    line = line.replace('\0',' ')
    print line

fileinput.close()
