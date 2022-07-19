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

#ifndef LIBS_TPU_DARWINN_DRIVER_HARDWARE_STRUCTURES_H_
#define LIBS_TPU_DARWINN_DRIVER_HARDWARE_STRUCTURES_H_

#include <stddef.h>

#include <algorithm>
#include <cstdint>

namespace platforms {
namespace darwinn {
namespace driver {

// DarwiNN page table management constants.

// DarwiNN virtual address format
// Simple addressing:
// [63] | [62:25]         | [24:12]          | [11:0]
// 0    | Reserved [0...] | Page Table Index | Page Offset
//
// Extended addressing:
// [63] | [62:34]         | [33:21]           | [20:12]          | [11:0]
// 1    | Reserved [0]    | Extended PT Index | Host Table Index | Page Offset

// The MSB.
static constexpr uint64_t kExtendedVirtualAddressBit = (1ULL << 63);
// Simple addressing: page table index.
static constexpr uint64_t kSimplePageTableIndexShiftBits = 12;
static constexpr uint64_t kSimplePageTableIndexWidthBits = 13;
// Extended addressing: page table index.
static constexpr uint64_t kExtendedPageTableIndexShiftBits = 21;
static constexpr uint64_t kExtendedPageTableIndexWidthBits = 13;
// Extended addressing: host page table index.
static constexpr uint64_t kExtendedHostPageTableIndexShiftBits = 12;
static constexpr uint64_t kExtendedHostPageTableIndexWidthBits = 9;
static constexpr uint64_t kExtendedHostPageTableSizePerPage =
    (1ULL << kExtendedHostPageTableIndexWidthBits);

// Host page info. 4096 bytes.
static constexpr uint64_t kHostPageShiftBits = 12;
static constexpr uint64_t kHostPageSize = (1ULL << kHostPageShiftBits);

// Manage valid / Invalid page table entries.
static constexpr uint64_t kValidPageTableEntryMask = 1;
static constexpr uint64_t kInvalidPageTableEntryValue = 0;

// Manage bar number and offsets.
static constexpr uint64_t kDarwinnBarNumber = 2;
static constexpr uint64_t kDarwinnBarSize = 1ULL * 1024ULL * 1024ULL;

// Defines a descriptor to fetch instructions in the host queue.
struct HostQueueDescriptor {
  uint64_t address;
  uint32_t size_in_bytes;
  uint32_t reserved;
} __attribute__((packed));
static_assert(sizeof(HostQueueDescriptor) == 16, "Must be 16 bytes.");

// Defines the status block that hardware updates.
struct HostQueueStatusBlock {
  // The value of completed_head pointer when the status block was updated.
  uint32_t completed_head_pointer;
  // A bit to indicate that fatal error has occured for the host queue. Using
  // uint32_t to align it to 8B boundary.
  uint32_t fatal_error;
} __attribute__((packed));
static_assert(sizeof(HostQueueStatusBlock) == 8, "Must be 8 bytes.");

// An MSIX table entry as shown in Figure 6-11 in PCI local bus specification
// rev 3.0 document.
struct MsixTableEntry {
  // An address to perform PCIe write at for an interrupt.
  uint64_t message_address;
  // Data to send in PCIe write for an interrupt.
  uint32_t message_data;
  // LSB is used to mask an interrupt. Other bits are reserved.
  uint32_t vector_control;
} __attribute__((packed));
static_assert(sizeof(MsixTableEntry) == 16, "Must be 16 bytes.");

// Size in bytes addressable by a single extended page table entry.
// When kHostPageSize is 4K, this is 2MB.
static constexpr uint64_t kExtendedPageTableEntryAddressableBytes =
    kExtendedHostPageTableSizePerPage * kHostPageSize;

// Size in bytes of the configured DarwiNN extended address space range.
// Must be a multiple of |kExtendedPageTableEntryAddressableBytes|. The maximum
// addressable extended address space range is 16 GB. However, this is
// restricted to 4GB to avoid using 64 bit math in the scalar core.
// See: go/g.d/1a9uNlUCrEu43L31v_gRENgCjKW4MMA-B-L64cR8z3I4
static constexpr uint64_t kExtendedAddressSpaceStart = 0x8000000000000000L;
static constexpr uint64_t kExtendedAddressSpaceSizeBytes =
    (4096 * 1024 * 1024ULL);
static constexpr int kExtendedAddressSpacePrefixWidthBits = 32;
static_assert(
    (kExtendedAddressSpaceStart >> kExtendedAddressSpacePrefixWidthBits) ==
        ((kExtendedAddressSpaceStart + kExtendedAddressSpaceSizeBytes - 1) >>
         kExtendedAddressSpacePrefixWidthBits),
    "Extended address space range cannot span 4 GB boundaries.");
static_assert((kExtendedAddressSpaceSizeBytes %
                   kExtendedPageTableEntryAddressableBytes ==
               0),
              "Must be multiple of extended host page");

// The upper 32 bits of the extended address space segment.
static constexpr uint32_t kExtendedAddressSpacePrefix =
    kExtendedAddressSpaceStart >> kExtendedAddressSpacePrefixWidthBits;

// Simple / Extended page table entry split.
// At the minimum, simple address space needs 256 * 4kB = 1MB.
static constexpr int kMinNumSimplePageTableEntries = 256;

// At the maximum, 2048 * 2MB = 4GB is reserved for extended address space.
static constexpr int kMaxNumExtendedPageTableEntries =
    kExtendedAddressSpaceSizeBytes / kExtendedPageTableEntryAddressableBytes;

// Returns number of simple page table entries given page table size.
inline int GetNumSimplePageTableEntries(int num_page_table_entries) {
  const int num_simple_entries =
      num_page_table_entries - kMaxNumExtendedPageTableEntries;
  return std::max(num_simple_entries, kMinNumSimplePageTableEntries);
}

// Returns number of extended page table entries given page table size.
inline int GetNumExtendedPageTableEntries(int num_page_table_entries) {
  return num_page_table_entries -
         GetNumSimplePageTableEntries(num_page_table_entries);
}

// Run control settings for tiles and scalar core.
enum class RunControl {
  kMoveToIdle = 0,
  kMoveToRun = 1,
  kMoveToHalt = 2,
  kMoveToSingleStep = 3,
};

// Run status settings for tiles and scalar core.
enum class RunStatus {
  kIdle = 0,
  kRun = 1,
  kSingleStep = 2,
  kHalting = 3,
  kHalted = 4,
};

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms

#endif  // LIBS_TPU_DARWINN_DRIVER_HARDWARE_STRUCTURES_H_
