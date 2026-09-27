#pragma once

#include <android/log.h>

#define LOGCAT_TAG "lovyloyv"

#define log_V(fmt, ...)  __android_log_print(ANDROID_LOG_VERBOSE, LOGCAT_TAG, "[ %s ] " fmt, __FUNCTION__, ##__VA_ARGS__)
#define log_D(fmt, ...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOGCAT_TAG, "[ %s ] " fmt, __FUNCTION__, ##__VA_ARGS__))
#define log_I(fmt, ...) ((void)__android_log_print(ANDROID_LOG_INFO, LOGCAT_TAG, "[ %s ] " fmt, __FUNCTION__, ##__VA_ARGS__))
#define log_E(fmt, ...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOGCAT_TAG, "[ %s ] " fmt, __FUNCTION__, ##__VA_ARGS__))
#define log_W(fmt, ...) ((void)__android_log_print(ANDROID_LOG_WARN, LOGCAT_TAG, "[ %s ] " fmt, __FUNCTION__, ##__VA_ARGS__))

#define LOG_FUNC_CALL( ) log_D("call")