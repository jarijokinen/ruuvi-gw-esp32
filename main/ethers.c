#include <string.h>

#include "ethers.h"

extern const uint8_t ethers_start[] asm("_binary_ethers_start");
extern const uint8_t ethers_end[] asm("_binary_ethers_end");

int ruuvi_gw_ether_to_name(char *ether, char *name, int len)
{
  char *pos_start, *pos_end;

  pos_start = strcasestr((char *)ethers_start, ether);

  if(pos_start == NULL)
    return -1;

  pos_start += strlen(ether)+1;
  pos_end = strstr(pos_start, "\n");

  if(pos_end == NULL)
    return -1;

  if(pos_end-pos_start+1 > len)
    return -2;

  strlcpy(name, pos_start, pos_end-pos_start+1);

  return 0;
}
