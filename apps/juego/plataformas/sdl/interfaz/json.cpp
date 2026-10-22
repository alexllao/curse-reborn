// Mantener json11 sin modificar; Clang reciente avisa de std::move sin calificar en esta versión.
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunqualified-std-cast-call"
#endif
#include "../../../terceros/json11/json11.cpp"
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
