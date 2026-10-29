"""Check that notification producers and the overlay loop share one state.

The optional ARM64 check executes the linked libnx event implementation. It
requires Unicorn; the default symbol check has no extra Python dependencies.
"""
import argparse
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess


def find_nm():
    installed = shutil.which('aarch64-none-elf-nm')
    if installed:
        return installed
    roots = [os.environ.get('DEVKITA64')]
    if os.environ.get('DEVKITPRO'):
        roots.append(str(Path(os.environ['DEVKITPRO']) / 'devkitA64'))
    roots.extend(('/opt/devkitpro/devkitA64', 'C:/devkitPro/devkitA64'))
    for root in filter(None, roots):
        for name in ('aarch64-none-elf-nm', 'aarch64-none-elf-nm.exe'):
            candidate = Path(root) / 'bin' / name
            if candidate.is_file():
                return str(candidate)
    return None


def read_symbols(elf, nm):
    output = subprocess.check_output([nm, '--defined-only', '-n', str(elf)], text=True)
    symbols = {}
    for line in output.splitlines():
        fields = line.split()
        if len(fields) == 3:
            symbols[fields[2]] = (int(fields[0], 16), fields[1])
    return symbols


def state_symbols(symbols, name):
    pattern = re.compile(r'^_ZN3tslL?\d+' + name + r'E(?:\.lto_priv\.\d+)?$')
    return [(symbol, address, binding) for symbol, (address, binding) in symbols.items()
            if pattern.fullmatch(symbol)]


def check_shared(symbols):
    result = {}
    for name in ('notificationEvent', 'notificationJsonMutex', 'notificationGeneration'):
        found = state_symbols(symbols, name)
        # LTO may internalize a single merged inline object (nm type 'b').
        # TU-local mangling and cloned copies are the regression, not visibility.
        if len(found) != 1 or found[0][0].startswith('_ZN3tslL') or '.lto_priv.' in found[0][0]:
            raise RuntimeError(f'{name}: expected one shared object, found {found}')
        result[name] = hex(found[0][1])
    return result


def execute_event(elf, symbols, initialized_event, signalled_event):
    from unicorn import Uc, UcError, UC_ARCH_ARM64, UC_MODE_ARM, UC_HOOK_CODE
    from unicorn import arm64_const as registers

    raw = elf.read_bytes()
    if raw[:6] != b'\x7fELF\x02\x01':
        raise RuntimeError('Expected a little-endian ELF64 image')
    phoff = struct.unpack_from('<Q', raw, 32)[0]
    phsize, phnum = struct.unpack_from('<HH', raw, 54)
    segments = []
    for index in range(phnum):
        kind, _, offset, va, _, files, memory, _ = struct.unpack_from('<IIQQQQQQ', raw, phoff + index * phsize)
        if kind == 1:
            segments.append((va, raw[offset:offset + files], memory))
    base, stack, tls, stop = 0x10000000, 0x40000000, 0x50000000, 0x60000000
    machine = Uc(UC_ARCH_ARM64, UC_MODE_ARM)
    size = (max(va + memory for va, _, memory in segments) + 4095) & ~4095
    machine.mem_map(base, size)
    for va, data, _ in segments:
        machine.mem_write(base + va, data)
    machine.mem_map(stack, 0x10000)
    machine.mem_map(tls, 0x1000)
    machine.mem_map(stop, 0x1000)
    machine.reg_write(registers.UC_ARM64_REG_TPIDRRO_EL0, tls)
    machine.mem_write(tls + 0x1e4, struct.pack('<I', 0x42))

    def on_code(current, address, _size, _data):
        if address == stop:
            current.emu_stop()

    machine.hook_add(UC_HOOK_CODE, on_code)
    for function, event in (('ueventCreate', initialized_event), ('ueventSignal', signalled_event)):
        machine.reg_write(registers.UC_ARM64_REG_X0, base + event)
        machine.reg_write(registers.UC_ARM64_REG_X1, 0)
        machine.reg_write(registers.UC_ARM64_REG_X30, stop)
        machine.reg_write(registers.UC_ARM64_REG_SP, stack + 0xff00)
        try:
            machine.emu_start(base + symbols[function][0], stop + 4, count=10000)
        except UcError as error:
            return {'fault': str(error), 'function': function,
                    'pc_offset': hex(machine.reg_read(registers.UC_ARM64_REG_PC) - base)}
        if machine.reg_read(registers.UC_ARM64_REG_PC) != stop:
            raise RuntimeError(f'{function} failed to return')
    return {'event_create_and_signal': 'pass'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('elf', type=Path)
    parser.add_argument('--nm', default=find_nm())
    parser.add_argument('--emulate', action='store_true')
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    if not args.nm:
        parser.error('Cannot find devkitA64 nm; set DEVKITPRO/DEVKITA64 or pass --nm')
    symbols = read_symbols(args.elf, args.nm)
    report = {'shared_state': check_shared(symbols)}
    if args.emulate:
        address = state_symbols(symbols, 'notificationEvent')[0][1]
        result = execute_event(args.elf, symbols, address, address)
        if 'fault' in result:
            raise RuntimeError(result)
        report['arm64'] = result
    if args.baseline:
        baseline = read_symbols(args.baseline, args.nm)
        events = state_symbols(baseline, 'notificationEvent')
        if len(events) < 2:
            raise RuntimeError('Baseline must contain the duplicate-event regression')
        result = execute_event(args.baseline, baseline, events[-1][1], events[0][1])
        if result.get('function') != 'ueventSignal' or result.get('pc_offset') != '0x65bc8':
            raise RuntimeError(f'Baseline did not reproduce the reported Data Abort: {result}')
        report['reported_crash_reproduced'] = result
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
