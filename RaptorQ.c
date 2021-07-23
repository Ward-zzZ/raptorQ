#include "RaptorQ.h"

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
                int overhead, float expected_loss)
{
  int num_esi = nanorq_block_symbols(rq, sbn);
  int num_dropped = 0, num_rep = 0;
  fprintf(stderr, "expected_loss:  %f .\n", expected_loss);
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
  fprintf(stdout, "block %d is %d packets, source dropped %d, repair dropped %d\n",
          sbn, num_esi, num_dropped, overhead - num_rep);
}

double encode(uint64_t len, size_t packet_size, size_t num_packets,
              int overhead, float expected_loss, struct ioctx *myio, symvec *packets,
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
      fprintf(stderr, "decode of sbn %d failed.\n", sbn);
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

bool run(size_t num_packets, size_t packet_size, int overhead, float expected_loss)
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
//--------------------------------------------------------------------------------------------
/*
*根据给定的源symbol数目和大小，生成随机数据，并初始化一个RaptorQ编码器
*/
uint8_t *RQ_encode_init(nanorq **rq, struct ioctx **myio_in, size_t num_packets, size_t packet_size, bool precalc)
{

  uint64_t len = num_packets * packet_size; //源数据大小，bytes
  uint8_t *in = calloc(1, len);             //编码输入数据
  random_bytes(in, len);                    //generate len bytes source data

  *myio_in = ioctx_from_mem(in, len);
  if (!*myio_in)
  {
    fprintf(stderr, "couldnt access mem at %p\n", in);
    free(in);
    exit(1);
  }

  *rq = nanorq_encoder_new_ex(len, packet_size, num_packets, 0, 1);

  if (*rq == NULL)
  {
    fprintf(stderr, "Could not initialize encoder.\n");
    exit(1);
  }

  if (precalc)
    nanorq_precalculate(*rq);
  int num_sbn = nanorq_blocks(*rq);
  if (num_sbn != 1)
  {
    fprintf(stderr, "Dont support multiple blocks.\n");
    exit(1);
  }

  for (int b = 0; b < num_sbn; b++)
  {
    nanorq_generate_symbols(*rq, b, *myio_in);
    nanorq_encoder_reset(*rq, 0);
  }

  return in;
}

/*
*根据给定的sbn,esi和编码器，生成并保存对应编号的symbol
*/
void RQ_generate_symbols(nanorq *rq, struct ioctx *myio, int sbn, symvec *packets, uint32_t esi)
{
  int packet_size = nanorq_symbol_size(rq); //单位字节
  uint8_t *data = malloc(packet_size);      //保留一个symbol的空间
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
  // nanorq_encoder_cleanup(rq, sbn);
}

/*
*给定当前TB size、rq编码器和起始esi	，生成不大于TB size的若干个symbol,返回下一个esi
*/
uint32_t RQ_receive(nanorq *rq, struct ioctx *myio, int NetTBS, uint32_t esi, symvec *packets)
{
  int packet_size = nanorq_symbol_size(rq); //单位字节
  int symbol_Bits = packet_size * 8 + 32;   //发送的一个symbol+Payload id的总bit数
  int symbol_num = (NetTBS) / (symbol_Bits);

  for (int i = 0; i < symbol_num; i++)
  {
    RQ_generate_symbols(rq, myio, 0, packets, esi + i);
  }
  return (uint32_t)(esi + symbol_num);
}

void RQ_encodedData(nanorq *rq, struct ioctx *myio, symvec *packets, float overhead)
{
  int sbn = 0;
  int num_esi = nanorq_block_symbols(rq, sbn);
  int packet_size = nanorq_symbol_size(rq);
  int total_symbolNum = (int)(overhead * num_esi);
  for (uint32_t esi = 0; esi < total_symbolNum; esi++)
  {
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
}

void RQ_saveEncoded_data(uint8_t *Senderbuff, nanorq *rq, symvec *packets)
{
  // int packet_size = nanorq_symbol_size(rq); //单位字节,symbol长度
  int packet_size = nanorq_symbol_size(rq); //单位字节
  int sum = 0;
  struct sym s;
  uint32_t esi;
  uint8_t *data;
  for (int i = 0; i < kv_size(*packets); i++)
  {
    s = kv_A(*packets, i);
    esi = s.tag;
    data = s.data;
    //esi
    for (int j = 3; j >= 0; j--)
    {
      *Senderbuff = (esi >> (j * 8)) & 0xFF;
      Senderbuff++;
      // printf("%d\n", a[i]);
    }
    //source data
    for (int m = 0; m < packet_size; m++)
    {
      *Senderbuff = *data;
      *Senderbuff++;
      data++;
    }
  }
}

void RQ_encodePush(int *viINFObits, uint8_t *Senderbuff, int packet_size, int NetTBS, struct OTI_python *oti)
{
  int sum = 0;
  int symbol_Bits = packet_size * 8 + 32; //发送的一个symbol+Payload id的总bit数
  int symbol_num = (NetTBS) / (symbol_Bits);
  if (symbol_num == 0)
  {
    fprintf(stderr, "TBs<symbol_Bits\n");
    exit(1);
  }
  int total_symbolNum = (int)(oti->overhead * oti->srcsymNum);
  int startesi = oti->totalTrans;
  if ((symbol_num + startesi) >= total_symbolNum)
  {
    symbol_num = total_symbolNum - startesi;
    oti->Endflag = true;
  }
  Senderbuff += startesi * (packet_size + 4);
  for (int i = 0; i < symbol_num * symbol_Bits; i++)
  {
    uint8_t bytedata = *Senderbuff;
    Senderbuff++;
    for (int n = 7; n >= 0; n--)
    {
      if (bytedata & (1 << n))
        *viINFObits = 1;
      else
        *viINFObits = 0;
      viINFObits++;
      sum++;
    }
  }
  oti->transNum = (size_t)symbol_num; //更新本次传输的symbol num
  oti->totalTrans = (size_t)(symbol_num + startesi);
  // fprintf(stderr, "总共压入数据 %d比特.\n", sum);
}

void RQ_encodeControl(uint8_t *Senderbuff,
                      int *viINFObits, int *viTBs, struct OTI_python *oti_python,
                      unsigned long Maxblocksize, size_t num_packets, size_t packet_size, int MaxTBs, int iSendrN)
{
  for (int i = 0; i < iSendrN; i++)
  {

    Senderbuff += i * Maxblocksize;
    viINFObits += i * MaxTBs;
    viTBs += i;
    oti_python += i;

    //创建新的block
    if (oti_python->Endflag)
    {
      nanorq *rq;
      struct ioctx *myio_in;
      oti_python->totalTrans = 0;
      oti_python->srcsymNum = (size_t)num_packets;
      uint8_t *srcData = RQ_encode_init(&rq, &myio_in, num_packets, packet_size, true);
      symvec packets;
      kv_init(packets);
      RQ_encodedData(rq, myio_in, &packets, oti_python->overhead);
      RQ_saveEncoded_data(Senderbuff, rq, &packets);

      //更新OTI
      oti_python->oti_common = nanorq_oti_common(rq);
      oti_python->oti_scheme = nanorq_oti_scheme_specific(rq);
      oti_python->packet_size = (size_t)packet_size;
      oti_python->Endflag = false;

      RQ_encodePush(viINFObits, Senderbuff, packet_size, *viTBs, oti_python); //更新当前block已经发送的symbol num

      //释放内存空间
      // nanorq_free(rq);
      // myio_in->destroy(myio_in);
      clear_packets(&packets);
      // free(srcData);
    }
    //继续传输未完成的block
    else
    {
      RQ_encodePush(viINFObits, Senderbuff, packet_size, *viTBs, oti_python); //更新当前block已经发送的symbol num
    }
  }
}

/*-----------------------------------------------DECODE-----------------------------------------------------------------*/

/*
 * 根据底层解码情况，将数据放入对应的缓存区
 * viINFObits: 底层解码出来的01比特流
 * Receiverbuff: 接收机的缓存区，以byte的形式存储(iSendrN*iRecvrM)
 * viCRCs_pool: 本次传输的CRC(iSendrN*iRecvrM)
 * oti: 发送方的OTI信息
 * totalRecvr: 接收机对当前块成功接收的symbol数
 */
void RQ_decodePush(int *viINFObits, uint8_t *Receiverbuff, int *viCRCs_pool,
                   struct OTI_python *oti, int *totalRecvr, int iRecvrM,
                   int iSendrN, int MaxTBs, int Maxblocksize)
{
  for (int i = 0; i < iSendrN; i++)
  {
    int packet_size = oti->packet_size;
    int symbol_Bits = packet_size * 8 + 32; //发送的一个symbol+Payload id的总bit数
    int symbol_num = oti->transNum;         //本次传输的symbol总数
    for (int j = 0; j < iRecvrM; j++)
    {
      uint8_t *oneBolockbuff = Receiverbuff + (j + i * iRecvrM) * Maxblocksize;
      // totalRecvr += (j + i * iRecvrM);
      // viCRCs_pool += (j + i * iRecvrM);

      if (*viCRCs_pool == 1)
      {
        //当前接收机接收当前发送方的信息失败
        totalRecvr++;
        viCRCs_pool++;
        continue;
      }
      else
      {
        //移动指针到当前buff的末尾
        oneBolockbuff += (*totalRecvr) * (packet_size + 4);
        int byteindex = 7;
        uint8_t bytedata = 0;
        for (int m = 0; m < symbol_num * symbol_Bits; m++)
        {
          bytedata += (uint8_t)pow(2.0, byteindex) * (*(viINFObits + m));
          byteindex--;
          if (byteindex == -1)
          {
            byteindex = 7;
            *oneBolockbuff = bytedata;
            oneBolockbuff++;
            bytedata = 0;
          }
        }
        //总接收的块数
        *totalRecvr = *totalRecvr + symbol_num;
        totalRecvr++;
        viCRCs_pool++;
      }
    }
    oti++;
    memset(viINFObits, 0, sizeof(int) * MaxTBs);
    viINFObits += MaxTBs;
  }
}
//r1s1 r2s1 r1s2 r2s2

/*
 * 根据block传输的结束与否控制RQ解码
 * 返回本次传输解码成功的bit数
 */
uint64_t RQ_decodeControl(uint8_t *Receiverbuff, struct OTI_python *oti,
                          int *totalRecvr, int iRecvrM, int iSendrN, int Maxblocksize)
{
  uint64_t SumOKBITS = 0;
  uint8_t *viCRCblock = malloc(sizeof(uint8_t) * iSendrN); //每个发送block的CRC
  uint8_t *free_point = viCRCblock;
  memset(viCRCblock, 1, iSendrN);
  for (int i = 0; i < iSendrN; i++)
  {
    oti += i;
    viCRCblock += i;
    if (oti->Endflag)
    {
      for (int j = 0; j < iRecvrM; j++)
      {
        uint8_t *oneBolockbuff = Receiverbuff + (j + i * iRecvrM) * Maxblocksize;

        // totalRecvr += (j + i * iRecvrM);
        if (*totalRecvr < oti->srcsymNum)
        {
          *viCRCblock = ((*viCRCblock) <= (1) ? (*viCRCblock) : (1)); //接收的symbol太少，解码失败
        }
        else
        {
          int temp = RQ_decode(oneBolockbuff, oti, *totalRecvr); //解码状态
          *viCRCblock = ((*viCRCblock) <= (temp) ? (*viCRCblock) : (temp));
        }
        *totalRecvr = 0;
        totalRecvr++;
        // memset(oneBolockbuff, 0, Maxblocksize);
      }
      SumOKBITS += (1 - *viCRCblock) * oti->packet_size * oti->srcsymNum * 8;
    }
    else
    {
      continue;
    }
  }
  free(free_point);
  return SumOKBITS;
}

void RQ_buffTosymvec(uint8_t *Receiverbuff, struct OTI_python *oti, symvec *packets, int totalRecvr)
{
  int packet_size = oti->packet_size;
  for (int i = 0; i < totalRecvr; i++)
  {
    uint32_t tag = 0;
    for (int j = 3; j >= 0; j--)
    {
      tag = *Receiverbuff << (j * 8) | tag;
      *Receiverbuff = 0;
      Receiverbuff++;
    }
    uint8_t *data = malloc(sizeof(uint8_t) * packet_size); //保留一个symbol的空间
    for (int m = 0; m < packet_size; m++)
    {
      *(data + m) = *Receiverbuff;
      *Receiverbuff = 0;
      Receiverbuff++;
    }
    struct sym s = {.tag = tag, .data = data};
    kv_push(struct sym, *packets, s); //保存一个symbol数据
  }
}

int RQ_decode(uint8_t *Receiverbuff, struct OTI_python *oti, int totalRecvr)
{
  uint64_t oti_common = oti->oti_common;
  uint32_t oti_scheme = oti->oti_scheme;

  nanorq *rq = nanorq_decoder_new(oti_common, oti_scheme);
  if (rq == NULL)
  {
    fprintf(stderr, "Could not initialize decoder.\n");
    return -1;
  }

  int num_sbn = nanorq_blocks(rq);

  symvec packets;
  kv_init(packets);

  //将buff的数据以symbol的格式存入packet中
  RQ_buffTosymvec(Receiverbuff, oti, &packets, totalRecvr);

  struct ioctx *myio_out;

  uint64_t sz = oti->packet_size * oti->srcsymNum; //源数据大小，bytes
  uint8_t *out = calloc(1, sz);                    //解码输出数据

  myio_out = ioctx_from_mem(out, sz);
  if (!myio_out)
  {
    fprintf(stderr, "couldnt access mem at %p\n", out);
    free(out);
    return -1;
  }

  for (int i = 0; i < kv_size(packets); i++)
  {
    struct sym s = kv_A(packets, i);
    if (NANORQ_SYM_ERR ==
        nanorq_decoder_add_symbol(rq, (void *)s.data, s.tag, myio_out))
    {
      fprintf(stderr, "adding symbol %d to sbn %d failed.\n",
              s.tag & 0x00ffffff, (s.tag >> 24) & 0xff);
      exit(1);
    }
  }

  //RQ解码
  for (int sbn = 0; sbn < num_sbn; sbn++)
  {
    if (!nanorq_repair_block(rq, myio_out, sbn))
    {
      fprintf(stderr, "decode of sbn %d failed.\n", sbn);
      return 1;
    }
  }
  nanorq_encoder_reset(rq, 0);

  for (int sbn = 0; sbn < num_sbn; sbn++)
  {
    nanorq_encoder_cleanup(rq, sbn);
  }
  myio_out->destroy(myio_out);
  nanorq_free(rq);
  free(out);
  return 0;
}

// void RQ_pushTB(int *viINFObits, nanorq *rq, symvec *subpackets)
// {
//   int packet_size = nanorq_symbol_size(rq); //单位字节
//   int sum = 0;
//   for (int i = 0; i < kv_size(*subpackets); i++)
//   {
//     struct sym s = kv_A(*subpackets, i);
//     uint32_t esi = s.tag;
//     uint8_t *data = s.data;
//     //esi
//     for (int j = 31; j >= 0; j--)
//     {
//       if (esi & (1 << j))
//         *viINFObits = 1;
//       else
//         *viINFObits = 0;
//       viINFObits++;
//       sum++;
//     }
//     //source data
//     for (int m = 0; m < packet_size; m++)
//     {
//       uint8_t bytedata = *data;
//       data++;
//       for (int n = 7; n >= 0; n--)
//       {
//         if (bytedata & (1 << n))
//           *viINFObits = 1;
//         else
//           *viINFObits = 0;
//         viINFObits++;
//         sum++;
//       }
//     }
//   }
//   fprintf(stderr, "总共压入数据 %d比特.\n", sum);
// }
