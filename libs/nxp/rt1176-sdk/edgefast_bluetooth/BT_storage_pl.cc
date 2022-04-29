// Port of
// third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_storage_pl.c
// to use Valiant's filesystem APIs.

#include "libs/base/filesystem.h"

extern "C" {

#include "third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_storage_pl.h"

static lfs_file_t *fp[STORAGE_NUM_TYPES];
static lfs_file_t lfs_file[STORAGE_NUM_TYPES];
/* Storage File Name array */
static UCHAR *fn[STORAGE_NUM_TYPES] = {
    (UCHAR *)"btps.db",
#ifdef STORAGE_RETENTION_SUPPORT
    (UCHAR *)"btrn.db",
#endif /* STORAGE_RETENTION_SUPPORT */
};

/* Storage Signature Key array */
static UCHAR ssign[STORAGE_NUM_TYPES][STORAGE_SKEY_SIZE] = {
    {'E', 'T', 'H', 'E', 'R', 'M', 'I', 'N', 'D', 'P', 'S'}};

API_RESULT storage_open_pl(UCHAR type, UCHAR mode) {
  if (NULL != fp[type]) {
    coral::micro::filesystem::Close(fp[type]);
    fp[type] = NULL;
  }
  bool open;
  if (mode == STORAGE_OPEN_MODE_WRITE) {
    open = coral::micro::filesystem::Open(&lfs_file[type], (char *)fn[type], true);
  } else {
    open = coral::micro::filesystem::Open(&lfs_file[type], (char *)fn[type]);
  }
  if (!open) {
    return API_FAILURE;
  }

  fp[type] = &lfs_file[type];
  return API_SUCCESS;
}

API_RESULT storage_close_pl(UCHAR type, UCHAR mode) {
  if (NULL != fp[type]) {
    coral::micro::filesystem::Close(fp[type]);
    fp[type] = NULL;
  }

  return API_SUCCESS;
}

INT16 storage_write_pl(UCHAR type, void *buffer, UINT16 size) {
  INT16 nbytes;
  nbytes = 0;

  if (NULL != fp[type]) {
    nbytes = (INT16)coral::micro::filesystem::Write(fp[type], buffer, size);
  }
  return nbytes;
}

INT16 storage_read_pl(UCHAR type, void *buffer, UINT16 size) {
  INT16 nbytes = 0;
  if (NULL != fp[type]) {
    nbytes = (INT16)coral::micro::filesystem::Read(fp[type], buffer, size);
  }
  return nbytes;
}

INT16 storage_write_signature_pl(UCHAR type) {
  INT16 nbytes = 0;
  if (NULL != fp[type]) {
    nbytes = (INT16)coral::micro::filesystem::Write(fp[type], ssign[type],
                                               STORAGE_SKEY_SIZE);
  }
  return nbytes;
}

INT16 storage_read_signature_pl(UCHAR type) {
  INT16 nbytes = 0;
  UCHAR sign[STORAGE_SKEY_SIZE];
  if (NULL != fp[type]) {
    nbytes = coral::micro::filesystem::Read(fp[type], sign, STORAGE_SKEY_SIZE);
    if (BT_mem_cmp(ssign[type], sign, STORAGE_SKEY_SIZE)) {
      return -1;
    }
  }
  return nbytes;
}

void storage_bt_init_pl(void) {
  for (unsigned int i = 0; i < STORAGE_NUM_TYPES; ++i) {
    fp[i] = NULL;
  }
}

}  // extern "C"