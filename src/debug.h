#ifndef debug_h
#define debug_h

// Debugging macros
#define DEBUG true

#if DEBUG
#define DEBUG_P(args...) Serial.print(args)
#define DEBUG_PLN(args...) Serial.println(args)
#else
#define DEBUG_P(args...)
#define DEBUG_PLN(args...)
#endif
#endif