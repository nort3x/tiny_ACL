//
// Created by human on 2/16/22.
//

#ifndef SECURITY_CONSTANTS_H
#define SECURITY_CONSTANTS_H

char* ERROR_REASON = "NOT SET";
#define return_false_with(arg)  {ERROR_REASON = arg; return false;}

// bool!
typedef enum {
    false,
    true
} bool;

#define UNKNOWN_INPUT "unrecognized input"
#define ILLEGAL_COUNT "expected more input"

typedef struct {
   char* username;
   char* password;
} User;

typedef struct {
    char* type;
    char* domain;
} DomainTypeTuple;

#endif //SECURITY_CONSTANTS_H