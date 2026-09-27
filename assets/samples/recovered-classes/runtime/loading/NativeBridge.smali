.class public Lruntime/loading/NativeBridge;
.super Ljava/lang/Object;


# static fields
.field private static isRegisterNativesDone:Z


# direct methods
.method static constructor <clinit>()V
    .locals 1

    const/4 v0, 0x0

    sput-boolean v0, Lruntime/loading/NativeBridge;->isRegisterNativesDone:Z

    invoke-static {}, Lruntime/loading/NativeBridge;->registerNatives()V

    return-void
.end method

.method public constructor <init>()V
    .locals 0

    invoke-direct {p0}, Ljava/lang/Object;-><init>()V

    return-void
.end method

.method public static native getRootCAsPEM()Ljava/lang/String;
.end method

.method public static native nfLog(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;ZLjava/lang/String;)V
.end method

.method public static native savePersistentDictionary(Ljava/lang/String;)V
.end method

.method public static native getObfAppdomeMethod(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
.end method

.method public static native stringIndexerNativeInitializer()Ljava/lang/String;
.end method

.method public static native getIntFromConfig(Ljava/lang/String;I)I
.end method

.method public static native getBoolFromConfig(Ljava/lang/String;Z)Z
.end method

.method public static native getApplicationIDAppdome()Ljava/lang/String;
.end method

.method public static varargs native sendDevEvent(Ljava/lang/String;[Ljava/lang/String;)V
.end method

.method public static native onResumedCalled()V
.end method

.method public static native getExternalEventID(Ljava/lang/String;)Ljava/lang/String;
.end method

.method public static native getFileFromConfigByUuid(Ljava/lang/String;)[B
.end method

.method public static native getStringArrayFromConfig(Ljava/lang/String;)[Ljava/lang/String;
.end method

.method public static native handleEnforcement(Ljava/lang/String;)V
.end method

.method public static declared-synchronized registerNatives()V
    .locals 2

    const-class v0, Lruntime/loading/NativeBridge;

    monitor-enter v0

    :try_start_0
    sget-boolean v1, Lruntime/loading/NativeBridge;->isRegisterNativesDone:Z
    :try_end_0
    .catchall {:try_start_0 .. :try_end_0} :catchall_0

    if-eqz v1, :cond_0

    monitor-exit v0

    return-void

    :cond_0
    :try_start_1
    invoke-static {}, Lqwerty/asdfgh/zxcvbn;->lkjhgf()V

    const/4 v1, 0x1

    sput-boolean v1, Lruntime/loading/NativeBridge;->isRegisterNativesDone:Z
    :try_end_1
    .catchall {:try_start_1 .. :try_end_1} :catchall_0

    monitor-exit v0

    return-void

    :catchall_0
    move-exception v1

    monitor-exit v0

    throw v1
.end method

.method public static native setApplication(Ljava/lang/Object;)V
.end method

.method public static native markCrash()V
.end method

.method public static native getStringIndexerDB()[B
.end method

.method public static native killForkUtilChildProcesses()V
.end method

.method public static native appRegisteredToEvent(Ljava/lang/String;)V
.end method

.method public static native isLowLevelLogDisabled()Z
.end method

.method public static native appUnRegisteredFromEvent(Ljava/lang/String;)V
.end method

.method public static native supportsDevEvent(Ljava/lang/String;)Z
.end method

.method public static native onPausedCalled()V
.end method

.method public static native runFirstActivityActions()V
.end method

.method public static native getStringFromConfig(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
.end method

.method public static native putStringToPersistentDictionary(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
.end method
