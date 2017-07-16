#include <stdint.h>
#include "strtok.h"

static int16_t itok;
static int16_t slen;
static char* ptok;
static char* ps;

char * _strtok(char *s, char * del){
  int16_t i,j, dlen;
  if (s!=0){
    ps = s;
    itok=0;
    slen=0;
    while (s[slen]) slen++;
    for (i=0; i<slen; i++){
      for (j=0; del[j]; j++){
        if (s[i] == del[j]) 
          s[i] = 0;
      }
    }
  }
  i=j=0;
  while (i<slen){
    if (ps[i]) {
      if (j==itok){ 
        itok++;
        return &ps[i];
      }
      while (ps[i]) i++;
      j++;
    }
    else i++;
  }
  return 0;
}
