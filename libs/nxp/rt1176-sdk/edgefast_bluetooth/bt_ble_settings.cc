// Port of
// third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_settings.c
// to use Dev Board Micro's filesystem APIs.

#include "libs/base/filesystem.h"

extern "C" {

#include "third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_settings.h"

#define SETTINGS_NAME_SEPARATOR '/'
Z_STRUCT_SECTION_DEFINE(settings_handler_static);
int settings_init(void) { return 0; }

int settings_load_one(const char *name, void *value, size_t val_len) {
  lfs_file_t readFile;
  bool success = coral::micro::filesystem::Open(&readFile, name);
  if (!success) {
    return -1;
  }

  int len = coral::micro::filesystem::Size(&readFile);
  if (value != NULL) {
    len = coral::micro::filesystem::Read(&readFile, value, val_len);
  }
  coral::micro::filesystem::Close(&readFile);
  return len;
}

static ssize_t settings_read_data(void *cb_arg, void *data, size_t len) {
  return settings_load_one((const char *)cb_arg, data, len);
}

int settings_load_subtree_direct(const char *subtree,
                                 settings_load_direct_cb cb, void *param) {
    int err;
    struct lfs_info info;
    lfs_dir_t dir;
    char name[SETTINGS_KEY_MAX + 1];

    if (NULL == cb)
    {
        return -EINVAL;
    }

    memset((void *)&dir, 0, sizeof(dir));
    bool success = coral::micro::filesystem::Stat(subtree, &info);
    if (!success) {
        return -1;
    }
    if (info.type == LFS_TYPE_DIR)
    {
        success = coral::micro::filesystem::DirOpen(&dir, subtree);
        if (!success) {
            return -1;
        }

        /* iterate until end of directory */
        while ((err = coral::micro::filesystem::DirRead(&dir, &info)) != 0)
        {
            if ((!strcmp(info.name, ".")) || (!strcmp(info.name, "..")))
            {
                continue;
            }
            memcpy(name, subtree, strlen(subtree));
            name[strlen(subtree)] = SETTINGS_NAME_SEPARATOR;
            memcpy(&name[strlen(subtree) + 1], info.name, sizeof(name) - strlen(subtree) - 1);
            name[sizeof(name) - 1] = '\0';
            err = cb(info.name, settings_load_one(name, NULL, 0), settings_read_data, name, param);
        }

        coral::micro::filesystem::DirClose(&dir);
    }
    else
    {
        err = cb(NULL, settings_load_one(subtree, NULL, 0), settings_read_data, (void*)subtree, param);
    }

    return err;
}

static int settings_load_subtree_scan(struct settings_handler_static *set,
                                      const char *key, int level) {
  struct lfs_info info;
  lfs_dir_t dir;
  char name[SETTINGS_KEY_MAX + 1];
  const char *next;
  int err = -EINVAL;
  bool ret;

  memset((void *)&dir, 0, sizeof(dir));
  ret = coral::micro::filesystem::Stat(key, &info);
  if (!ret) {
    return -EIO;
  }
  if (info.type == LFS_TYPE_DIR) {
    if (level > 2) {
      return err;
    }

    ret = coral::micro::filesystem::DirOpen(&dir, key);
    if (!ret) {
      return -EIO;
    }
    while ((err = coral::micro::filesystem::DirRead(&dir, &info)) != 0) {
      if ((!strcmp(info.name, ".")) || (!strcmp(info.name, ".."))) {
        continue;
      }
      memcpy(name, key, strlen(key));
      name[strlen(key)] = SETTINGS_NAME_SEPARATOR;
      memcpy(&name[strlen(key) + 1], info.name,
             MIN(strlen(info.name) + 1, (sizeof(name) - strlen(key) - 1)));
      name[sizeof(name) - 1] = '\0';
      err = settings_load_subtree_scan(set, name, level + 1);
    }
    ret = coral::micro::filesystem::DirClose(&dir);
    if (!ret) {
      return -EIO;
    }
  } else {
    err = settings_name_steq(key, set->name, &next);
    if (0 != err) {
      err = set->h_set(next, settings_load_one(key, NULL, 0),
                       settings_read_data, (void *)key);
    }
  }
  return err;
}

int settings_load_subtree(const char *subtree) {
  Z_STRUCT_SECTION_FOREACH(settings_handler_static, set) {
    if (NULL == set->h_set) {
      continue;
    }

    if ((NULL != subtree) && (!settings_name_steq(set->name, subtree, NULL))) {
      continue;
    }
    (void)settings_load_subtree_scan(set, set->name, 1);
  }

  return settings_commit_subtree(subtree);
}

static int settings_has_spacer(const char *name, const char spacer, int count) {
  int times = 0;
  for (size_t index = 0; index < strlen(name); index++) {
    if (spacer == name[index]) {
      times++;
      if (times == count) {
        return (int)index;
      }
    }
  }
  return -1;
}

static int settings_check_create_path(const char *name) {
  char dname[SETTINGS_KEY_MAX + 1];
  int times = 1;
  int index;

  index = settings_has_spacer(name, SETTINGS_NAME_SEPARATOR, times++);
  while (index > 0) {
    memcpy(dname, name, index);
    dname[index] = 0;
    bool success = coral::micro::filesystem::MakeDirs(dname);
    if (!success) {
      return -1;
    }
    index = settings_has_spacer(name, SETTINGS_NAME_SEPARATOR, times++);
  }
  return 0;
}

int settings_load(void) { return settings_load_subtree(NULL); }

int settings_save_one(const char *name, const void *value, size_t val_len) {
  int err;
  lfs_file_t writeFile;
  err = settings_check_create_path(name);
  if (err < 0) {
    return err;
  }

  bool success = coral::micro::filesystem::Open(&writeFile, name, true);
  if (!success) {
    return -1;
  }

  int len = coral::micro::filesystem::Write(&writeFile, value, val_len);
  success = coral::micro::filesystem::Close(&writeFile);
  if (!success) {
    return -1;
  }

  return len;
}

int settings_name_steq(const char *name, const char *key, const char **next) {
  if (next) {
    *next = NULL;
  }

  if ((!name) || (!key)) {
    return 0;
  }

  /* name might come from flash directly, in flash the name would end
   * with '=' or '\0' depending how storage is done. Flash reading is
   * limited to what can be read
   */

  while ((*key != '\0') && (*key == *name) && (*name != '\0')) {
    key++;
    name++;
  }

  if (*key != '\0') {
    return 0;
  }

  if (*name == SETTINGS_NAME_SEPARATOR) {
    if (next) {
      *next = name + 1;
    }
    return 1;
  }

  if ((*name == '\0')) {
    return 1;
  }

  return 0;
}

int settings_name_next(const char *name, const char **next) {
  int rc = 0;

  if (next) {
    *next = NULL;
  }

  if (!name) {
    return 0;
  }

  /* name might come from flash directly, in flash the name would end
   * with '=' or '\0' depending how storage is done. Flash reading is
   * limited to what can be read
   */
  while ((*name != '\0') && (*name != SETTINGS_NAME_SEPARATOR)) {
    rc++;
    name++;
  }

  if (*name == SETTINGS_NAME_SEPARATOR) {
    if (next) {
      *next = name + 1;
    }
    return rc;
  }

  return rc;
}

int settings_delete(const char *name) {
  if (name) {
      if (coral::micro::filesystem::Remove(name)) {
          return 0;
      }
  }
  return -1;
}

int settings_commit_subtree(const char *subtree) {
  int err;
  /* h_commit callback */
  Z_STRUCT_SECTION_FOREACH(settings_handler_static, set) {
    if ((NULL != subtree) && (!settings_name_steq(set->name, subtree, NULL))) {
      continue;
    }

    if (NULL == set->h_commit) {
      continue;
    }

    err = set->h_commit();
  }
  (void)err;
  return 0;
}

int settings_commit(void) { return settings_commit_subtree(NULL); }
}  // extern "C"
