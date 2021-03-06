/*
 * Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */

/*********************************************************************
 * PM requirements are data structures allocated for each valid
 * master/slave pair, used for tracking master's requests for slave's
 * capabilities/states.
 *********************************************************************/

#ifndef PM_REQUIREMENT_H_
#define PM_REQUIREMENT_H_

#include "pm_common.h"
#include "xil_types.h"
#include "pm_node.h"

/* Forward declarations */
typedef struct PmMaster PmMaster;
typedef struct PmSlave PmSlave;
typedef struct PmRequirement PmRequirement;

typedef enum {
	RELEASE_ONE,
	RELEASE_ALL,
	RELEASE_UNREQUESTED,
} PmReleaseScope;

/*********************************************************************
 * Macros
 ********************************************************************/
/*
 * Max number of master/slave pairs (max number of combinations that can
 * exist at the runtime). The value is used to statically initialize
 * size of the pmReqData array, which is used as the heap.
 */
#define PM_REQUIREMENT_MAX	200U

/* Requirement flags */
#define PM_MASTER_WAKEUP_REQ_MASK	0x1U
#define PM_MASTER_REQUESTED_SLAVE_MASK	0x2U
#define PM_MASTER_SET_LATENCY_REQ	0x4U
#define PM_SYSTEM_USING_SLAVE_MASK	0x8U

#define MASTER_REQUESTED_SLAVE(reqPtr)	\
	(0U != (PM_MASTER_REQUESTED_SLAVE_MASK & (reqPtr)->info))

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmRequirement - structure for tracking requirements of a master for the slave
 *              setting. One structure should be statically assigned for each
 *              possible combination of master/slave, because dynamic memory
 *              allocation cannot be used.
 * @slave       Pointer to the slave structure
 * @master      Pointer to the master structure
 * @nextSlave   Pointer to the master's requirement for a next slave in the list
 * @nextMaster  Pointer to the requirement of a next master that uses the slave
 * @preReq      Requirements of a master that it cannot request for itself (when
 *              a master starts the cold boot there are some resources it will
 *              use before it is capable of requesting them, like memories)
 * @defaultReq  Default requirements of a master - requirements for slave
 *              capabilities without which the master cannot run
 * @currReq     Currently holding requirements of a master for this slave
 * @nextReq     Requirements of a master to be configured when it changes the
 *              state (after it goes to sleep or before it gets awake)
 * @latencyReq  Latency requirements of master for the slave's transition time
 *              from any to its maximum (highest id) state
 * @info        Contains information about master's request - a bit for
 *              encoding has master requested or released node, and a bit to
 *              encode has master requested a wake-up of this slave.
 */
typedef struct PmRequirement {
	PmSlave* slave;
	PmMaster* master;
	PmRequirement* nextSlave;
	PmRequirement* nextMaster;
	u32 preReq;
	u32 defaultReq;
	u32 currReq;
	u32 nextReq;
	u32 latencyReq;
	u8 info;
} PmRequirement;

/*********************************************************************
 * Function declarations
 ********************************************************************/

void PmRequirementCancelScheduled(const PmMaster* const master);
void PmRequirementPreRequest(const PmMaster* const master);
void PmRequirementClockRestore(const PmMaster* const master);
void PmRequirementFreeAll(void);
void PmRequirementClear(PmRequirement* const req);

int PmRequirementSchedule(PmRequirement* const masterReq, const u32 caps);
int PmRequirementUpdate(PmRequirement* const masterReq, const u32 caps);
int PmRequirementUpdateScheduled(const PmMaster* const master, const bool swap);
int PmRequirementRequest(PmRequirement* const req, const u32 caps);
int PmRequirementRelease(PmRequirement* const first, const PmReleaseScope scope);

PmRequirement* PmRequirementAdd(PmMaster* const master, PmSlave* const slave);
PmRequirement* PmRequirementGet(const PmMaster* const master,
				const PmSlave* const slave);
PmRequirement* PmRequirementGetNoMaster(const PmSlave* const slave);

int PmRequirementSetConfig(PmRequirement* const req, const u32 flags,
			   const u32 currReq, const u32 defaultReq);

#endif
