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
  uint8_t *Senderbuff = malloc(sizeof(uint8_t) * 200);
  memset(Senderbuff, 0, sizeof(uint8_t) * 200);

  unsigned long Maxblocksize = 100;

  int MaxTBs = 100; //一次最多传100bits
  int *viINFObits = malloc(sizeof(int) * 200);
  memset(viINFObits, 0, sizeof(int) * 200);

  int viTB[2] = {100, 50};
  int *viTBs = viTB;




  struct OTI_python oti_python[2];
  struct OTI_python *oti = oti_python;

  for (int i = 0; i < 2; i++)
  {
    oti_python[i].Endflag = true;
    oti_python[i].overhead = 1.5;
    oti_python[i].totalTrans = 0;
  }

  size_t num_packets = 2;
  size_t packet_size = 2;
  int iSendrN = 2;

  RQ_encodeControl(Senderbuff,
                   viINFObits, viTBs, oti_python,
                   Maxblocksize, num_packets, packet_size, MaxTBs, iSendrN);

  memset(viINFObits, 0, sizeof(int) * 200);

  RQ_encodeControl(Senderbuff,
                   viINFObits, viTBs, oti_python,
                   Maxblocksize, num_packets, packet_size, MaxTBs, iSendrN);
                   
  return 0;
}
