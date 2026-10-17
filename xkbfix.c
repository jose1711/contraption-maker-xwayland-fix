/*
 * LD_PRELOAD workaround for Contraption Maker (Steam app 241240) crashing on
 * startup under Xwayland.
 *
 * The game statically links an old GLFW 3.0, whose X11 backend calls
 * XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd) and dereferences
 * the result without a NULL check. Xwayland provides no XKB geometry, so the
 * request fails, returns NULL and the game segfaults in _glfwPlatformInit.
 *
 * If the full request fails, retry it without the geometry component, which
 * GLFW does not use anyway.
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

/* Bind to the old symbol version so the prebuilt .so also loads on glibc < 2.34. */
__asm__(".symver dlsym,dlsym@GLIBC_2.2.5");

XkbDescPtr XkbGetKeyboard(Display *dpy, unsigned int which, unsigned int device)
{
    static XkbDescPtr (*real)(Display *, unsigned int, unsigned int);
    XkbDescPtr desc;

    if (!real)
        real = (XkbDescPtr (*)(Display *, unsigned int, unsigned int))
            dlsym(RTLD_NEXT, "XkbGetKeyboard");

    desc = real(dpy, which, device);
    if (!desc && (which & XkbGBN_GeometryMask))
        desc = real(dpy, which & ~XkbGBN_GeometryMask, device);
    return desc;
}
