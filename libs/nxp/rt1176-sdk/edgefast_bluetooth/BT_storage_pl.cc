// Port of
// third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_storage_pl.c

#include "libs/base/filesystem.h"

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

using coralmicro::Lfs;

extern "C" {

#include "third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_storage_pl.h"

/* Storage File Handle array */
static lfs_file_t* fp[STORAGE_NUM_TYPES];
static lfs_file_t lfs_file[STORAGE_NUM_TYPES];

/* Storage File Name array */
static UCHAR* fn[STORAGE_NUM_TYPES] = {
    (UCHAR*)"btps.db",
#ifdef STORAGE_RETENTION_SUPPORT
    (UCHAR*)"btrn.db",
#endif /* STORAGE_RETENTION_SUPPORT */
};

/* Storage Signature Key array */
static UCHAR ssign[STORAGE_NUM_TYPES][STORAGE_SKEY_SIZE] = {
    {'E', 'T', 'H', 'E', 'R', 'M', 'I', 'N', 'D', 'P', 'S'}};

API_RESULT storage_open_pl(UCHAR type, UCHAR mode) {
  if (NULL != fp[type]) {
    (void)lfs_file_close(Lfs(), fp[type]);
    fp[type] = NULL;
  }
  /* Set the file access mode */
  int rw =
      (int)((STORAGE_OPEN_MODE_WRITE == mode) ? (LFS_O_WRONLY | LFS_O_CREAT)
                                              : LFS_O_RDONLY);

  int err = lfs_file_open(Lfs(), &lfs_file[type], (CHAR*)fn[type], rw);

  if (err < 0) {
    return API_FAILURE;
  }

  fp[type] = &lfs_file[type];

  return API_SUCCESS;
}

API_RESULT storage_close_pl(UCHAR type, UCHAR mode) {
  BT_IGNORE_UNUSED_PARAM(mode);
  if (NULL != fp[type]) {
    (void)lfs_file_close(Lfs(), fp[type]);
    fp[type] = NULL;
  }
  return API_SUCCESS;
}

INT16 storage_write_pl(UCHAR type, void* buffer, UINT16 size) {
  INT16 nbytes = 0;
  if (NULL != fp[type]) {
    nbytes = (INT16)lfs_file_write(Lfs(), fp[type], buffer, size);
  }
  return nbytes;
}

INT16 storage_read_pl(UCHAR type, void* buffer, UINT16 size) {
  INT16 nbytes = 0;
  if (NULL != fp[type]) {
    nbytes = (INT16)lfs_file_read(Lfs(), fp[type], buffer, size);
  }
  return nbytes;
}

INT16 storage_write_signature_pl(UCHAR type) {
  INT16 nbytes = 0;
  if (NULL != fp[type]) {
    nbytes =
        (INT16)lfs_file_write(Lfs(), fp[type], ssign[type], STORAGE_SKEY_SIZE);
  }
  return nbytes;
}

INT16 storage_read_signature_pl(UCHAR type) {
  INT16 nbytes = 0;
  UCHAR sign[STORAGE_SKEY_SIZE];

  if (NULL != fp[type]) {
    nbytes = lfs_file_read(Lfs(), fp[type], sign, STORAGE_SKEY_SIZE);

    if (BT_mem_cmp(ssign[type], sign, STORAGE_SKEY_SIZE)) {
      return -1;
    }
  }
  return nbytes;
}

void storage_bt_init_pl() {
  for (UCHAR i = 0; i < STORAGE_NUM_TYPES; i++) {
    fp[i] = NULL;
  }
}

void storage_bt_shutdown_pl() {
  for (UCHAR i = 0; i < STORAGE_NUM_TYPES; i++) {
    if (NULL != fp[i]) {
      lfs_file_close(Lfs(), fp[i]);
      fp[i] = NULL;
    }
  }
}

}  // extern "C"
