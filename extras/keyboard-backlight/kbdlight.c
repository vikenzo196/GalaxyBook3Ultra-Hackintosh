// kbdlight            -> stato
// kbdlight 0..3       -> livello scelto (attivo solo dopo "arm")
// kbdlight <comando>  -> validate | osys | sta | get | enable-pm | enable-kbd | set0..set3 | arm | disarm
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    io_service_t s = IOServiceGetMatchingService(kIOMainPortDefault, IOServiceMatching("GalaxyBookKbd"));
    if (!s) { fprintf(stderr, "GalaxyBookKbd non caricato\n"); return 1; }
    if (argc > 1) {
        kern_return_t kr;
        if (!strcmp(argv[1], "hotkey") && argc > 2) {
            int code = (int)strtol(argv[2], NULL, 0);
            CFNumberRef hn = CFNumberCreate(NULL, kCFNumberIntType, &code);
            kr = IORegistryEntrySetCFProperty(s, CFSTR("HotkeyCode"), hn); CFRelease(hn);
            goto show;
        }
        char *end; long lvl = strtol(argv[1], &end, 10);
        if (!*end && lvl >= 0 && lvl <= 3) {
            int v = (int)lvl;
            CFNumberRef n = CFNumberCreate(NULL, kCFNumberIntType, &v);
            kr = IORegistryEntrySetCFProperty(s, CFSTR("UserLevel"), n);
            CFRelease(n);
        } else {
            CFStringRef c = CFStringCreateWithCString(NULL, argv[1], kCFStringEncodingUTF8);
            kr = IORegistryEntrySetCFProperty(s, CFSTR("Command"), c);
            CFRelease(c);
        }
    show:
        if (kr != KERN_SUCCESS) { fprintf(stderr, "errore 0x%x\n", kr); return 1; }
        sleep(1);
    }
    IOObjectRelease(s);
    execlp("/bin/sh", "sh", "-c",
           "ioreg -rlw0 -c GalaxyBookKbd -d1 | grep -E '\"(Version|Status|LastCommand|Result-[a-z-]+|CSFIPath|EvalPath|ValidPath|UserLevel|HardwareLevel|DisplayOn|DisplaySync|Armed|Errors|Calls|SCAIListening|SCAINotifications|SCAINotifyCount|HotkeyCode)\" ='",
           (char *)NULL);
    return 1;
}
