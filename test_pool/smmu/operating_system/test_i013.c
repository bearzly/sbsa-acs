/** @file
 * Copyright (c) 2023, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0

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
#include "val/include/val_interface.h"
#include "val/include/sbsa_avs_iovirt.h"
#include "val/include/sbsa_avs_pe.h"

#define TEST_NUM   (AVS_SMMU_TEST_NUM_BASE + 13)
#define TEST_RULE  "S_L7SM_01"
#define TEST_DESC  "Check if all DMA reqs behind SMMU "

static
void
payload()
{

  uint32_t num_pcie_rc, num_named_comp;
  uint32_t i, test_fails = 0;
  uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

  if (g_sbsa_level < 7) {
      val_set_status(index, RESULT_SKIP(g_sbsa_level, TEST_NUM, 01));
      return;
  }

  /* check whether all DMA capable PCIe root complexes are behind a SMMU */
  num_pcie_rc = val_iovirt_get_pcie_rc_info(NUM_PCIE_RC, 0);

  for (i = 0; i < num_pcie_rc; i++) {
      /* print info fields */
      val_print(AVS_PRINT_DEBUG, "\n       RC segment no  : 0x%llx",
                    val_iovirt_get_pcie_rc_info(RC_SEGMENT_NUM, i));
      val_print(AVS_PRINT_DEBUG, "\n       CCA attribute  : 0x%x",
                    val_iovirt_get_pcie_rc_info(RC_MEM_ATTRIBUTE, i));
      val_print(AVS_PRINT_DEBUG, "\n       SMMU base addr : 0x%llx\n",
                    val_iovirt_get_pcie_rc_info(RC_SMMU_BASE, i));

      if (val_iovirt_get_pcie_rc_info(RC_MEM_ATTRIBUTE, i) == 0x1 &&
                                    val_iovirt_get_pcie_rc_info(RC_SMMU_BASE, i) == 0) {
          val_print(AVS_PRINT_ERR,
                    "\n       DMA capable PCIe root port with segment no: %llx not behind a SMMU.",
                    val_iovirt_get_pcie_rc_info(RC_SEGMENT_NUM, i));
          test_fails++;
      }
  }

  /* check whether all DMA capable Named component requestors are behind a SMMU */
  num_named_comp = val_iovirt_get_named_comp_info(NUM_NAMED_COMP, 0);
  for (i = 0; i < num_named_comp; i++) {
      /* print info fields */
      val_print(AVS_PRINT_DEBUG, "\n       Named component  :", 0);
      val_print(AVS_PRINT_DEBUG,
                    (char8_t *)val_iovirt_get_named_comp_info(NAMED_COMP_DEV_OBJ_NAME, i), 0);
      val_print(AVS_PRINT_DEBUG, "\n       CCA attribute    : 0x%x",
                    val_iovirt_get_named_comp_info(NAMED_COMP_CCA_ATTR, i));
      val_print(AVS_PRINT_DEBUG, "\n       SMMU base addr   : 0x%llx\n",
                    val_iovirt_get_named_comp_info(NAMED_COMP_SMMU_BASE, i));
      if (val_iovirt_get_named_comp_info(NAMED_COMP_CCA_ATTR, i) == 0x1 &&
                                    val_iovirt_get_named_comp_info(NAMED_COMP_SMMU_BASE, i) == 0) {
          val_print(AVS_PRINT_ERR,
                    "\n       DMA capable named component with namespace path: ", 0);
          val_print(AVS_PRINT_ERR,
                    (char8_t *)val_iovirt_get_named_comp_info(NAMED_COMP_DEV_OBJ_NAME, i), 0);
          val_print(AVS_PRINT_ERR, " not behind a SMMU.", 0);
          test_fails++;
      }
  }

  if (test_fails)
      val_set_status(index, RESULT_FAIL(g_sbsa_level, TEST_NUM, 01));
  else
      val_set_status(index, RESULT_PASS(g_sbsa_level, TEST_NUM, 01));
}

uint32_t
i013_entry(uint32_t num_pe)
{

  uint32_t status = AVS_STATUS_FAIL;

  num_pe = 1;  /* This test is run on single processor */

  status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe, g_sbsa_level, TEST_RULE);
  if (status != AVS_STATUS_SKIP)
      val_run_test_payload(TEST_NUM, num_pe, payload, 0);

  /* get the result from all PE and check for failure */
  status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);

  val_report_status(0, SBSA_AVS_END(g_sbsa_level, TEST_NUM), TEST_RULE);

  return status;
}
