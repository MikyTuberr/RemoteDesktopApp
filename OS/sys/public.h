#ifndef _PUBLIC_H
#define _PUBLIC_H

#define IOCTL_INDEX_GET_ATTRIBUTES 0x804
#define IOCTL_INDEX_DISPATCH_IO    0x800

#define IOCTL_KBFILTR_GET_KEYBOARD_ATTRIBUTES CTL_CODE( FILE_DEVICE_KEYBOARD,   \
                                                        IOCTL_INDEX_GET_ATTRIBUTES,    \
                                                        METHOD_BUFFERED,    \
                                                        FILE_READ_DATA)

#define IOCTL_DISPATCH_KEYBOARD_IO CTL_CODE( FILE_DEVICE_KEYBOARD, \
                                    IOCTL_INDEX_DISPATCH_IO,    \
                                    METHOD_BUFFERED,    \
                                    FILE_READ_DATA )

#endif
