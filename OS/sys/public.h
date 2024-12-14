#ifndef _PUBLIC_H
#define _PUBLIC_H

#define IOCTL_INDEX             0x800

#define IOCTL_KBFILTR_GET_KEYBOARD_ATTRIBUTES CTL_CODE( FILE_DEVICE_KEYBOARD,   \
                                                        IOCTL_INDEX,    \
                                                        METHOD_BUFFERED,    \
                                                        FILE_READ_DATA)

#define IOCTL_DISPATCH_KEYBOARD_IO CTL_CODE( FILE_DEVICE_KEYBOARD, \
                                    IOCTL_INDEX,    \
                                    METHOD_BUFFERED,    \
                                    FILE_ANY_ACCESS )

#define MAX_KEY_BUFFER 256

#endif
