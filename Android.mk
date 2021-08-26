LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := reversetcpd

LOCAL_SRC_FILES :=	\
	src/Daemon.cpp	\
	src/main.cpp	\
	src/ReverseTcpShell.cpp	\
	src/TcpClient.cpp

LOCAL_C_INCLUDES :=	\
	$(LOCAL_PATH)/external	\
	$(LOCAL_PATH)/include 

LOCAL_CFLAGS :=	\
	-Wall	\
	-std=gnu++14	\
	-DHAVE_PTHREADS	\
	-DASIO_NO_TYPEID	\
	-DASIO_STANDALONE

LOCAL_CPP_FEATURES += exceptions
LOCAL_CPPFLAGS += -fexceptions

include $(BUILD_EXECUTABLE)
