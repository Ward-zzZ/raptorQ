// #include <stdio.h>
// #include <math.h>
// #include <stdint.h>
// #include <assert.h>
// #include <math.h>
// #include <stdlib.h>
// #include <string.h>

// int main()
// {
//   uint32_t i = 429496729;
//   for (int j = 31; j >= 0; j--)
//   {
//     if (i & (1 << j))
//       fprintf(stderr, "1");
//     else
//       fprintf(stderr, "0");
//   }
//   fprintf(stderr, "\n");
//   return 0;
// }
#include "RaptorQ.h"

int main()
{

  size_t num_packets = 2;
  size_t packet_size = 2;
  int iSendrN = 2;
  int iRecvrM = 1;
  unsigned long Maxblocksize = 10000;
  int MaxTBs = 75400; //一次最多传bits

  uint8_t *Senderbuff = malloc(sizeof(uint8_t) * Maxblocksize * iSendrN);
  memset(Senderbuff, 0, sizeof(uint8_t) * Maxblocksize * iSendrN);

  uint8_t *Receiverbuff = malloc(sizeof(uint8_t) * Maxblocksize * iSendrN * iRecvrM);
  memset(Receiverbuff, 0, sizeof(uint8_t) * Maxblocksize * iSendrN * iRecvrM);

  int *viINFObits = malloc(sizeof(int) * 75400 * iSendrN);
  memset(viINFObits, 0, sizeof(int) * 75400 * iSendrN);

  int viTB[2] = {100, 50};
  int *viTBs = viTB;

  int CRCs_pool[2] = {0, 0};
  int *viCRCs_pool = CRCs_pool;

  int totalRecvr[2] = {0, 0};
  int *vitotalRecvr = totalRecvr;

  struct OTI_python oti_python[2];
  struct OTI_python *oti = oti_python;

  for (int i = 0; i < 2; i++)
  {
    oti_python[i].Endflag = true;
    oti_python[i].overhead = 1.5;
    oti_python[i].totalTrans = 0;
  }

  RQ_encodeControl(Senderbuff,
                   viINFObits, viTBs, oti_python,
                   Maxblocksize, num_packets, packet_size, MaxTBs, iSendrN);

  RQ_decodePush(viINFObits, Receiverbuff, viCRCs_pool, oti,
                vitotalRecvr,  iRecvrM,  iSendrN,  MaxTBs,  Maxblocksize);

   RQ_decodeControl(Receiverbuff,  oti,
                            totalRecvr,  iRecvrM,  iSendrN,  Maxblocksize);

      RQ_encodeControl(Senderbuff,
                       viINFObits, viTBs, oti_python,
                       Maxblocksize, num_packets, packet_size, MaxTBs, iSendrN);

  return 0;
}
