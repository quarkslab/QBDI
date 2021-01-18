/*
 * This file is part of QBDI.
 *
 * Copyright 2017 Quarkslab
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _CALLBACK_H_
#define _CALLBACK_H_

#include "Platform.h"
#include "State.h"
#include "Bitmask.h"
#include "InstAnalysis.h"
#ifdef __cplusplus
#include <vector>

namespace QBDI {
#endif

/*! The callback results.
 */
typedef enum {
    _QBDI_EI(CONTINUE)    = 0, /*!< The execution of the basic block continues. */
    _QBDI_EI(BREAK_TO_VM) = 1, /*!< The execution breaks and returns to the VM causing a complete reevaluation of
                                *   the execution state. A BREAK_TO_VM is needed to ensure that modifications of
                                *   the Program Counter or the program code are taken into account.
                                */
    _QBDI_EI(STOP)        = 2, /*!< Stops the execution of the program. This causes the run function to return
                                *   early.
                                */
} VMAction;

typedef void* VMInstance;

#ifdef __cplusplus
class VM;
using VMInstanceRef = VM*;
#else
typedef VMInstance* VMInstanceRef;
#endif

/*! Instruction callback function type.
 *
 * @param[in] vm            VM instance of the callback.
 * @param[in] gprState      A structure containing the state of the General Purpose Registers. Modifying
 *                          it affects the VM execution accordingly.
 * @param[in] fprState      A structure containing the state of the Floating Point Registers. Modifying
 *                          it affects the VM execution accordingly.
 * @param[in] data          User defined data which can be defined when registering the callback.
 *
 * @return                  The callback result used to signal subsequent actions the VM needs to take.
 */
typedef VMAction (*InstCallback)(VMInstanceRef vm, GPRState *gprState, FPRState *fprState, void *data);

/*! Position relative to an instruction.
 */
typedef enum {
    _QBDI_EI(PREINST) = 0, /*!< Positioned before the instruction.*/
    _QBDI_EI(POSTINST)     /*!< Positioned after the instruction.*/
} InstPosition;

typedef enum {
    _QBDI_EI(SEQUENCE_ENTRY)        = 1,    /*!< Triggered when the execution enters a sequence.*/
    _QBDI_EI(SEQUENCE_EXIT)         = 1<<1, /*!< Triggered when the execution exits from the current sequence.*/
    _QBDI_EI(BASIC_BLOCK_ENTRY)     = 1<<2, /*!< Triggered when the execution enters a basic block.*/
    _QBDI_EI(BASIC_BLOCK_EXIT)      = 1<<3, /*!< Triggered when the execution exits from the current basic block.*/
    _QBDI_EI(BASIC_BLOCK_NEW)       = 1<<4, /*!< Triggered when the execution enters a new (~unknown) basic block.*/
    _QBDI_EI(EXEC_TRANSFER_CALL)    = 1<<5, /*!< Triggered when the ExecBroker executes an execution transfer.*/
    _QBDI_EI(EXEC_TRANSFER_RETURN)  = 1<<6, /*!< Triggered when the ExecBroker returns from an execution transfer.*/
    _QBDI_EI(SYSCALL_ENTRY)         = 1<<7, /*!< Not implemented.*/
    _QBDI_EI(SYSCALL_EXIT)          = 1<<8, /*!< Not implemented.*/
    _QBDI_EI(SIGNAL)                = 1<<9, /*!< Not implemented.*/
} VMEvent;

_QBDI_ENABLE_BITMASK_OPERATORS(VMEvent)

/*!
 * Structure describing the current VM state
 */
typedef struct {
    VMEvent event;           /*!< The event(s) which triggered the callback (must be checked using a mask: event & BASIC_BLOCK_ENTRY).*/
    rword basicBlockStart;   /*!< The current basic block start address which can also be the execution transfer destination.*/
    rword basicBlockEnd;     /*!< The current basic block end address which can also be the execution transfer destination.*/
    rword sequenceStart;     /*!< The current sequence start address which can also be the execution transfer destination.*/
    rword sequenceEnd;       /*!< The current sequence end address which can also be the execution transfer destination.*/
    rword lastSignal;        /*!< Not implemented.*/
} VMState;

/*! VM callback function type.
 *
 * @param[in] vm            VM instance of the callback.
 * @param[in] vmState       A structure containing the current state of the VM.
 * @param[in] gprState      A structure containing the state of the General Purpose Registers. Modifying
 *                          it affects the VM execution accordingly.
 * @param[in] fprState      A structure containing the state of the Floating Point Registers. Modifying
 *                          it affects the VM execution accordingly.
 * @param[in] data          User defined data which can be defined when registering the callback.
 *
 * @return                  The callback result used to signal subsequent actions the VM needs to take.
 */
typedef VMAction (*VMCallback)(VMInstanceRef vm, const VMState *vmState, GPRState *gprState, FPRState *fprState, void *data);

static const uint16_t NO_REGISTRATION = 0xFFFF;
static const uint16_t NOT_FOUND = 0xFFFF;
static const uint16_t ANY = 0xFFFF;

/*! Memory access type (read / write / ...)
 */
typedef enum {
    _QBDI_EI(MEMORY_READ)       = 1,    /*!< Memory read access */
    _QBDI_EI(MEMORY_WRITE)      = 1<<1, /*!< Memory write access */
    _QBDI_EI(MEMORY_READ_WRITE) = 3     /*!< Memory read/write access */
} MemoryAccessType;

_QBDI_ENABLE_BITMASK_OPERATORS(MemoryAccessType);

/*! Memory access flags
 */
typedef enum {
    _QBDI_EI(MEMORY_NO_FLAGS)       = 0,
    _QBDI_EI(MEMORY_UNKNOWN_SIZE)   = 1<<0, /*!< The size of the access isn't known */
    _QBDI_EI(MEMORY_UNKNOWN_VALUE)  = 1<<1  /*!< The value of the access is unknown or hasn't been retrived */
} MemoryAccessFlags;

_QBDI_ENABLE_BITMASK_OPERATORS(MemoryAccessFlags);

/*! Describe a memory access
 */
typedef struct {
    rword instAddress;        /*!< Address of instruction making the access */
    rword accessAddress;      /*!< Address of accessed memory */
    rword value;              /*!< Value read from / written to memory */
    uint16_t size;            /*!< Size of memory access (in bytes) */
    MemoryAccessType type;    /*!< Memory access type (READ / WRITE) */
    MemoryAccessFlags flags;  /*!< Memory access flags */
} MemoryAccess;


#ifdef __cplusplus
struct InstrumentDataCBK {
    InstPosition position;  /*!< Relative position of the event callback (PREINST / POSTINST). */
    InstCallback cbk;       /*!< Address of the function to call when the instruction is executed */
    void* data;             /*!< User defined data which will be forward to cbk */

    InstrumentDataCBK(InstPosition position, InstCallback cbk, void* data) : position(position), cbk(cbk), data(data) {}
};

using InstrumentDataVec = std::vector<InstrumentDataCBK>*;
#else
typedef void* InstrumentDataVec;
#endif

/*! Instrument callback function type.
 *
 * @param[in] vm                VM instance of the callback.
 * @param[in] inst              AnalysisType of the current instrumented Instruction.
 * @param[in] cbks              An object to add the callback to apply for this instruction.
 * @param[in] data              User defined data which can be defined when registering the callback.
 */
typedef void (*QBDI_InstrumentCallback)(VMInstanceRef vm, const InstAnalysis *inst, InstrumentDataVec cbks, void* data);

#ifdef __cplusplus

/*! Instrument callback function type.
 *
 * @param[in] vm                VM instance of the callback.
 * @param[in] inst              AnalysisType of the current instrumented Instruction.
 * @param[in] data              User defined data which can be defined when registering the callback.
 *
 * @return                      Return cbk to call when this instruction is run.
 */
typedef std::vector<InstrumentDataCBK> (*InstrumentCallback)(VMInstanceRef vm, const InstAnalysis *inst, void* data);

} // QBDI::
#endif

#endif
