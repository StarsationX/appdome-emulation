.class public Lruntime/ADConfig/ADConfig;
.super Ljava/lang/Object;


# direct methods
.method public constructor <init>()V
    .locals 0

    invoke-direct {p0}, Ljava/lang/Object;-><init>()V

    return-void
.end method

.method public static getBool(Ljava/lang/String;Z)Z
    .locals 0

    invoke-static {p0, p1}, Lruntime/loading/NativeBridge;->getBoolFromConfig(Ljava/lang/String;Z)Z

    move-result p0

    return p0
.end method

.method public static getFileFromName(Ljava/lang/String;[B)[B
    .locals 1

    const/4 v0, 0x0

    invoke-static {p0, v0}, Lruntime/ADConfig/ADConfig;->getString(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;

    move-result-object p0

    if-nez p0, :cond_0

    return-object p1

    :cond_0
    invoke-static {p0}, Lruntime/loading/NativeBridge;->getFileFromConfigByUuid(Ljava/lang/String;)[B

    move-result-object p0

    if-nez p0, :cond_1

    return-object p1

    :cond_1
    return-object p0
.end method

.method public static getInt(Ljava/lang/String;I)I
    .locals 0

    invoke-static {p0, p1}, Lruntime/loading/NativeBridge;->getIntFromConfig(Ljava/lang/String;I)I

    move-result p0

    return p0
.end method

.method public static getString(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
    .locals 0

    invoke-static {p0, p1}, Lruntime/loading/NativeBridge;->getStringFromConfig(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;

    move-result-object p0

    return-object p0
.end method

.method public static getStringArray(Ljava/lang/String;)[Ljava/lang/String;
    .locals 0

    invoke-static {p0}, Lruntime/loading/NativeBridge;->getStringArrayFromConfig(Ljava/lang/String;)[Ljava/lang/String;

    move-result-object p0

    return-object p0
.end method
