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

struct OTI_python
{
  uint64_t oti_common;
  uint32_t oti_scheme;
  size_t packet_size;
  float overhead;    //（repair symbol number/source symbol number）+1
  size_t srcsymNum;  //source symbol number
  size_t transNum;   //本次传输的symbol number
  size_t totalTrans; //当前块以传输的总数
  bool Endflag;      //当前block结束标志
};

typedef kvec_t(struct sym) symvec;

void random_bytes(uint8_t *buf, uint64_t len);
uint8_t *RQ_encode_init(nanorq **rq, struct ioctx **myio_in, size_t num_packets, size_t packet_size, bool precalc);
void RQ_generate_symbols(nanorq *rq, struct ioctx *myio, int sbn, symvec *packets, uint32_t esi);
uint32_t RQ_receive(nanorq *rq, struct ioctx *myio, int NetTBS, uint32_t esi, symvec *packets);
int RQ_decode(uint8_t *Receiverbuff, struct OTI_python *oti, int totalRecvr);
void RQ_buffTosymvec(uint8_t *Receiverbuff, struct OTI_python *oti, symvec *packets, int totalRecvr);
void RQ_decodePush(int *viINFObits, uint8_t *Receiverbuff, int *viCRCs_pool, struct OTI_python *oti,
                   int *totalRecvr, int iRecvrM, int iSendrN, int MaxTBs, int Maxblocksize);
void RQ_encodePush(int *viINFObits, uint8_t *Senderbuff, int packet_size, int NetTBS, struct OTI_python *oti);
void RQ_saveEncoded_data(uint8_t *Senderbuff, nanorq *rq, symvec *packets);
void RQ_encodedData(nanorq *rq, struct ioctx *myio, symvec *packets, float overhead);
uint64_t RQ_decodeControl(uint8_t *Receiverbuff, struct OTI_python *oti,
                          int *totalRecvr, int iRecvrM, int iSendrN, int Maxblocksize);
void RQ_encodeControl(uint8_t *Senderbuff,
                      int *viINFObits, int *viTBs, struct OTI_python *oti_python,
                      unsigned long Maxblocksize, size_t num_packets, size_t packet_size, int MaxTBs, int iSendrN);
