#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nanorq.h"

#include "kvec.h"

struct sym
{
  uint32_t tag;
  uint8_t *data;
};

struct OTI_common
{
  size_t F;  /* input size in bytes */
  size_t T;  /* the symbol size in octets, which MUST be a multiple of Al */
  size_t Al; /* byte alignment, 0 < Al <= 8, 4 is recommended */
};

struct OTI_scheme
{
  size_t Z;  /* number of source blocks */
  size_t N;  /* number of sub-blocks in each source block */
  size_t Kt; /* the total number of symbols required to represent input */
};

struct OTI
{
  struct OTI_common common;
  struct OTI_scheme scheme;
  uint32_t Curesi;
  bool Endflag;
};



typedef kvec_t(struct sym) symvec;

void random_bytes(uint8_t *buf, uint64_t len);
void RQ_encode_init(nanorq **rq, struct ioctx **myio_in, size_t num_packets, size_t packet_size, bool precalc);
void RQ_generate_symbols(nanorq *rq, struct ioctx *myio, int sbn, symvec *packets, uint32_t esi);
uint32_t RQ_receive(nanorq *rq, struct ioctx *myio, int NetTBS, uint32_t esi, symvec *packets);
void RQ_pushTB(int *viINFObits, nanorq *rq, symvec *subpackets);
bool RQ_isBlockend(nanorq *rq, uint32_t esi, float overhead);
void RQ_encoder_free(nanorq *rq, struct ioctx *myio);
