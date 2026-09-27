#include <appdome.hpp>
#include <logcat.hpp>
#include <jni.h>

// setApplication
/* extern "C" JNIEXPORT  */ void Java_INIT_CB(JNIEnv *env, jclass clazz, jobject context)
{
    LOG_FUNC_CALL();

    // it is possible to get the application context from here.

    exports::setup_handlers();
}

// onResumedCalled
/* extern "C" JNIEXPORT  */ void Java_ACTIVITY_CB2(JNIEnv *env, jclass clazz)
{
    (void)env;
    (void)clazz;
}

// runFirstActivityActions
/* extern "C" JNIEXPORT  */ void Java_ACTIVITY_CB(JNIEnv *env, jclass clazz)
{
    LOG_FUNC_CALL();
}

// getExternalEventID
/* extern "C" JNIEXPORT  */ jstring Java_STRING_CB(JNIEnv *env, jclass clazz, jstring str)
{
    LOG_FUNC_CALL();
    return env->NewStringUTF("");
}

// onPausedCalled
/* extern "C" JNIEXPORT  */ void Java_ON_PAUSE_CB(JNIEnv *env, jclass clazz)
{
    LOG_FUNC_CALL();
}

// markCrash
/* extern "C" JNIEXPORT  */ void Java_ERROR_CB(JNIEnv *env, jclass clazz)
{
    LOG_FUNC_CALL();
}

/* extern "C" JNIEXPORT  */ jstring Java_getApplicationIDAppdome(JNIEnv *env, jclass clazz)
{
    LOG_FUNC_CALL();
    return env->NewStringUTF("");
}

/* extern "C" JNIEXPORT  */ jstring Java_getStringFromConfig(JNIEnv *env, jclass clazz, jstring key, jstring on_null)
{
    LOG_FUNC_CALL();

    if (key)
    {
        const char *key_cstr = env->GetStringUTFChars(key, nullptr);
        std::string key_str(key_cstr);

        env->ReleaseStringUTFChars(key, key_cstr);

        std::string value = assets::get_as_key(key_str);
        log_D("Config value for %s : %s", key_str.c_str(), value.c_str());

        return env->NewStringUTF(value.c_str());
    }
    return on_null;
}

//(Ljava/lang/String;)V
/* extern "C" JNIEXPORT  */ void Java_appRegisteredToEvent(JNIEnv *env, jclass clazz, jstring event)
{
    LOG_FUNC_CALL();

    const char *event_cstr = env->GetStringUTFChars(event, nullptr);
    std::string event_str(event_cstr);

    env->ReleaseStringUTFChars(event, event_cstr);

    log_D("Registered to event: %s", event_str.c_str());
}

/* extern "C" JNIEXPORT  */ void Java_appUnRegisteredFromEvent(JNIEnv *env, jclass clazz, jstring event)
{
    LOG_FUNC_CALL();

    const char *event_cstr = env->GetStringUTFChars(event, nullptr);
    std::string event_str(event_cstr);

    env->ReleaseStringUTFChars(event, event_cstr);

    log_D("Unregistered from event: %s", event_str.c_str());
}

inline std::string get_caller_class(JNIEnv *env)
{
    // Thread.currentThread()
    jclass thread_class = env->FindClass(("java/lang/Thread"));
    jmethodID currentThread_mid = env->GetStaticMethodID(thread_class, ("currentThread"), ("()Ljava/lang/Thread;"));
    jobject thread = env->CallStaticObjectMethod(thread_class, currentThread_mid);

    // getStackTrace()
    jmethodID getStack = env->GetMethodID(thread_class, ("getStackTrace"), ("()[Ljava/lang/StackTraceElement;"));
    jobjectArray stack = (jobjectArray)env->CallObjectMethod(thread, getStack);

    if (stack == nullptr)
    {
        env->DeleteLocalRef(thread);
        env->DeleteLocalRef(thread_class);
        log_E("getStackTrace returned null");
        return "";
    }

    jsize stacklen = env->GetArrayLength(stack);
    log_D("stack trace length: %d", stacklen);

    if (stacklen <= 7)
    {
        env->DeleteLocalRef(stack);
        env->DeleteLocalRef(thread);
        env->DeleteLocalRef(thread_class);
        log_E("stack too short to resolve caller class: %d", stacklen);
        return "";
    }

    // index 0 = getStackTrace
    // index 1 = current method (Native)
    // index 2 = the caller of Native
    // index 3 = its caller
    jobject element = env->GetObjectArrayElement(stack, 7);

    jclass ste_class = env->FindClass(("java/lang/StackTraceElement"));
    jmethodID getClassName = env->GetMethodID(ste_class, ("getClassName"), ("()Ljava/lang/String;"));
    jstring class_name = (jstring)env->CallObjectMethod(element, getClassName);

    const char *caller = env->GetStringUTFChars(class_name, 0);
    std::string caller_str(caller);
    env->ReleaseStringUTFChars(class_name, caller);

    // get only package, no class
    size_t last_dot = caller_str.rfind(("."));
    if (last_dot != std::string::npos)
    {
        caller_str = caller_str.substr(0, last_dot);
    }

    env->DeleteLocalRef(class_name);
    env->DeleteLocalRef(ste_class);
    env->DeleteLocalRef(element);
    env->DeleteLocalRef(stack);
    env->DeleteLocalRef(thread);
    env->DeleteLocalRef(thread_class);

    return caller_str;
}

inline void dump_stacktrace(JNIEnv *env)
{
    // Thread.currentThread()
    jclass thread_class = env->FindClass(("java/lang/Thread"));
    jmethodID currentThread_mid = env->GetStaticMethodID(thread_class, ("currentThread"), ("()Ljava/lang/Thread;"));
    jobject thread = env->CallStaticObjectMethod(thread_class, currentThread_mid);

    // getStackTrace()
    jmethodID getStack = env->GetMethodID(thread_class, ("getStackTrace"), ("()[Ljava/lang/StackTraceElement;"));
    jobjectArray stack = (jobjectArray)env->CallObjectMethod(thread, getStack);

    if (stack == nullptr)
    {
        env->DeleteLocalRef(thread);
        env->DeleteLocalRef(thread_class);
        log_E("getStackTrace returned null");
        return;
    }

    // dump stack for debugging
    jsize stacklen = env->GetArrayLength(stack);
    log_D("stack trace length: %d", stacklen);

    const jsize frames_to_dump = stacklen > 16 ? 16 : stacklen;
    for (jsize i = 0; i < frames_to_dump; ++i)
    {
        jobject element = env->GetObjectArrayElement(stack, i);

        jclass ste_class = env->FindClass(("java/lang/StackTraceElement"));
        jmethodID getClassName = env->GetMethodID(ste_class,
                                                  ("getClassName"),
                                                  ("()Ljava/lang/String;"));
        jstring class_name = (jstring)env->CallObjectMethod(element, getClassName);

        jmethodID getMethodName = env->GetMethodID(ste_class,
                                                   ("getMethodName"),
                                                   ("()Ljava/lang/String;"));

        jstring method_name = (jstring)env->CallObjectMethod(element, getMethodName);

        const char *class_name_cstr = env->GetStringUTFChars(class_name, 0);
        const char *method_name_cstr = env->GetStringUTFChars(method_name, 0);
        log_D("Stack[%d]: %s->%s", i, class_name_cstr, method_name_cstr);
        env->ReleaseStringUTFChars(class_name, class_name_cstr);
        env->ReleaseStringUTFChars(method_name, method_name_cstr);
        env->DeleteLocalRef(method_name);
        env->DeleteLocalRef(class_name);
        env->DeleteLocalRef(ste_class);
        env->DeleteLocalRef(element);
    }

    env->DeleteLocalRef(stack);
    env->DeleteLocalRef(thread);
    env->DeleteLocalRef(thread_class);
}

/* extern "C" JNIEXPORT  */ jbyteArray Java_getStringIndexerDB(JNIEnv *env, jclass clazz)
{
    LOG_FUNC_CALL();

    // first we need the class that called this native
    std::string caller_class_name = get_caller_class(env);
    log_D("called by class: %s", caller_class_name.data());

    const std::string package_name = assets::get_package_name();
    log_D("app package name: %s", package_name.data());

    // "classname_JavaStringIndexerStringDb_com.package.name";
    std::string target = caller_class_name + STRING_INDEXER_DB_SUFFIX + package_name;
    log_D("target for db: %s", target.data());

    std::string dbs = assets::get_from_name(target);
    if (dbs.empty())
    {
        log_E("Failed to get dbs from blobs with key: %s", target.data());
        return nullptr;
    }

    log_D("got dbs of size: %zu", dbs.size());
    jsize dbs_size = static_cast<jsize>(dbs.size());

    jbyteArray byteArray = env->NewByteArray(dbs_size);
    env->SetByteArrayRegion(byteArray, 0, dbs_size, reinterpret_cast<const jbyte *>(dbs.data()));
    return byteArray;
}

/* extern "C" JNIEXPORT  */ jstring Java_stringIndexerNativeInitializer(JNIEnv *env, jclass clazz)
{
    LOG_FUNC_CALL();

    std::string StringIndexerInitData = assets::get_as_key(("StringIndexerInitData"));
    log_D("StringIndexerInitData: %s", StringIndexerInitData.data());
    return env->NewStringUTF(StringIndexerInitData.data());
}

/* extern "C" JNIEXPORT */ void dynamicInitDone(JNIEnv *env, jclass clazz)
{
    LOG_FUNC_CALL();
}
