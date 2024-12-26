#ifndef DEBUGPRINT_H_
#define DEBUGPRINT_H_

//#define DEBUG  //set for debug output

#ifdef  DEBUG
  #define DEBUG_ERROR true
  #define DEBUG_ERROR_SERIAL if(DEBUG_ERROR)Serial

  #define DEBUG_WARNING true
  #define DEBUG_WARNING_SERIAL if(DEBUG_WARNING)Serial

  #define DEBUG_INFORMATION true
  #define DEBUG_INFORMATION_SERIAL if(DEBUG_INFORMATION)Serial

  #define DSERIALBEGIN(...)   Serial.begin(__VA_ARGS__)
  #define DPRINTLN(...)       Serial.println(__VA_ARGS__)
  #define DPRINT(...)         Serial.print(__VA_ARGS__)
  #define DPRINTF(...)        Serial.print(F(__VA_ARGS__))
  #define DPRINTLNF(...)      Serial.println(F(__VA_ARGS__)) //printing text using the F macro
  #define DELAY(...)          delay(__VA_ARGS__)
  #define PINMODE(...)        pinMode(__VA_ARGS__)
  #define DEBUG_PRINT(...)    Serial.print(F(#__VA_ARGS__" = ")); Serial.print(__VA_ARGS__); Serial.print(F(" ")) 
  #define DEBUG_PRINTLN(...)  DEBUG_PRINT(__VA_ARGS__); Serial.println()
  #define DEBUG_PRINTF(...)   Serial.printf(__VA_ARGS__)
#else
  #define DSERIALBEGIN(...)
  #define DPRINTLN(...)
  #define DPRINT(...)
  #define DPRINTF(...)      
  #define DPRINTLNF(...)    
  #define DELAY(...)        
  #define PINMODE(...)      
  #define DEBUG_PRINT(...)    
  #define DEBUG_PRINTLN(...)
  #define DEBUG_PRINTF(...)
#endif

#endif  //DEBUGPRINT_H_