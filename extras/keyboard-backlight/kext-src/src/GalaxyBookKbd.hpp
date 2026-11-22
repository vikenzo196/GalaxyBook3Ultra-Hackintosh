// Keyboard backlight for Samsung Galaxy Book (SCAI / SAM0429) on macOS.
// Protocol from Linux drivers/platform/x86/samsung-galaxybook.c.
//
// v3 safety rule: NO ACPI evaluation happens during boot. Every firmware call is
// started explicitly from user space (setProperties "Command"), one at a time, so a
// bad call can at most crash the running system once and never cause a boot loop.
#pragma once
#include <IOKit/IOService.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <IOKit/IOLocks.h>
#include <kern/thread_call.h>
#include <libkern/c++/OSArray.h>
#include <libkern/c++/OSString.h>

class GalaxyBookKbd : public IOService {
    OSDeclareDefaultStructors(GalaxyBookKbd)
public:
    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
    IOReturn setProperties(OSObject *props) override;
    IOReturn powerStateDidChangeTo(IOPMPowerFlags caps, unsigned long state, IOService *dev) override;

private:
    IOACPIPlatformDevice *acpi {nullptr};
    IOService *ecService {nullptr};
    IOService *wrangler {nullptr};
    IONotifier *ecNotifier {nullptr};
    IONotifier *wranglerNotifier {nullptr};
    IOACPIPlatformDevice *scai {nullptr};   // \_SB.SCAI nub (SAM0429): notifications + SDLS
    IONotifier *scaiNotifier {nullptr};
    IONotifier *scaiInterest {nullptr};
    thread_call_t notifyCall {nullptr};
    uint8_t notifyRing[16] {};
    uint32_t notifyCount {0};
    uint32_t hotkeyCode {0};                // SCAI notify value of Fn+F9 (0 = not set)
    uint32_t hotkeyPending {0};
    IOLock *lock {nullptr};
    thread_call_t displayCall {nullptr};
    thread_call_t commandCall {nullptr};
    OSArray *calls {nullptr};
    char pending[64] {};
    char csfiPath[48] {"CSFI"};
    int userLevel {3};
    int hwLevel {-1};
    bool displayOn {true};
    bool armed {false};   // display sync active only after a successful "init" command
    uint32_t errors {0};

    IOReturn csfi(uint8_t sasb, uint8_t gunm, uint8_t guds0, uint8_t *outGunm, uint8_t *outGuds0);
    bool enableFeature(uint8_t sasb);
    IOReturn setHw(int level);
    int getHw();
    void apply();
    void publish();
    void runCommand(const char *cmd);
    void recordCall(uint8_t sasb, uint8_t gunm, IOReturn ret, const uint8_t *reply, size_t len);
    void setResult(const char *cmd, unsigned long long value);
    static void displayCallFn(thread_call_param_t p0, thread_call_param_t p1);
    static void commandCallFn(thread_call_param_t p0, thread_call_param_t p1);
    static bool onEc(void *target, void *ref, IOService *svc, IONotifier *n);
    static bool onWrangler(void *target, void *ref, IOService *svc, IONotifier *n);
    static bool onScai(void *target, void *ref, IOService *svc, IONotifier *n);
    static IOReturn scaiMessage(void *target, void *ref, UInt32 type, IOService *provider, void *arg, vm_size_t size);
    static void notifyCallFn(thread_call_param_t p0, thread_call_param_t p1);
};
