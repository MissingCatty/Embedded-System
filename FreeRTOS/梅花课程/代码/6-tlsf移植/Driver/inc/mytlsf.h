#ifndef MYMEM_H
#define MyMEM_H

#include "tlsf.h"

extern tlsf_t tlsf;

void mytlsf_init(void);

void *mytlsf_malloc(size_t size);

void mytlsf_free(tlsf_t ptr);

#endif
