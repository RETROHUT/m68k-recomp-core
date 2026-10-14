/*
 * function_finder.h — 68K function boundary detection interface.
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "rom_parser.h"
#include "game_config.h"
#include "m68k_decoder.h"

typedef struct {
    uint32_t addr;
    /* Bitset describing why this address is present.  Keeping provenance on
     * the entry (rather than only in log counters) lets the finder report how
     * much of an explicit TOML function list it can reproduce on its own.
     * Multiple bits are expected: an address may be both a configured root
     * and independently rediscovered by a direct call/table edge. */
    uint32_t evidence;
} FunctionEntry;

enum {
    FUNC_EVIDENCE_VECTOR = 1u << 0,
    FUNC_EVIDENCE_CONFIG = 1u << 1,
    FUNC_EVIDENCE_AUTO   = 1u << 2,
    FUNC_EVIDENCE_LATE   = 1u << 3,
};

typedef struct {
    FunctionEntry *entries;
    int            count;
    int            capacity;
} FunctionList;

void function_finder_run(const GenesisRom *rom, FunctionList *list,
                         const GameConfig *cfg, const char *diagnostics_dir);
void function_list_free(FunctionList *list);

/* True if the routine at `start` unconditionally pops its own return address
 * off the stack at entry (Obj_WaitOffscreen idiom). The code generator uses
 * this to emit `jsr`/`bsr` to such a routine as a non-returning tail transfer
 * instead of falling through to the instruction after the call. */
bool function_finder_pops_return_unconditionally(const GenesisRom *rom, uint32_t start);

/* True if `call` is a JSR/BSR to a configured inline-data callee
 * ([functions].inline_data_callees): the bytes after the call are a data block
 * whose first word is its length in bytes (that word included), and the
 * callee returns past it. Sets *cont to the address after the block. False
 * (with a warning) when the length word is implausible. */
bool function_finder_inline_data_continuation(const GenesisRom *rom,
                                              const GameConfig *cfg,
                                              const M68KInstr *call,
                                              uint32_t *cont);
