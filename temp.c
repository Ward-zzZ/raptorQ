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
  nanorq *rq [2];
  struct ioctx *myio_in ;
  RQ_encode_init(&rq[1], &myio_in, 2, 1, true);
  symvec packets;
  kv_init(packets);
  RQ_receive(rq[1], myio_in, 80, 0, &packets);
  int viINFObits[100]={0};
  RQ_pushTB(viINFObits, rq[1], &packets);
  for(int i =0;i<100;i++)
  {
    fprintf(stderr, " %d:%d\n", i,viINFObits[i]);
  }
  return 0;
}
