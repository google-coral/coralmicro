/*
 * Copyright 2022 Google LLC
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

#ifndef LIBS_TPU_DARWINN_DRIVER_CONFIG_DEBUG_HIB_USER_CSR_OFFSETS_H_
#define LIBS_TPU_DARWINN_DRIVER_CONFIG_DEBUG_HIB_USER_CSR_OFFSETS_H_

#include <cstdint>

namespace platforms {
namespace darwinn {
namespace driver {
namespace config {

// This struct holds various CSR offsets that will be dumped as part of the
// driver bug report for user hib. Members are intentionally named to match the
// GCSR register names.
struct DebugHibUserCsrOffsets {
  uint64_t instruction_inbound_queue_total_occupancy;
  uint64_t instruction_inbound_queue_threshold_counter;
  uint64_t instruction_inbound_queue_insertion_counter;
  uint64_t instruction_inbound_queue_full_counter;
  uint64_t input_actv_inbound_queue_total_occupancy;
  uint64_t input_actv_inbound_queue_threshold_counter;
  uint64_t input_actv_inbound_queue_insertion_counter;
  uint64_t input_actv_inbound_queue_full_counter;
  uint64_t param_inbound_queue_total_occupancy;
  uint64_t param_inbound_queue_threshold_counter;
  uint64_t param_inbound_queue_insertion_counter;
  uint64_t param_inbound_queue_full_counter;
  uint64_t output_actv_inbound_queue_total_occupancy;
  uint64_t output_actv_inbound_queue_threshold_counter;
  uint64_t output_actv_inbound_queue_insertion_counter;
  uint64_t output_actv_inbound_queue_full_counter;
  uint64_t status_block_write_inbound_queue_total_occupancy;
  uint64_t status_block_write_inbound_queue_threshold_counter;
  uint64_t status_block_write_inbound_queue_insertion_counter;
  uint64_t status_block_write_inbound_queue_full_counter;
  uint64_t queue_fetch_inbound_queue_total_occupancy;
  uint64_t queue_fetch_inbound_queue_threshold_counter;
  uint64_t queue_fetch_inbound_queue_insertion_counter;
  uint64_t queue_fetch_inbound_queue_full_counter;
  uint64_t instruction_outbound_queue_total_occupancy;
  uint64_t instruction_outbound_queue_threshold_counter;
  uint64_t instruction_outbound_queue_insertion_counter;
  uint64_t instruction_outbound_queue_full_counter;
  uint64_t input_actv_outbound_queue_total_occupancy;
  uint64_t input_actv_outbound_queue_threshold_counter;
  uint64_t input_actv_outbound_queue_insertion_counter;
  uint64_t input_actv_outbound_queue_full_counter;
  uint64_t param_outbound_queue_total_occupancy;
  uint64_t param_outbound_queue_threshold_counter;
  uint64_t param_outbound_queue_insertion_counter;
  uint64_t param_outbound_queue_full_counter;
  uint64_t output_actv_outbound_queue_total_occupancy;
  uint64_t output_actv_outbound_queue_threshold_counter;
  uint64_t output_actv_outbound_queue_insertion_counter;
  uint64_t output_actv_outbound_queue_full_counter;
  uint64_t status_block_write_outbound_queue_total_occupancy;
  uint64_t status_block_write_outbound_queue_threshold_counter;
  uint64_t status_block_write_outbound_queue_insertion_counter;
  uint64_t status_block_write_outbound_queue_full_counter;
  uint64_t queue_fetch_outbound_queue_total_occupancy;
  uint64_t queue_fetch_outbound_queue_threshold_counter;
  uint64_t queue_fetch_outbound_queue_insertion_counter;
  uint64_t queue_fetch_outbound_queue_full_counter;
  uint64_t page_table_request_outbound_queue_total_occupancy;
  uint64_t page_table_request_outbound_queue_threshold_counter;
  uint64_t page_table_request_outbound_queue_insertion_counter;
  uint64_t page_table_request_outbound_queue_full_counter;
  uint64_t read_tracking_fifo_total_occupancy;
  uint64_t read_tracking_fifo_threshold_counter;
  uint64_t read_tracking_fifo_insertion_counter;
  uint64_t read_tracking_fifo_full_counter;
  uint64_t write_tracking_fifo_total_occupancy;
  uint64_t write_tracking_fifo_threshold_counter;
  uint64_t write_tracking_fifo_insertion_counter;
  uint64_t write_tracking_fifo_full_counter;
  uint64_t read_buffer_total_occupancy;
  uint64_t read_buffer_threshold_counter;
  uint64_t read_buffer_insertion_counter;
  uint64_t read_buffer_full_counter;
  uint64_t axi_aw_credit_shim_total_occupancy;
  uint64_t axi_aw_credit_shim_threshold_counter;
  uint64_t axi_aw_credit_shim_insertion_counter;
  uint64_t axi_aw_credit_shim_full_counter;
  uint64_t axi_ar_credit_shim_total_occupancy;
  uint64_t axi_ar_credit_shim_threshold_counter;
  uint64_t axi_ar_credit_shim_insertion_counter;
  uint64_t axi_ar_credit_shim_full_counter;
  uint64_t axi_w_credit_shim_total_occupancy;
  uint64_t axi_w_credit_shim_threshold_counter;
  uint64_t axi_w_credit_shim_insertion_counter;
  uint64_t axi_w_credit_shim_full_counter;
  uint64_t instruction_inbound_queue_empty_cycles_count;
  uint64_t input_actv_inbound_queue_empty_cycles_count;
  uint64_t param_inbound_queue_empty_cycles_count;
  uint64_t output_actv_inbound_queue_empty_cycles_count;
  uint64_t status_block_write_inbound_queue_empty_cycles_count;
  uint64_t queue_fetch_inbound_queue_empty_cycles_count;
  uint64_t instruction_outbound_queue_empty_cycles_count;
  uint64_t input_actv_outbound_queue_empty_cycles_count;
  uint64_t param_outbound_queue_empty_cycles_count;
  uint64_t output_actv_outbound_queue_empty_cycles_count;
  uint64_t status_block_write_outbound_queue_empty_cycles_count;
  uint64_t queue_fetch_outbound_queue_empty_cycles_count;
  uint64_t page_table_request_outbound_queue_empty_cycles_count;
  uint64_t read_tracking_fifo_empty_cycles_count;
  uint64_t write_tracking_fifo_empty_cycles_count;
  uint64_t read_buffer_empty_cycles_count;
  uint64_t read_request_arbiter_instruction_request_cycles;
  uint64_t read_request_arbiter_instruction_blocked_cycles;
  uint64_t read_request_arbiter_instruction_blocked_by_arbitration_cycles;
  uint64_t read_request_arbiter_instruction_cycles_blocked_over_threshold;
  uint64_t read_request_arbiter_input_actv_request_cycles;
  uint64_t read_request_arbiter_input_actv_blocked_cycles;
  uint64_t read_request_arbiter_input_actv_blocked_by_arbitration_cycles;
  uint64_t read_request_arbiter_input_actv_cycles_blocked_over_threshold;
  uint64_t read_request_arbiter_param_request_cycles;
  uint64_t read_request_arbiter_param_blocked_cycles;
  uint64_t read_request_arbiter_param_blocked_by_arbitration_cycles;
  uint64_t read_request_arbiter_param_cycles_blocked_over_threshold;
  uint64_t read_request_arbiter_queue_fetch_request_cycles;
  uint64_t read_request_arbiter_queue_fetch_blocked_cycles;
  uint64_t read_request_arbiter_queue_fetch_blocked_by_arbitration_cycles;
  uint64_t read_request_arbiter_queue_fetch_cycles_blocked_over_threshold;
  uint64_t read_request_arbiter_page_table_request_request_cycles;
  uint64_t read_request_arbiter_page_table_request_blocked_cycles;
  uint64_t
      read_request_arbiter_page_table_request_blocked_by_arbitration_cycles;
  uint64_t
      read_request_arbiter_page_table_request_cycles_blocked_over_threshold;
  uint64_t write_request_arbiter_output_actv_request_cycles;
  uint64_t write_request_arbiter_output_actv_blocked_cycles;
  uint64_t write_request_arbiter_output_actv_blocked_by_arbitration_cycles;
  uint64_t write_request_arbiter_output_actv_cycles_blocked_over_threshold;
  uint64_t write_request_arbiter_status_block_write_request_cycles;
  uint64_t write_request_arbiter_status_block_write_blocked_cycles;
  uint64_t
      write_request_arbiter_status_block_write_blocked_by_arbitration_cycles;
  uint64_t
      write_request_arbiter_status_block_write_cycles_blocked_over_threshold;
  uint64_t address_translation_arbiter_instruction_request_cycles;
  uint64_t address_translation_arbiter_instruction_blocked_cycles;
  uint64_t
      address_translation_arbiter_instruction_blocked_by_arbitration_cycles;
  uint64_t
      address_translation_arbiter_instruction_cycles_blocked_over_threshold;
  uint64_t address_translation_arbiter_input_actv_request_cycles;
  uint64_t address_translation_arbiter_input_actv_blocked_cycles;
  uint64_t address_translation_arbiter_input_actv_blocked_by_arbitration_cycles;
  uint64_t address_translation_arbiter_input_actv_cycles_blocked_over_threshold;
  uint64_t address_translation_arbiter_param_request_cycles;
  uint64_t address_translation_arbiter_param_blocked_cycles;
  uint64_t address_translation_arbiter_param_blocked_by_arbitration_cycles;
  uint64_t address_translation_arbiter_param_cycles_blocked_over_threshold;
  uint64_t address_translation_arbiter_status_block_write_request_cycles;
  uint64_t address_translation_arbiter_status_block_write_blocked_cycles;
  uint64_t
      address_translation_arbiter_status_block_write_blocked_by_arbitration_cycles;  // NOLINT
  uint64_t
      address_translation_arbiter_status_block_write_cycles_blocked_over_threshold;  // NOLINT
  uint64_t address_translation_arbiter_output_actv_request_cycles;
  uint64_t address_translation_arbiter_output_actv_blocked_cycles;
  uint64_t
      address_translation_arbiter_output_actv_blocked_by_arbitration_cycles;
  uint64_t
      address_translation_arbiter_output_actv_cycles_blocked_over_threshold;
  uint64_t address_translation_arbiter_queue_fetch_request_cycles;
  uint64_t address_translation_arbiter_queue_fetch_blocked_cycles;
  uint64_t
      address_translation_arbiter_queue_fetch_blocked_by_arbitration_cycles;
  uint64_t
      address_translation_arbiter_queue_fetch_cycles_blocked_over_threshold;
  uint64_t issued_interrupt_count;
  uint64_t data_read_16byte_count;
  uint64_t waiting_for_tag_cycles;
  uint64_t waiting_for_axi_cycles;
  uint64_t simple_translations;
  uint64_t instruction_credits_per_cycle_sum;
  uint64_t input_actv_credits_per_cycle_sum;
  uint64_t param_credits_per_cycle_sum;
  uint64_t output_actv_credits_per_cycle_sum;
  uint64_t status_block_write_credits_per_cycle_sum;
  uint64_t queue_fetch_credits_per_cycle_sum;
  uint64_t page_table_request_credits_per_cycle_sum;
  uint64_t output_actv_queue_control;
  uint64_t output_actv_queue_status;
  uint64_t output_actv_queue_descriptor_size;
  uint64_t output_actv_queue_minimum_size;
  uint64_t output_actv_queue_maximum_size;
  uint64_t output_actv_queue_base;
  uint64_t output_actv_queue_status_block_base;
  uint64_t output_actv_queue_size;
  uint64_t output_actv_queue_tail;
  uint64_t output_actv_queue_fetched_head;
  uint64_t output_actv_queue_completed_head;
  uint64_t output_actv_queue_int_control;
  uint64_t output_actv_queue_int_status;
  uint64_t instruction_queue_control;
  uint64_t instruction_queue_status;
  uint64_t instruction_queue_descriptor_size;
  uint64_t instruction_queue_minimum_size;
  uint64_t instruction_queue_maximum_size;
  uint64_t instruction_queue_base;
  uint64_t instruction_queue_status_block_base;
  uint64_t instruction_queue_size;
  uint64_t instruction_queue_tail;
  uint64_t instruction_queue_fetched_head;
  uint64_t instruction_queue_completed_head;
  uint64_t instruction_queue_int_control;
  uint64_t instruction_queue_int_status;
  uint64_t input_actv_queue_control;
  uint64_t input_actv_queue_status;
  uint64_t input_actv_queue_descriptor_size;
  uint64_t input_actv_queue_minimum_size;
  uint64_t input_actv_queue_maximum_size;
  uint64_t input_actv_queue_base;
  uint64_t input_actv_queue_status_block_base;
  uint64_t input_actv_queue_size;
  uint64_t input_actv_queue_tail;
  uint64_t input_actv_queue_fetched_head;
  uint64_t input_actv_queue_completed_head;
  uint64_t input_actv_queue_int_control;
  uint64_t input_actv_queue_int_status;
  uint64_t param_queue_control;
  uint64_t param_queue_status;
  uint64_t param_queue_descriptor_size;
  uint64_t param_queue_minimum_size;
  uint64_t param_queue_maximum_size;
  uint64_t param_queue_base;
  uint64_t param_queue_status_block_base;
  uint64_t param_queue_size;
  uint64_t param_queue_tail;
  uint64_t param_queue_fetched_head;
  uint64_t param_queue_completed_head;
  uint64_t param_queue_int_control;
  uint64_t param_queue_int_status;
  uint64_t sc_host_int_control;
  uint64_t sc_host_int_status;
  uint64_t top_level_int_control;
  uint64_t top_level_int_status;
  uint64_t fatal_err_int_control;
  uint64_t fatal_err_int_status;
  uint64_t sc_host_int_count;
  uint64_t dma_pause;
  uint64_t dma_paused;
  uint64_t status_block_update;
  uint64_t hib_error_status;
  uint64_t hib_error_mask;
  uint64_t hib_first_error_status;
  uint64_t hib_first_error_timestamp;
  uint64_t hib_inject_error;
  uint64_t read_request_arbiter;
  uint64_t write_request_arbiter;
  uint64_t address_translation_arbiter;
  uint64_t sender_queue_threshold;
  uint64_t page_fault_address;
  uint64_t instruction_credits;
  uint64_t input_actv_credits;
  uint64_t param_credits;
  uint64_t output_actv_credits;
  uint64_t pause_state;
  uint64_t snapshot;
  uint64_t idle_assert;
  uint64_t wire_int_pending_bit_array;
  uint64_t tileconfig0;
  uint64_t tileconfig1;
};

}  // namespace config
}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_CONFIG_DEBUG_HIB_USER_CSR_OFFSETS_H_
