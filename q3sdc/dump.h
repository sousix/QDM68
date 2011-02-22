#ifndef DUMP_H
#define DUMP_H

#include "common.h"

typedef struct {
        entityState_t   currentState;	// zeiger auf ringbuff pso
        qboolean        currentValid;	// true if cg.frame holds this entity
        int             previousEvent;
} centity_t;

static char clientNames[MAX_CLIENTS][36];
static centity_t	centities[MAX_GENTITIES];

static void UpdateClientInfo( int clientNum, const char *info );

static void CG_CheckEvents( centity_t *cent );

char * GameStateParsed( void );

static void UpdateConfigString( int index, char *string );

void NewFrameParsed( void );


#endif // DUMP_H
