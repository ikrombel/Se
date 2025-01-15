//#include <SDL2/SDL.h>

//struct SDL_RWops;

inline static const char* APK = "/apk/";
// Macro for checking if a given pathname is inside APK's assets directory
#  define SE_IS_ASSET(p) p.starts_with(APK)
// Macro for truncating the APK prefix string from the asset pathname and at the same time patching the directory name components (see custom_rules.xml)
#  ifdef ASSET_DIR_INDICATOR
#    define SE_ASSET(p) p.substr(5).replaced("/", ASSET_DIR_INDICATOR "/").c_str()
#  else
#     define SE_ASSET(p) p.substr(5).c_str()
#  endif