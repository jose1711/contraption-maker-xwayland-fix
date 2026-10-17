# Contraption Maker: Xwayland crash fix

A tiny `LD_PRELOAD` library that fixes the native Linux version of
[Contraption Maker](https://store.steampowered.com/app/241240/) (Steam app 241240)
crashing right at startup on Wayland desktops (GNOME, KDE Plasma, Sway, Hyprland, …).

## Symptoms

You click **Play** in Steam, the game shows as running for about a second and
then stops. No window ever appears. The game log
(`~/.config/spotkincm/contraptionmaker.log`) ends with:

```
[cocos] Creating with full screen.
```

and `coredumpctl` shows a `SIGSEGV` with this backtrace:

```
#0  _glfwPlatformInit (ContraptionMaker)
#1  glfwInit (ContraptionMaker)
#2  cocos2d::GLViewImpl::GLViewImpl()
#3  cocos2d::GLViewImpl::createWithFullScreen(std::string const&)
#4  AppDelegate::applicationDidFinishLaunching()
```

## Cause

The game statically links an old version of GLFW (3.0). During initialization
its X11 backend calls

```c
XkbGetKeyboard(display, XkbAllComponentsMask, XkbUseCoreKbd)
```

and uses the result without checking for `NULL`. Xwayland does not provide XKB
keyboard *geometry*, so a request that includes the geometry component fails
and returns `NULL`, and the game crashes.

This library wraps `XkbGetKeyboard`: if the full request fails, it repeats it
without the geometry component. GLFW only needs the key names, so the game
works normally.

## Install

### Option A: prebuilt library

1. Download `xkbfix.so` from the
   [latest release](https://github.com/jose1711/contraption-maker-xwayland-fix/releases/latest).
2. Put it somewhere permanent, for example
   `~/.local/share/contraption-fix/xkbfix.so`.

### Option B: build it yourself

You need `gcc`, `make` and the libX11 development headers
(`libx11-dev` on Debian/Ubuntu, `libX11-devel` on Fedora, `libx11` on Arch).

```sh
git clone https://github.com/jose1711/contraption-maker-xwayland-fix.git
cd contraption-maker-xwayland-fix
make
mkdir -p ~/.local/share/contraption-fix
cp xkbfix.so ~/.local/share/contraption-fix/
```

### Set the launch options

In Steam: right-click **Contraption Maker** → **Properties** → **Launch Options**
and enter (replace `YOURNAME` with your user name; use an absolute path, not `~`):

```
LD_PRELOAD=/home/YOURNAME/.local/share/contraption-fix/xkbfix.so %command%
```

The library has to be in your home directory (or another location that Steam
Linux Runtime can see). Locations like `/tmp` may be mounted `noexec`, and the
library then fails to load.

## Notes

- Tested on Arch Linux with a Wayland session, running the game through
  Steam Linux Runtime 1.0 (scout-on-soldier), the default for this game.
- The library only changes `XkbGetKeyboard`, and only when the original call
  fails, so it does nothing on a regular X11 session.
- The same bug probably affects other games built with cocos2d-x 3.x and its
  bundled GLFW 3.0 on Linux. If a game crashes in `_glfwPlatformInit` under
  Wayland, this library may help there too.

## License

MIT
