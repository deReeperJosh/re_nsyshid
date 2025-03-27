#include <coreinit/debug.h>
#include <wums.h>

WUMS_MODULE_EXPORT_NAME("homebrew_examplemodule");
WUMS_MODULE_AUTHOR("Maschell");
WUMS_MODULE_VERSION("0.1");
WUMS_MODULE_LICENSE("GPL");
WUMS_MODULE_DESCRIPTION("Just an example module");

/**
 * If this modules depends on another module (e.g. the FunctionPatcherModule) you can add a dependency to that module.
 * This will enforce that the other module has been loaded and initialized before this module is initialized.
 * If the other module is not loaded, this module also fails to load.
 * Usage: WUMS_DEPENDS_ON(export_name)
 * Example: WUMS_DEPENDS_ON(homebrew_functionpatcher)
 */

WUMS_INITIALIZE(/*wums_app_init_args_t*/ args) {
    /* Called once when the module has been loaded */

    // Information about the module can be get via the (optional) argument
    module_information_t *module_information = args.module_information;

    if (module_information == nullptr) {
        OSFatal("Failed to get module_information pointer.");
    }
    // Make sure the module is using a compatible version with the loader
    if (module_information->version != MODULE_INFORMATION_VERSION) {
        OSFatal("The module information struct version does not match.");
    }
}

WUMS_APPLICATION_STARTS() {
    /* Called whenever a new application has been started */
}

WUMS_APPLICATION_REQUESTS_EXIT() {
    /* Called whenever a application wants to exit */
}

WUMS_APPLICATION_ENDS() {
    /* Called whenever a application actually ends */
}

WUMS_RELOCATIONS_DONE() {
    /* Called whenever the relocations have been updated, but before WUMS_APPLICATION_STARTS() */
}