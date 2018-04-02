#include <stdlib.h>
#include <string.h>

#include "fwk.h"

static char _randstrbuf[MAX_RANDSTRING_LEN+1];
static char *randspace="abcdefghijklmnopqurstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";


char *utilRand_getRandString(UINT32 minchars, UINT32 maxchars)
{
   int i;
   int mod;

   if (maxchars > MAX_RANDSTRING_LEN)
      return NULL;

   if (maxchars < minchars)
      return NULL;

   if (minchars < 1)
      return NULL;

   memset(_randstrbuf, 0, sizeof(_randstrbuf));
   for (i=0; i < maxchars; i++)
   {
      // first char is a-z, rest of chars can be anything
      mod = (0==i) ? 26 : strlen(randspace);
      _randstrbuf[i] = randspace[rand() % mod];

      if (i+1 >= minchars)
      {
         // we've satisfied the minchars requirement, see if we break
         if (0 == (rand() % 2))
            break;
      }
   }

   return _randstrbuf;
}
