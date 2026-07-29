typedef unsigned char uint8_t;
typedef int int32_t;
typedef unsigned long long uint64_t;
typedef unsigned long size_t;
typedef unsigned long uintptr_t;

#define NULL ((void *)0)
#define RTLD_DEFAULT ((void *)0)
#define RTLD_LAZY 0x00001
#define RTLD_NOLOAD 0x00004

typedef unsigned long pthread_t;
typedef struct {
    unsigned long opaque[8];
} pthread_attr_t;

extern void *dlopen(const char *, int);
extern void *dlsym(void *, const char *);
extern int pthread_create(
    pthread_t *, const pthread_attr_t *, void *(*)(void *), void *);
extern int pthread_detach(pthread_t);
extern int usleep(unsigned int);
extern void *memcpy(void *, const void *, size_t);
extern void *memset(void *, int, size_t);
extern int strcmp(const char *, const char *);

typedef void Il2CppDomain;
typedef void Il2CppThread;
typedef void Il2CppAssembly;
typedef void Il2CppImage;
typedef void Il2CppClass;
typedef void MethodInfo;
typedef void Il2CppObject;
typedef void Il2CppString;

typedef Il2CppDomain *(*domain_get_fn)(void);
typedef Il2CppThread *(*thread_attach_fn)(Il2CppDomain *);
typedef const Il2CppAssembly **(*domain_get_assemblies_fn)(const Il2CppDomain *, size_t *);
typedef const Il2CppImage *(*assembly_get_image_fn)(const Il2CppAssembly *);
typedef const char *(*image_get_name_fn)(const Il2CppImage *);
typedef Il2CppClass *(*class_from_name_fn)(const Il2CppImage *, const char *, const char *);
typedef const MethodInfo *(*class_get_method_from_name_fn)(Il2CppClass *, const char *, int);
typedef Il2CppString *(*string_new_fn)(const char *);
typedef Il2CppObject *(*runtime_invoke_fn)(const MethodInfo *, void *, void **, Il2CppObject **);

enum {
    THHEAL_STARTING = 2,
    THHEAL_IDLE = 3,
    THHEAL_PROCESSING = 4,
    THHEAL_DONE = 5,
    THHEAL_ERROR_API = 6,
    THHEAL_ERROR_METHOD = 7,
    THHEAL_ERROR_INVOKE = 8
};

struct heal_command {
    char equipment_id[37];
    uint8_t action;
    uint8_t string_padding[2];
    int32_t api_token;
    uint8_t pointer_padding[4];
    uint64_t target;
};

__attribute__((visibility("default"))) volatile uint64_t thheal_status;
__attribute__((visibility("default"))) volatile struct heal_command thheal_control;

static void *resolve(const char *name)
{
    /*
     * libil2cpp's symbols are not in loader global scope on this build.
     * RTLD_NOLOAD only obtains its existing handle; no duplicate load.
     */
    void *il2cpp = dlopen("libil2cpp.so", RTLD_LAZY | RTLD_NOLOAD);
    void *symbol = il2cpp ? dlsym(il2cpp, name) : NULL;
    return symbol ? symbol : dlsym(RTLD_DEFAULT, name);
}

static const Il2CppImage *find_game_image(
    Il2CppDomain *domain,
    domain_get_assemblies_fn get_assemblies,
    assembly_get_image_fn get_image,
    image_get_name_fn get_image_name)
{
    size_t count = 0;
    const Il2CppAssembly **assemblies = get_assemblies(domain, &count);
    for (size_t i = 0; i < count; ++i) {
        const Il2CppImage *image = get_image(assemblies[i]);
        const char *name = image ? get_image_name(image) : NULL;
        if (name && strcmp(name, "Assembly-CSharp.dll") == 0)
            return image;
    }
    return NULL;
}

static const MethodInfo *find_method(
    const Il2CppImage *image,
    class_from_name_fn class_from_name,
    class_get_method_from_name_fn get_method,
    const char *class_name,
    const char *method_name,
    int parameter_count)
{
    Il2CppClass *klass = class_from_name(image, "", class_name);
    return klass ? get_method(klass, method_name, parameter_count) : NULL;
}

static void *start_heal_service(void *unused)
{
    (void)unused;
    thheal_status = THHEAL_STARTING;

    domain_get_fn domain_get = NULL;
    thread_attach_fn thread_attach = NULL;
    domain_get_assemblies_fn get_assemblies = NULL;
    assembly_get_image_fn get_image = NULL;
    image_get_name_fn get_image_name = NULL;
    class_from_name_fn class_from_name = NULL;
    class_get_method_from_name_fn get_method = NULL;
    string_new_fn string_new = NULL;
    runtime_invoke_fn runtime_invoke = NULL;

    /* libthheal may initialize before libil2cpp enters the loader namespace. */
    for (int attempt = 0; attempt < 300; ++attempt) {
        domain_get = (domain_get_fn)resolve("il2cpp_domain_get");
        thread_attach = (thread_attach_fn)resolve("il2cpp_thread_attach");
        get_assemblies =
            (domain_get_assemblies_fn)resolve("il2cpp_domain_get_assemblies");
        get_image =
            (assembly_get_image_fn)resolve("il2cpp_assembly_get_image");
        get_image_name =
            (image_get_name_fn)resolve("il2cpp_image_get_name");
        class_from_name =
            (class_from_name_fn)resolve("il2cpp_class_from_name");
        get_method = (class_get_method_from_name_fn)resolve(
            "il2cpp_class_get_method_from_name");
        string_new = (string_new_fn)resolve("il2cpp_string_new");
        runtime_invoke =
            (runtime_invoke_fn)resolve("il2cpp_runtime_invoke");
        if (domain_get && thread_attach && get_assemblies && get_image &&
            get_image_name && class_from_name && get_method && string_new &&
            runtime_invoke)
            break;
        usleep(100000);
    }

    if (!domain_get || !thread_attach || !get_assemblies || !get_image ||
        !get_image_name || !class_from_name || !get_method || !string_new ||
        !runtime_invoke) {
        thheal_status = THHEAL_ERROR_API;
        return NULL;
    }

    /*
     * Houdini/Unity exposes IL2CPP exports before runtime initialization.
     * Waiting prevents domain_get from touching partially initialized state.
     */
    usleep(10000000);
    Il2CppDomain *domain = domain_get();
    if (!domain || !thread_attach(domain)) {
        thheal_status = THHEAL_ERROR_API;
        return NULL;
    }

    const Il2CppImage *game_image = find_game_image(
        domain, get_assemblies, get_image, get_image_name);
    if (!game_image) {
        thheal_status = THHEAL_ERROR_METHOD;
        return NULL;
    }

    const MethodInfo *get_api_token =
        find_method(game_image, class_from_name, get_method,
                    "PlayerBehavior", "GetApiToken", 0);
    const MethodInfo *methods[] = {
        NULL,
        find_method(game_image, class_from_name, get_method,
                    "PlayerBehavior", "RequestUseItem", 3),
        find_method(game_image, class_from_name, get_method,
                    "PlayerBehavior", "CmdUseItem", 3),
        find_method(game_image, class_from_name, get_method,
                    "PlayerBehavior", "LogicUseItem", 3),
        find_method(game_image, class_from_name, get_method,
                    "PlayerAuto", "ProcessAutoUsePotion", 0),
        find_method(game_image, class_from_name, get_method,
                    "PlayerBehavior",
                    "UserCode_CmdUseItem__String__Int32__String", 3),
        find_method(game_image, class_from_name, get_method,
                    "PlayerBehavior", "RequestRevive", 2),
    };

    thheal_status = THHEAL_IDLE;
    for (;;) {
        uint64_t target = thheal_control.target;
        if (!target) {
            usleep(10000);
            continue;
        }

        struct heal_command command;
        memcpy(&command, (const void *)&thheal_control, sizeof(command));
        memset((void *)&thheal_control, 0, sizeof(thheal_control));
        command.equipment_id[36] = '\0';
        thheal_status = THHEAL_PROCESSING;

        if (command.action < 1 || command.action > 6 ||
            !methods[command.action] ||
            (command.action != 3 && command.action != 4 && !get_api_token)) {
            thheal_status = THHEAL_ERROR_METHOD;
            continue;
        }

        Il2CppObject *exception = NULL;
        if (command.action != 3 && command.action != 4) {
            Il2CppObject *boxed_token = runtime_invoke(
                get_api_token, (void *)(uintptr_t)command.target,
                NULL, &exception);
            if (exception || !boxed_token) {
                thheal_status = THHEAL_ERROR_INVOKE;
                continue;
            }
            memcpy(&command.api_token, (uint8_t *)boxed_token + 16,
                   sizeof(command.api_token));
        }

        Il2CppString *equipment_id = string_new(command.equipment_id);
        Il2CppString *pass_code = string_new("");
        uint8_t use_swap_rate_limited = 0;
        /* action 6 = RequestRevive(Boolean isNear, Int32 apiToken) */
        uint8_t revive_is_near = command.equipment_id[0] == '1';
        void *revive_args[] = {&revive_is_near, &command.api_token};
        void *standard_args[] = {
            equipment_id, &command.api_token, pass_code};
        void *logic_args[] = {
            equipment_id, pass_code, &use_swap_rate_limited};
        void **args = command.action == 6 ? revive_args
                    : command.action == 3 ? logic_args : standard_args;
        if (command.action == 4)
            args = NULL;

        exception = NULL;
        runtime_invoke(
            methods[command.action], (void *)(uintptr_t)command.target,
            args, &exception);
        thheal_status = exception ? THHEAL_ERROR_INVOKE : THHEAL_DONE;
    }
}

__attribute__((constructor))
static void init_heal_service(void)
{
    pthread_t thread;
    thheal_status = 1;
    if (pthread_create(&thread, NULL, start_heal_service, NULL) == 0)
        pthread_detach(thread);
    else
        thheal_status = THHEAL_ERROR_API;
}