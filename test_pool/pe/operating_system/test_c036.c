/** @file
 * Copyright (c) 2023, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#include "val/include/sbsa_avs_val.h"
#include "val/include/sbsa_avs_pe.h"
#include "val/include/val_interface.h"

#define TEST_NUM   (AVS_PE_TEST_NUM_BASE + 36)
#define TEST_RULE  "S_L7PE_09"
#define TEST_DESC  "Check WFE Fine tune delay feature "

static void payload(void)
{
    uint64_t data = 0;
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

    if (g_sbsa_level < 7) {
        val_set_status(index, RESULT_SKIP(g_sbsa_level, TEST_NUM, 01));
        return;
    }

    /* ID_AA64MMFR1_EL1.TWED [35:32] = 0b0001 indicates support for configurable delayed
       trapping for WFE instruction */
    data = VAL_EXTRACT_BITS(val_pe_reg_read(ID_AA64MMFR1_EL1), 32, 35);

    if (data == 1)
        val_set_status(index, RESULT_PASS(g_sbsa_level, TEST_NUM, 01));
    else
        val_set_status(index, RESULT_FAIL(g_sbsa_level, TEST_NUM, 01));
}

uint32_t c036_entry(uint32_t num_pe)
{
    uint32_t status = AVS_STATUS_FAIL;

    status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe, g_sbsa_level,
                                                                TEST_RULE);
    /* This check is when user is forcing us to skip this test */
    if (status != AVS_STATUS_SKIP)
        val_run_test_payload(TEST_NUM, num_pe, payload, 0);

    /* get the result from all PE and check for failure */
    status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);
    val_report_status(0, SBSA_AVS_END(g_sbsa_level, TEST_NUM), TEST_RULE);

    return status;
}
