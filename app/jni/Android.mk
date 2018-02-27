LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := minizip
LOCAL_SRC_FILES := minizip/ioapi_buf.c minizip/ioapi_mem.c minizip/ioapi.c minizip/unzip.c minizip/zip.c
LOCAL_SRC_FILES += minizip/minizip.cpp
LOCAL_LDLIBS += -lz
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog

include $(BUILD_SHARED_LIBRARY)