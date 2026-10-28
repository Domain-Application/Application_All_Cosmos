// AllcosmosTicketMapping.h : which AllcosmosSystem call a ticket needs, and its JSON
//Notes: docs/CPP_PROGRAMMING_NOTES.md.

#if !defined(ALLCOSMOSTICKETMAPPING_H_INCLUDED)
#define ALLCOSMOSTICKETMAPPING_H_INCLUDED

#include "AllcosmosWbApi.h"

// Lorry in: Raw Material Ticket In (type 1, receiving) or DO Time In (type 2, despatch).
SAllcosmosApiResult AllcosmosWeighIn(DB *df, int type);

// Lorry out (df = WB or DESPATCH): sends the "In" first if it never succeeded, then the "Out".
SAllcosmosApiResult AllcosmosWeighOut(DB *df);

#endif
