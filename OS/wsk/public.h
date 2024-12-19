#ifndef _PUBLIC2_H
#define _PUBLIC2_H

#define IOCTL_INDEX_SEND_DATA    0x800

#define IOCTL_SEND_DATA CTL_CODE(   FILE_DEVICE_NETWORK,   \
                                    IOCTL_INDEX_SEND_DATA,    \
                                    METHOD_BUFFERED,    \
                                    FILE_WRITE_ACCESS)

#endif
