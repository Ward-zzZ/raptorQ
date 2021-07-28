#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <nanorq.h>

#include "kvec.h"

struct sym
{
  uint32_t tag;
  uint8_t *data;
};

typedef kvec_t(struct sym) symvec;

uint64_t usecs()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * (uint64_t)1000000 + tv.tv_usec);
}

void random_bytes(uint8_t *buf, uint64_t len)
{
  for (int i = 0; i < len; i++)
  {
    buf[i] = (uint8_t)rand();
  }
}

void dump_esi(nanorq *rq, struct ioctx *myio, int sbn, uint32_t esi,
              symvec *packets)
{
  int packet_size = nanorq_symbol_size(rq);
  uint8_t *data = malloc(packet_size); //保留一个symbol的空间
  uint64_t written = nanorq_encode(rq, (void *)data, esi, sbn, myio);

  if (written != packet_size)
  {
    free(data);
    fprintf(stderr, "failed to encode packet data for sbn %d esi %d.", sbn,
            esi);
    exit(1);
  }
  else
  {
    uint32_t tag = nanorq_tag(sbn, esi);
    struct sym s = {.tag = tag, .data = data};
    kv_push(struct sym, *packets, s); //保存一个symbol数据
  }
}

void dump_block(nanorq *rq, struct ioctx *myio, int sbn, symvec *packets,
                float overhead_pct, float expected_loss)
{
  int num_esi = nanorq_block_symbols(rq, sbn);
  int overhead = (int)(num_esi * overhead_pct - 20);
  // fprintf(stderr, "overhead %d failed.\n", overhead);
  int num_dropped = 0, num_rep = 0;
  for (uint32_t esi = 0; esi < num_esi; esi++)
  {
    float dropped = ((float)(rand()) / (float)RAND_MAX) * (float)100.0;
    float drop_prob = expected_loss;
    if (dropped < drop_prob)
    {
      num_dropped++;
    }
    else
    {
      dump_esi(rq, myio, sbn, esi, packets);
    }
  }
  for (uint32_t esi = num_esi; esi < num_esi + overhead; esi++)
  {
    float dropped = ((float)(rand()) / (float)RAND_MAX) * (float)100.0;
    float drop_prob = expected_loss;
    if (dropped < drop_prob)
    {
      continue;
    }
    else
    {
      dump_esi(rq, myio, sbn, esi, packets);
      num_rep++;
    }
  }

  nanorq_encoder_cleanup(rq, sbn);
  // fprintf(stdout, "block %d is %d packets, source dropped %d, repair dropped %d\n",
  //         sbn, num_esi, num_dropped, overhead - num_rep);
}

void usage(char *prog)
{
  fprintf(stderr,
          "usage:\n%s <packet_size> <num_packets> <overhead> <expected_loss>\n",
          prog);
  exit(1);
}

double encode(uint64_t len, size_t packet_size, size_t num_packets,
              float overhead, float expected_loss, struct ioctx *myio, symvec *packets,
              uint64_t *oti_common, uint32_t *oti_scheme, bool precalc)
{
  nanorq *rq = nanorq_encoder_new_ex(len, packet_size, num_packets, 0, 1);

  if (rq == NULL)
  {
    fprintf(stderr, "Could not initialize encoder.\n");
    return -1;
  }

  *oti_common = nanorq_oti_common(rq);
  *oti_scheme = nanorq_oti_scheme_specific(rq);

  if (precalc)
    nanorq_precalculate(rq);
  int num_sbn = nanorq_blocks(rq);
  double elapsed = 0.0;

  for (int b = 0; b < num_sbn; b++)
  {
    nanorq_generate_symbols(rq, b, myio);
    nanorq_encoder_reset(rq, 0);
  }

  for (int sbn = 0; sbn < num_sbn; sbn++)
  {
    dump_block(rq, myio, sbn, packets, overhead, expected_loss);
  }
  nanorq_free(rq);
  return elapsed;
}

bool decode(uint64_t oti_common, uint32_t oti_scheme, struct ioctx *myio,
            symvec *packets)
{

  nanorq *rq = nanorq_decoder_new(oti_common, oti_scheme);
  if (rq == NULL)
  {
    fprintf(stderr, "Could not initialize decoder.\n");
    return -1;
  }

  int num_sbn = nanorq_blocks(rq);

  for (int i = 0; i < kv_size(*packets); i++)
  {
    struct sym s = kv_A(*packets, i);
    if (NANORQ_SYM_ERR ==
        nanorq_decoder_add_symbol(rq, (void *)s.data, s.tag, myio))
    {
      fprintf(stderr, "adding symbol %d to sbn %d failed.\n",
              s.tag & 0x00ffffff, (s.tag >> 24) & 0xff);
      exit(1);
    }
  }

  for (int sbn = 0; sbn < num_sbn; sbn++)
  {
    if (!nanorq_repair_block(rq, myio, sbn))
    {
      // fprintf(stderr, "decode of sbn %d failed.\n", sbn);
      return false;
    }
  }
  nanorq_encoder_reset(rq, 0);

  for (int sbn = 0; sbn < num_sbn; sbn++)
  {
    nanorq_encoder_cleanup(rq, sbn);
  }
  nanorq_free(rq);
  return true;
}

void clear_packets(symvec *packets)
{
  if (kv_size(*packets) > 0)
  {
    for (int i = 0; i < kv_size(*packets); i++)
    {
      free(kv_A(*packets, i).data);
    }
    kv_destroy(*packets);
  }
  kv_init(*packets);
}

bool run(size_t num_packets, size_t packet_size, float overhead, float expected_loss)
{

  uint64_t oti_common = 0;
  uint32_t oti_scheme = 0;
  struct ioctx *myio_in, *myio_out;

  uint64_t sz = num_packets * packet_size; //源数据大小，bytes
  uint8_t *in = calloc(1, sz);             //编码输入数据
  uint8_t *out = calloc(1, sz);            //解码输出数据
  random_bytes(in, sz);                    //generate sz bytes source data

  myio_in = ioctx_from_mem(in, sz);
  if (!myio_in)
  {
    fprintf(stderr, "couldnt access mem at %p\n", in);
    free(in);
    free(out);
    return -1;
  }

  myio_out = ioctx_from_mem(out, sz);
  if (!myio_out)
  {
    fprintf(stderr, "couldnt access mem at %p\n", out);
    free(in);
    free(out);
    return -1;
  }

  symvec packets;   //源数据向量，包含Pay Load ID + data
  kv_init(packets); //该变量保存编码后的数据，并发送给解码器

  //一次编码一个sz bytes的block
  encode(sz, packet_size, num_packets, overhead, expected_loss, myio_in,
         &packets, &oti_common, &oti_scheme, true);
  decode(oti_common, oti_scheme, myio_out, &packets);

  myio_in->destroy(myio_in);
  myio_out->destroy(myio_out);
  // verify
  for (int i = 0; i < sz; i++)
  {
    if (in[i] != out[i])
      return false;
  }

  // cleanup
  clear_packets(&packets);

  free(in);
  free(out);
  return true;
}

int main(int argc, char *argv[])
{
  if (argc < 4)
    usage(argv[0]);

  srand((unsigned int)time(0));

  // determine chunks, symbol size
  size_t packet_size = strtol(argv[1], NULL, 10); // T
  size_t num_packets = strtol(argv[2], NULL, 10); // K
  float overhead = atof(argv[3]);                 // overhead packets number
  float expected_loss = strtof(argv[4], NULL);
  int fail_times = 0;
  int sim_times = 10000;
  //overhead:1-6
  fprintf(stdout, "Overhead       Psuc\n");
  for (int i = 0; i < 41; i++)
  {
    fail_times = 0;
    overhead = (float)((i + 20.0) / 20);
    for (int j = 0; j < sim_times; j++)
    {
      if (!(run(num_packets, packet_size, overhead, expected_loss))) //一次发送一个block（packet_size和Al对齐的情况，否则为2个block）
        fail_times += 1;
    }
    // fprintf(stderr, "decode of sbn %6.2f failed.%d\n", overhead, fail_times);
    fprintf(stdout, "%10.2f %10.2f \n", overhead, (float)(100-100*(float)fail_times/sim_times));
  }
  return 0;
}
