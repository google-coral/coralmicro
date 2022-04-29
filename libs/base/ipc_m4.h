#ifndef __LIBS_BASE_IPC_M4_H__
#define __LIBS_BASE_IPC_M4_H__

#include "libs/base/ipc.h"
#include <functional>

namespace coral::micro {
class IPCM4 : public IPC {
  public:
    static IPCM4* GetSingleton();

    void Init() override;
  protected:
    void RxTaskFn() override;
  private:
    void HandleSystemMessage(const ipc::SystemMessage& message) override;
};
}  // namespace coral::micro

#endif  // __LIBS_BASE_IPC_M4_H__
