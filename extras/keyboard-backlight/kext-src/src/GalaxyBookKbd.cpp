#include "GalaxyBookKbd.hpp"
#include <IOKit/IOLib.h>
#include <libkern/c++/OSData.h>
#include <libkern/c++/OSNumber.h>
#include <libkern/c++/OSDictionary.h>
#include <pexpert/pexpert.h>

OSDefineMetaClassAndStructors(GalaxyBookKbd, IOService)

#define LOG(fmt, ...) IOLog("GalaxyBookKbd: " fmt "\n", ##__VA_ARGS__)

static constexpr uint16_t kSAFN = 0x5843;
static constexpr uint8_t kSasbKbd = 0x78, kSasbPower = 0x7a;
static constexpr uint8_t kGunmGet = 0x81, kGunmSet = 0x82;
static constexpr uint8_t kGunmEnable = 0xbb, kGudsEnable = 0xaa;
static constexpr size_t kLen = 0x15;
static constexpr int kMaxLevel = 3;
static constexpr unsigned long long kErr = 0xdead0000ULL;

void GalaxyBookKbd::recordCall(uint8_t sasb, uint8_t gunm, IOReturn ret, const uint8_t *reply, size_t len) {
    // Never mutate a collection that is already published in the registry (kernel panics:
    // "Trying to change a collection in the registry"). Build a fresh array every time.
    OSArray *next = calls ? OSArray::withArray(calls, 16) : OSArray::withCapacity(8);
    if (!next) return;
    while (next->getCount() >= 16) next->removeObject(0);
    OSDictionary *d = OSDictionary::withCapacity(4);
    if (!d) { next->release(); return; }
    OSNumber *n;
    if ((n = OSNumber::withNumber(sasb, 8))) { d->setObject("sasb", n); n->release(); }
    if ((n = OSNumber::withNumber(gunm, 8))) { d->setObject("gunm", n); n->release(); }
    if ((n = OSNumber::withNumber(static_cast<unsigned long long>(static_cast<uint32_t>(ret)), 32))) { d->setObject("ret", n); n->release(); }
    if (reply && len) { OSData *r = OSData::withBytes(reply, static_cast<unsigned int>(len)); if (r) { d->setObject("reply", r); r->release(); } }
    next->setObject(d);
    d->release();
    OSSafeReleaseNULL(calls);
    calls = next;
    setProperty("Calls", calls);
}

IOReturn GalaxyBookKbd::csfi(uint8_t sasb, uint8_t gunm, uint8_t guds0, uint8_t *outGunm, uint8_t *outGuds0) {
    if (!acpi) return kIOReturnNotReady;
    uint8_t buf[kLen] = {};
    buf[0] = kSAFN & 0xff; buf[1] = kSAFN >> 8;
    buf[2] = sasb; buf[3] = 0;
    buf[5] = gunm; buf[6] = guds0;
    OSData *in = OSData::withBytes(buf, sizeof(buf));
    if (!in) return kIOReturnNoMemory;
    OSObject *params[1] = {in};
    OSObject *res = nullptr;
    IOReturn ret = acpi->evaluateObject(csfiPath, &res, params, 1);
    in->release();
    OSData *out = OSDynamicCast(OSData, res);
    recordCall(sasb, gunm, ret, out ? static_cast<const uint8_t *>(out->getBytesNoCopy()) : nullptr, out ? out->getLength() : 0);
    if (ret != kIOReturnSuccess) { errors++; OSSafeReleaseNULL(res); return ret; }
    if (!out || out->getLength() < 7) { errors++; OSSafeReleaseNULL(res); return kIOReturnError; }
    const uint8_t *o = static_cast<const uint8_t *>(out->getBytesNoCopy());
    if (o[4] != 0xaa || o[5] == 0xff) { errors++; res->release(); return kIOReturnError; }
    if (outGunm) *outGunm = o[5];
    if (outGuds0) *outGuds0 = o[6];
    res->release();
    return kIOReturnSuccess;
}

bool GalaxyBookKbd::enableFeature(uint8_t sasb) {
    uint8_t g = 0, d = 0;
    if (csfi(sasb, kGunmEnable, kGudsEnable, &g, &d) != kIOReturnSuccess) return false;
    return g == 0xdd || d == 0xcc;
}

int GalaxyBookKbd::getHw() {
    uint8_t g = 0;
    if (csfi(kSasbKbd, kGunmGet, 0, &g, nullptr) != kIOReturnSuccess) return -1;
    return g;
}

IOReturn GalaxyBookKbd::setHw(int level) {
    if (level < 0) level = 0;
    if (level > kMaxLevel) level = kMaxLevel;
    IOReturn r = csfi(kSasbKbd, kGunmSet, static_cast<uint8_t>(level), nullptr, nullptr);
    if (r == kIOReturnSuccess) hwLevel = level;
    return r;
}

void GalaxyBookKbd::publish() {
    setProperty("UserLevel", static_cast<unsigned long long>(userLevel), 32);
    setProperty("HardwareLevel", static_cast<unsigned long long>(hwLevel < 0 ? 0xff : hwLevel), 32);
    setProperty("DisplayOn", displayOn);
    setProperty("Armed", armed);
    setProperty("Errors", errors, 32);
}

void GalaxyBookKbd::setResult(const char *cmd, unsigned long long value) {
    char key[48];
    snprintf(key, sizeof(key), "Result-%s", cmd);
    setProperty(key, value, 64);
}

void GalaxyBookKbd::apply() {
    IOLockLock(lock);
    if (armed) {
        int want = displayOn ? userLevel : 0;
        if (want != hwLevel) setHw(want);
    }
    publish();
    IOLockUnlock(lock);
}

// One firmware call per command. Nothing here runs unless user space asks for it.
void GalaxyBookKbd::runCommand(const char *cmd) {
    IOLockLock(lock);
    setProperty("LastCommand", cmd);
    if (!acpi) {
        setProperty("Status", "no ACPI device yet");
    } else if (!strncmp(cmd, "path:", 5) && cmd[5]) {
        strlcpy(csfiPath, cmd + 5, sizeof(csfiPath));
        setProperty("CSFIPath", csfiPath);
        setResult("path", static_cast<uint32_t>(acpi->validateObject(csfiPath)));
    } else if (!strncmp(cmd, "evali:", 6) && cmd[6]) {
        UInt32 v = 0;
        IOReturn r = acpi->evaluateInteger(cmd + 6, &v);
        setProperty("EvalPath", cmd + 6);
        setResult("evali", r == kIOReturnSuccess ? v : (kErr | (r & 0xffff)));
    } else if (!strncmp(cmd, "valid:", 6) && cmd[6]) {
        setProperty("ValidPath", cmd + 6);
        setResult("valid", static_cast<uint32_t>(acpi->validateObject(cmd + 6)));
    } else if (!strcmp(cmd, "validate")) {
        IOReturn r = acpi->validateObject(csfiPath);
        setResult(cmd, static_cast<uint32_t>(r));
    } else if (!strcmp(cmd, "osys")) {
        UInt32 v = 0;
        IOReturn r = acpi->evaluateInteger("GOSY", &v);
        setResult(cmd, r == kIOReturnSuccess ? v : (kErr | (r & 0xffff)));
    } else if (!strcmp(cmd, "sta")) {
        UInt32 v = 0;
        IOReturn r = acpi->evaluateInteger("GSTA", &v);
        setResult(cmd, r == kIOReturnSuccess ? v : (kErr | (r & 0xffff)));
    } else if (!strcmp(cmd, "get")) {
        setResult(cmd, static_cast<unsigned long long>(static_cast<long long>(getHw())));
    } else if (!strcmp(cmd, "enable-pm")) {
        setResult(cmd, enableFeature(kSasbPower));
    } else if (!strcmp(cmd, "enable-kbd")) {
        setResult(cmd, enableFeature(kSasbKbd));
    } else if (!strncmp(cmd, "set", 3) && cmd[3] >= '0' && cmd[3] <= '3' && !cmd[4]) {
        setResult("set", static_cast<uint32_t>(setHw(cmd[3] - '0')));
    } else if (!strcmp(cmd, "arm")) {
        int cur = getHw();
        if (cur >= 0 && cur <= kMaxLevel) { hwLevel = cur; if (cur > 0) userLevel = cur; armed = true; }
        setResult(cmd, armed);
    } else if (!strcmp(cmd, "notify-on") || !strcmp(cmd, "notify-off")) {
        // \_SB.SCAI.SDLS(1) sets DVLD: the EC then reports Samsung hotkeys as Notify(SCAI, code).
        UInt32 v = 0; IOReturn r = kIOReturnNotReady;
        if (scai) { OSObject *arg = OSNumber::withNumber(cmd[8] == 'n' ? 1ULL : 0ULL, 32);
            OSObject *res = nullptr; r = scai->evaluateObject("SDLS", &res, &arg, 1);
            if (OSNumber *rn = OSDynamicCast(OSNumber, res)) v = rn->unsigned32BitValue();
            OSSafeReleaseNULL(res); OSSafeReleaseNULL(arg); }
        setResult(cmd, r == kIOReturnSuccess ? v : (kErr | (r & 0xffff)));
    } else if (!strcmp(cmd, "disarm")) {
        armed = false;
        setResult(cmd, 1);
    } else {
        setProperty("Status", "unknown command");
    }
    publish();
    IOLockUnlock(lock);
    apply();
}

void GalaxyBookKbd::commandCallFn(thread_call_param_t p0, thread_call_param_t) {
    auto *self = static_cast<GalaxyBookKbd *>(p0);
    char cmd[sizeof(self->pending)];
    IOLockLock(self->lock);
    strlcpy(cmd, self->pending, sizeof(cmd));
    self->pending[0] = 0;
    IOLockUnlock(self->lock);
    if (cmd[0]) self->runCommand(cmd);
}

void GalaxyBookKbd::displayCallFn(thread_call_param_t p0, thread_call_param_t) {
    static_cast<GalaxyBookKbd *>(p0)->apply();
}

IOReturn GalaxyBookKbd::powerStateDidChangeTo(IOPMPowerFlags, unsigned long state, IOService *dev) {
    if (dev == wrangler) {
        // IODisplayWrangler: 4 = on, 3 = dim, <= 2 = display off
        bool on = state >= 3;
        if (on != displayOn) {
            displayOn = on;
            if (displayCall) thread_call_enter(displayCall);
        }
    }
    return kIOPMAckImplied;
}

IOReturn GalaxyBookKbd::setProperties(OSObject *props) {
    OSDictionary *d = OSDynamicCast(OSDictionary, props);
    if (!d) return kIOReturnBadArgument;
    if (OSString *c = OSDynamicCast(OSString, d->getObject("Command"))) {
        if (c->getLength() >= sizeof(pending)) return kIOReturnBadArgument;
        IOLockLock(lock);
        strlcpy(pending, c->getCStringNoCopy(), sizeof(pending));
        IOLockUnlock(lock);
        thread_call_enter(commandCall);
        return kIOReturnSuccess;
    }
    if (OSNumber *hk = OSDynamicCast(OSNumber, d->getObject("HotkeyCode"))) {
        hotkeyCode = hk->unsigned32BitValue() & 0xff;
        setProperty("HotkeyCode", hotkeyCode, 32);
        return kIOReturnSuccess;
    }
    if (OSNumber *n = OSDynamicCast(OSNumber, d->getObject("UserLevel"))) {
        int lvl = static_cast<int>(n->unsigned32BitValue());
        if (lvl > kMaxLevel) lvl = kMaxLevel;
        IOLockLock(lock);
        userLevel = lvl;
        IOLockUnlock(lock);
        if (displayCall) thread_call_enter(displayCall);
        return kIOReturnSuccess;
    }
    return kIOReturnUnsupported;
}

bool GalaxyBookKbd::onEc(void *target, void *, IOService *svc, IONotifier *n) {
    auto *self = static_cast<GalaxyBookKbd *>(target);
    if (self->ecService) return true;
    svc->retain();
    self->ecService = svc;
    self->acpi = OSDynamicCast(IOACPIPlatformDevice, svc);   // stored only, no evaluation
    n->remove();
    self->ecNotifier = nullptr;
    self->setProperty("Status", self->acpi ? "idle on GBKB (no firmware calls at boot)" : "GBKB is not ACPI");
    return true;
}

bool GalaxyBookKbd::onWrangler(void *target, void *, IOService *svc, IONotifier *n) {
    auto *self = static_cast<GalaxyBookKbd *>(target);
    if (self->wrangler) return true;
    svc->retain();
    self->wrangler = svc;
    n->remove();
    self->wranglerNotifier = nullptr;
    svc->registerInterestedDriver(self);
    self->setProperty("DisplaySync", true);
    return true;
}

bool GalaxyBookKbd::onScai(void *target, void *, IOService *svc, IONotifier *n) {
    auto *self = static_cast<GalaxyBookKbd *>(target);
    auto *dev = OSDynamicCast(IOACPIPlatformDevice, svc);
    if (self->scai || !dev) return true;
    dev->retain();
    self->scai = dev;
    n->remove();
    self->scaiNotifier = nullptr;
    // Listening only: no ACPI evaluation here.
    self->scaiInterest = dev->registerInterest(gIOGeneralInterest, scaiMessage, self);
    self->setProperty("SCAIListening", self->scaiInterest != nullptr);
    return true;
}

IOReturn GalaxyBookKbd::scaiMessage(void *target, void *, UInt32 type, IOService *, void *arg, vm_size_t) {
    auto *self = static_cast<GalaxyBookKbd *>(target);
    if (type != kIOACPIMessageDeviceNotification || !arg) return kIOReturnSuccess;
    const uint32_t code = *static_cast<UInt32 *>(arg) & 0xff;
    const uint32_t n = __atomic_fetch_add(&self->notifyCount, 1, __ATOMIC_RELAXED);
    self->notifyRing[n % 16] = static_cast<uint8_t>(code);
    if (self->hotkeyCode && code == self->hotkeyCode) __atomic_fetch_add(&self->hotkeyPending, 1, __ATOMIC_RELAXED);
    if (self->notifyCall) thread_call_enter(self->notifyCall);
    return kIOReturnSuccess;
}

void GalaxyBookKbd::notifyCallFn(thread_call_param_t p0, thread_call_param_t) {
    auto *self = static_cast<GalaxyBookKbd *>(p0);
    const uint32_t n = __atomic_load_n(&self->notifyCount, __ATOMIC_RELAXED);
    uint8_t last[16]; unsigned k = n < 16 ? n : 16;
    for (unsigned i = 0; i < k; ++i) last[i] = self->notifyRing[(n - k + i) % 16];
    if (OSData *d = OSData::withBytes(last, k)) { self->setProperty("SCAINotifications", d); d->release(); }
    self->setProperty("SCAINotifyCount", n, 32);
    uint32_t presses = __atomic_exchange_n(&self->hotkeyPending, 0, __ATOMIC_RELAXED);
    if (!presses) return;
    IOLockLock(self->lock);
    if (self->armed) {
        // Fn+F9 cycles 0 -> 1 -> 2 -> 3 -> 0, like the Windows Samsung Settings behaviour.
        int lvl = self->userLevel;
        while (presses--) lvl = lvl >= kMaxLevel ? 0 : lvl + 1;
        self->userLevel = lvl;
    }
    IOLockUnlock(self->lock);
    self->apply();
}

bool GalaxyBookKbd::start(IOService *provider) {
    char dummy[8];
    if (PE_parse_boot_argn("-gbkbdoff", dummy, sizeof(dummy))) return false;
    if (!IOService::start(provider)) return false;
    lock = IOLockAlloc();
    displayCall = thread_call_allocate(displayCallFn, this);
    commandCall = thread_call_allocate(commandCallFn, this);
    notifyCall = thread_call_allocate(notifyCallFn, this);
    if (!lock || !displayCall || !commandCall || !notifyCall) return false;
    setProperty("Version", "7 (GBKB bridge, runtime commands only, SCAI hotkey listener)");
    setProperty("CSFIPath", csfiPath);
    setProperty("Status", "waiting for GBKB (SSDT-GBKB)");
    setProperty("DisplaySync", false);
    publish();
    OSDictionary *em = nameMatching("GBKB0000");   // SSDT-GBKB bridge to \_SB.SCAI
    OSDictionary *wm = serviceMatching("IODisplayWrangler");
    if (em) { ecNotifier = addMatchingNotification(gIOFirstPublishNotification, em, onEc, this); em->release(); }
    if (wm) { wranglerNotifier = addMatchingNotification(gIOFirstPublishNotification, wm, onWrangler, this); wm->release(); }
    OSDictionary *sm = nameMatching("SAM0429");   // \_SB.SCAI: Samsung hotkey notifications
    if (sm) { scaiNotifier = addMatchingNotification(gIOFirstPublishNotification, sm, onScai, this); sm->release(); }
    registerService();
    return true;
}

void GalaxyBookKbd::stop(IOService *provider) {
    if (ecNotifier) { ecNotifier->remove(); ecNotifier = nullptr; }
    if (wranglerNotifier) { wranglerNotifier->remove(); wranglerNotifier = nullptr; }
    if (scaiNotifier) { scaiNotifier->remove(); scaiNotifier = nullptr; }
    if (scaiInterest) { scaiInterest->remove(); scaiInterest = nullptr; }
    if (notifyCall) { thread_call_cancel(notifyCall); thread_call_free(notifyCall); notifyCall = nullptr; }
    OSSafeReleaseNULL(scai);
    if (wrangler) { wrangler->deRegisterInterestedDriver(this); OSSafeReleaseNULL(wrangler); }
    if (commandCall) { thread_call_cancel(commandCall); thread_call_free(commandCall); commandCall = nullptr; }
    if (displayCall) { thread_call_cancel(displayCall); thread_call_free(displayCall); displayCall = nullptr; }
    acpi = nullptr;
    OSSafeReleaseNULL(ecService);
    OSSafeReleaseNULL(calls);
    if (lock) { IOLockFree(lock); lock = nullptr; }
    IOService::stop(provider);
}
