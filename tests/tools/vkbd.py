#!/usr/bin/env python3
"""A uinput keyboard, for driving the key path on a device by hand.

Run it ON the device. It creates a virtual keyboard that advertises EV_REP, so
holding a key here produces the input core's own auto-repeat - the same thing a
real keyboard's driver produces, which is what the compositor expects to receive
("auto repeat key events are supposed to be handled by input drivers", says
WebOSKeyboard). Qt's evdevkeyboard plugin finds it by discovery, so the
compositor picks it up with no configuration.

It exists because the questions that matter here are not answerable by reading:
whether a held key repeats, whether a modifier survives the input method's
keyboard grab, whether a shortcut reaches the application at all. Each was
settled by injecting and then counting what came out the other end. What proved
key repeat was broken, for instance:

    fd = open_device()
    hold(fd, 'c', 1.5)          # then count the presses maliit logged
    close_device(fd)

A 1.5s hold put thirteen repeats on the wire and one character in the field. With
the compositor advertising a repeat rate instead of 0, the same hold reaches
maliit fourteen times.

The virtual device lands under /devices/virtual/input, which MImHwKeyboardTracker
deliberately ignores - so it will not be mistaken for a real keyboard and will not
take the on-screen keyboard away while you are testing.

Not part of "make check": it needs /dev/uinput, root, and a running compositor to
be worth anything.
"""
import fcntl, os, struct, sys, time

UINPUT_IOCTL_BASE = ord('U')
UI_DEV_CREATE  = (UINPUT_IOCTL_BASE << 8) | 1
UI_DEV_DESTROY = (UINPUT_IOCTL_BASE << 8) | 2
# _IOW('U', 100, struct uinput_setup) - 92 bytes
UI_DEV_SETUP   = 0x405c5503
UI_SET_EVBIT   = 0x40045564
UI_SET_KEYBIT  = 0x40045565

EV_SYN, EV_KEY, EV_REP = 0x00, 0x01, 0x14
SYN_REPORT = 0
REP_DELAY, REP_PERIOD = 0, 1

# A plain US letter row plus the modifiers and keys the tests need.
KEYS = {
    'a':30,'b':48,'c':46,'d':32,'e':18,'f':33,'g':34,'h':35,'i':23,'j':36,
    'k':37,'l':38,'m':50,'n':49,'o':24,'p':25,'q':16,'r':19,'s':31,'t':20,
    'u':22,'v':47,'w':17,'x':45,'y':21,'z':44,
    'enter':28,'backspace':14,'tab':15,'esc':1,'space':57,
    'ctrl':29,'shift':42,'alt':56,'meta':125,
    'left':105,'right':106,'up':103,'down':108,
}

def emit(fd, typ, code, val):
    # struct input_event on 64-bit: timeval(16) + type,code(2+2) + value(4)
    os.write(fd, struct.pack('QQHHi', 0, 0, typ, code, val))

def syn(fd):
    emit(fd, EV_SYN, SYN_REPORT, 0)

def open_device(name=b'luneos-test-keyboard'):
    fd = os.open('/dev/uinput', os.O_WRONLY | os.O_NONBLOCK)
    fcntl.ioctl(fd, UI_SET_EVBIT, EV_KEY)
    fcntl.ioctl(fd, UI_SET_EVBIT, EV_SYN)
    fcntl.ioctl(fd, UI_SET_EVBIT, EV_REP)
    for code in sorted(set(KEYS.values())):
        fcntl.ioctl(fd, UI_SET_KEYBIT, code)
    # struct uinput_setup { input_id id; char name[80]; __u32 ff_effects_max; }
    # input_id = bustype, vendor, product, version (4 x __u16)
    setup = struct.pack('HHHH80sI', 0x03, 0x1234, 0x5678, 1, name, 0)
    fcntl.ioctl(fd, UI_DEV_SETUP, setup)
    fcntl.ioctl(fd, UI_DEV_CREATE)
    # udev has to see it and the compositor has to open it
    time.sleep(1.5)
    return fd

def close_device(fd):
    fcntl.ioctl(fd, UI_DEV_DESTROY)
    os.close(fd)

def tap(fd, key, hold=0.05):
    code = KEYS[key]
    emit(fd, EV_KEY, code, 1); syn(fd)
    time.sleep(hold)
    emit(fd, EV_KEY, code, 0); syn(fd)
    time.sleep(0.05)

def hold(fd, key, seconds):
    """Press and keep held, so the input core's own repeat timer fires."""
    code = KEYS[key]
    emit(fd, EV_KEY, code, 1); syn(fd)
    time.sleep(seconds)
    emit(fd, EV_KEY, code, 0); syn(fd)
    time.sleep(0.05)

def combo(fd, mods, key):
    codes = [KEYS[m] for m in mods]
    for c in codes:
        emit(fd, EV_KEY, c, 1)
    syn(fd)
    time.sleep(0.03)
    emit(fd, EV_KEY, KEYS[key], 1); syn(fd)
    time.sleep(0.05)
    emit(fd, EV_KEY, KEYS[key], 0); syn(fd)
    for c in reversed(codes):
        emit(fd, EV_KEY, c, 0)
    syn(fd)
    time.sleep(0.05)
