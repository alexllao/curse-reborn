// Compila sin modificaciones la dependencia BSD; los avisos de parámetros opcionales son externos.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined(_MSC_VER)
#pragma warning(disable : 4100)
#endif
#include "../../terceros/ymfm/ymfm_opl.cpp"
