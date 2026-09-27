.class public Lruntime/Strings/StringIndexer;
.super Ljava/lang/Object;


# annotations
.annotation system Ldalvik/annotation/MemberClasses;
    value = {
        Lruntime/Strings/StringIndexer$stringIndexerHolder;
    }
.end annotation


# static fields
.field private static final TAG:Ljava/lang/String;

.field private static globalVariableConst12:I

.field private static globalVariableConst4:I

.field private static globalVariableConst7:I

.field private static globalVariableObfMap:Ljava/util/Map;
    .annotation system Ldalvik/annotation/Signature;
        value = {
            "Ljava/util/Map<",
            "Ljava/lang/Character;",
            "Ljava/lang/Integer;",
            ">;"
        }
    .end annotation
.end field

.field private static globalVariableObfMapLen:I

.field private static globalVariableObfPackageIdentifierLen:I


# instance fields
.field private globalVariableCache:Ljava/util/Map;
    .annotation system Ldalvik/annotation/Signature;
        value = {
            "Ljava/util/Map<",
            "Ljava/lang/String;",
            "Ljava/lang/String;",
            ">;"
        }
    .end annotation
.end field

.field private globalVariableStringDbsPackage:Ljava/util/Map;
    .annotation system Ldalvik/annotation/Signature;
        value = {
            "Ljava/util/Map<",
            "Ljava/lang/String;",
            "[B>;"
        }
    .end annotation
.end field


# direct methods
.method static constructor <clinit>()V
    .locals 1

    const/4 v0, 0x0

    sput-object v0, Lruntime/Strings/StringIndexer;->globalVariableObfMap:Ljava/util/Map;

    const-class v0, Lruntime/Strings/StringIndexer;

    invoke-virtual {v0}, Ljava/lang/Class;->getName()Ljava/lang/String;

    move-result-object v0

    sput-object v0, Lruntime/Strings/StringIndexer;->TAG:Ljava/lang/String;

    invoke-static {}, Lruntime/loading/Setup;->staticInit()V

    return-void
.end method

.method private constructor <init>()V
    .locals 1

    invoke-direct {p0}, Ljava/lang/Object;-><init>()V

    invoke-static {}, Lruntime/JavaMethodsHelper$MapHelper;->_newConcurrentHashMap()Ljava/util/Map;

    move-result-object v0

    iput-object v0, p0, Lruntime/Strings/StringIndexer;->globalVariableCache:Ljava/util/Map;

    invoke-static {}, Lruntime/JavaMethodsHelper$MapHelper;->_newConcurrentHashMap()Ljava/util/Map;

    move-result-object v0

    iput-object v0, p0, Lruntime/Strings/StringIndexer;->globalVariableStringDbsPackage:Ljava/util/Map;

    invoke-static {}, Lruntime/JavaMethodsHelper$MapHelper;->_newConcurrentHashMap()Ljava/util/Map;

    move-result-object v0

    sput-object v0, Lruntime/Strings/StringIndexer;->globalVariableObfMap:Ljava/util/Map;

    invoke-direct {p0}, Lruntime/Strings/StringIndexer;->initGlobalVariables()V

    return-void
.end method

.method synthetic constructor <init>(Lruntime/Strings/StringIndexer-IA;)V
    .locals 0

    invoke-direct {p0}, Lruntime/Strings/StringIndexer;-><init>()V

    return-void
.end method

.method private initGlobalVariables()V
    .locals 5

    invoke-static {}, Lruntime/loading/NativeBridge;->stringIndexerNativeInitializer()Ljava/lang/String;

    move-result-object v0

    const-string v1, ","

    invoke-static {v0, v1}, Lruntime/JavaMethodsHelper$StringsHelper;->_split(Ljava/lang/String;Ljava/lang/String;)[Ljava/lang/String;

    move-result-object v0

    const/4 v1, 0x0

    aget-object v2, v0, v1

    invoke-static {v2}, Ljava/lang/Integer;->parseInt(Ljava/lang/String;)I

    move-result v2

    sput v2, Lruntime/Strings/StringIndexer;->globalVariableConst4:I

    const/4 v2, 0x1

    aget-object v2, v0, v2

    invoke-static {v2}, Ljava/lang/Integer;->parseInt(Ljava/lang/String;)I

    move-result v2

    sput v2, Lruntime/Strings/StringIndexer;->globalVariableConst7:I

    const/4 v2, 0x2

    aget-object v2, v0, v2

    invoke-static {v2}, Ljava/lang/Integer;->parseInt(Ljava/lang/String;)I

    move-result v2

    sput v2, Lruntime/Strings/StringIndexer;->globalVariableConst12:I

    const/4 v2, 0x3

    aget-object v2, v0, v2

    invoke-static {v2}, Ljava/lang/Integer;->parseInt(Ljava/lang/String;)I

    move-result v2

    sput v2, Lruntime/Strings/StringIndexer;->globalVariableObfMapLen:I

    const/4 v2, 0x4

    aget-object v2, v0, v2

    invoke-static {v2}, Ljava/lang/Integer;->parseInt(Ljava/lang/String;)I

    move-result v2

    sput v2, Lruntime/Strings/StringIndexer;->globalVariableObfPackageIdentifierLen:I

    const/4 v2, 0x5

    aget-object v0, v0, v2

    nop

    :goto_0
    sget v2, Lruntime/Strings/StringIndexer;->globalVariableObfMapLen:I

    if-ge v1, v2, :cond_0

    sget-object v2, Lruntime/Strings/StringIndexer;->globalVariableObfMap:Ljava/util/Map;

    invoke-virtual {v0, v1}, Ljava/lang/String;->charAt(I)C

    move-result v3

    invoke-static {v3}, Ljava/lang/Character;->valueOf(C)Ljava/lang/Character;

    move-result-object v3

    invoke-static {v1}, Ljava/lang/Integer;->valueOf(I)Ljava/lang/Integer;

    move-result-object v4

    invoke-static {v2, v3, v4}, Lruntime/JavaMethodsHelper$MapHelper;->_put(Ljava/util/Map;Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;

    add-int/lit8 v1, v1, 0x1

    goto :goto_0

    :cond_0
    return-void
.end method

.method private declared-synchronized getStringDbForPackage(Ljava/lang/String;)[B
    .locals 2

    monitor-enter p0

    nop

    :try_start_0
    iget-object v0, p0, Lruntime/Strings/StringIndexer;->globalVariableStringDbsPackage:Ljava/util/Map;

    invoke-static {v0, p1}, Lruntime/JavaMethodsHelper$MapHelper;->_containsKey(Ljava/util/Map;Ljava/lang/Object;)Z

    move-result v0

    if-nez v0, :cond_0

    invoke-static {}, Lruntime/loading/NativeBridge;->getStringIndexerDB()[B

    move-result-object v0

    iget-object v1, p0, Lruntime/Strings/StringIndexer;->globalVariableStringDbsPackage:Ljava/util/Map;

    invoke-static {v1, p1, v0}, Lruntime/JavaMethodsHelper$MapHelper;->_put(Ljava/util/Map;Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;

    goto :goto_0

    :cond_0
    iget-object v0, p0, Lruntime/Strings/StringIndexer;->globalVariableStringDbsPackage:Ljava/util/Map;

    invoke-static {v0, p1}, Lruntime/JavaMethodsHelper$MapHelper;->_get(Ljava/util/Map;Ljava/lang/Object;)Ljava/lang/Object;

    move-result-object p1

    move-object v0, p1

    check-cast v0, [B
    :try_end_0
    .catchall {:try_start_0 .. :try_end_0} :catchall_0

    :goto_0
    monitor-exit p0

    return-object v0

    :catchall_0
    move-exception p1

    monitor-exit p0

    throw p1
.end method

.method private getIntFromBuffer(Ljava/lang/String;I[B)I
    .locals 1

    if-nez p3, :cond_0

    return p2

    :cond_0
    array-length p1, p3

    add-int/lit8 p1, p2, 0x3

    aget-byte p1, p3, p1

    shl-int/lit8 p1, p1, 0x18

    add-int/lit8 v0, p2, 0x2

    aget-byte v0, p3, v0

    and-int/lit16 v0, v0, 0xff

    shl-int/lit8 v0, v0, 0x10

    or-int/2addr p1, v0

    add-int/lit8 v0, p2, 0x1

    aget-byte v0, p3, v0

    and-int/lit16 v0, v0, 0xff

    shl-int/lit8 v0, v0, 0x8

    or-int/2addr p1, v0

    aget-byte p2, p3, p2

    and-int/lit16 p2, p2, 0xff

    or-int/2addr p1, p2

    return p1
.end method

.method private _internalGetString(Ljava/lang/String;)Ljava/lang/String;
    .locals 2

    iget-object v0, p0, Lruntime/Strings/StringIndexer;->globalVariableCache:Ljava/util/Map;

    invoke-static {v0, p1}, Lruntime/JavaMethodsHelper$MapHelper;->_containsKey(Ljava/util/Map;Ljava/lang/Object;)Z

    move-result v0

    if-eqz v0, :cond_0

    iget-object v0, p0, Lruntime/Strings/StringIndexer;->globalVariableCache:Ljava/util/Map;

    invoke-static {v0, p1}, Lruntime/JavaMethodsHelper$MapHelper;->_get(Ljava/util/Map;Ljava/lang/Object;)Ljava/lang/Object;

    move-result-object p1

    check-cast p1, Ljava/lang/String;

    goto :goto_0

    :cond_0
    invoke-direct {p0, p1}, Lruntime/Strings/StringIndexer;->_getStringFromDB(Ljava/lang/String;)Ljava/lang/String;

    move-result-object v0

    invoke-static {v0}, Ldd1UH0/ezLQq4/SkcL98;->R9LtJ9(Ljava/lang/String;)Ljava/lang/String;

    move-result-object v0

    invoke-virtual {v0}, Ljava/lang/String;->intern()Ljava/lang/String;

    move-result-object v0

    iget-object v1, p0, Lruntime/Strings/StringIndexer;->globalVariableCache:Ljava/util/Map;

    invoke-static {v1, p1, v0}, Lruntime/JavaMethodsHelper$MapHelper;->_put(Ljava/util/Map;Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;

    move-object p1, v0

    :goto_0
    return-object p1
.end method

.method private _getStringFromDB(Ljava/lang/String;)Ljava/lang/String;
    .locals 4

    sget v0, Lruntime/Strings/StringIndexer;->globalVariableConst4:I

    div-int/lit8 v0, v0, 0x2

    add-int/lit8 v0, v0, -0x2

    sget v1, Lruntime/Strings/StringIndexer;->globalVariableObfPackageIdentifierLen:I

    invoke-static {p1, v1}, Lruntime/JavaMethodsHelper$StringsHelper;->_substring(Ljava/lang/String;I)Ljava/lang/String;

    move-result-object v1

    invoke-direct {p0, v1}, Lruntime/Strings/StringIndexer;->base62ToInt(Ljava/lang/String;)J

    move-result-wide v1

    long-to-int v1, v1

    sget v2, Lruntime/Strings/StringIndexer;->globalVariableObfPackageIdentifierLen:I

    add-int/2addr v2, v0

    invoke-static {p1, v0, v2}, Lruntime/JavaMethodsHelper$StringsHelper;->_substring(Ljava/lang/String;II)Ljava/lang/String;

    move-result-object v0

    invoke-direct {p0, v0}, Lruntime/Strings/StringIndexer;->getStringDbForPackage(Ljava/lang/String;)[B

    move-result-object v0

    const/4 v2, 0x0

    invoke-direct {p0, p1, v2, v0}, Lruntime/Strings/StringIndexer;->getIntFromBuffer(Ljava/lang/String;I[B)I

    move-result v2

    add-int/lit8 v1, v1, -0x1

    sget v3, Lruntime/Strings/StringIndexer;->globalVariableConst4:I

    mul-int/2addr v1, v3

    sget v3, Lruntime/Strings/StringIndexer;->globalVariableConst4:I

    add-int/2addr v1, v3

    invoke-direct {p0, p1, v1, v0}, Lruntime/Strings/StringIndexer;->getIntFromBuffer(Ljava/lang/String;I[B)I

    move-result v1

    sget v3, Lruntime/Strings/StringIndexer;->globalVariableConst7:I

    add-int/lit8 v3, v3, -0x3

    add-int/2addr v2, v3

    add-int/2addr v2, v1

    invoke-direct {p0, p1, v2, v0}, Lruntime/Strings/StringIndexer;->getIntFromBuffer(Ljava/lang/String;I[B)I

    move-result p1

    add-int/lit8 v2, v2, 0x4

    new-instance v1, Ljava/lang/String;

    add-int/lit8 p1, p1, -0x1

    sget-object v3, Ljava/nio/charset/StandardCharsets;->UTF_8:Ljava/nio/charset/Charset;

    invoke-direct {v1, v0, v2, p1, v3}, Ljava/lang/String;-><init>([BIILjava/nio/charset/Charset;)V

    return-object v1
.end method

.method public static _getString(Ljava/lang/String;)Ljava/lang/String;
    .locals 1

    invoke-static {}, Lruntime/Strings/StringIndexer$stringIndexerHolder;->getInstance()Lruntime/Strings/StringIndexer;

    move-result-object v0

    invoke-direct {v0, p0}, Lruntime/Strings/StringIndexer;->_internalGetString(Ljava/lang/String;)Ljava/lang/String;

    move-result-object p0

    return-object p0
.end method

.method private base62ToInt(Ljava/lang/String;)J
    .locals 8

    nop

    sget v0, Lruntime/Strings/StringIndexer;->globalVariableObfMapLen:I

    invoke-static {p1}, Lruntime/JavaMethodsHelper$StringsHelper;->_toCharArray(Ljava/lang/String;)[C

    move-result-object p1

    array-length v1, p1

    const-wide/16 v2, 0x0

    const/4 v4, 0x0

    :goto_0
    if-ge v4, v1, :cond_0

    aget-char v5, p1, v4

    int-to-long v6, v0

    mul-long/2addr v2, v6

    sget-object v6, Lruntime/Strings/StringIndexer;->globalVariableObfMap:Ljava/util/Map;

    invoke-static {v5}, Ljava/lang/Character;->valueOf(C)Ljava/lang/Character;

    move-result-object v5

    invoke-static {v6, v5}, Lruntime/JavaMethodsHelper$MapHelper;->_get(Ljava/util/Map;Ljava/lang/Object;)Ljava/lang/Object;

    move-result-object v5

    check-cast v5, Ljava/lang/Integer;

    invoke-virtual {v5}, Ljava/lang/Integer;->intValue()I

    move-result v5

    int-to-long v5, v5

    add-long/2addr v2, v5

    add-int/lit8 v4, v4, 0x1

    goto :goto_0

    :cond_0
    return-wide v2
.end method
