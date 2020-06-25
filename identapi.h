// Idenapi.h Single header Library
#ifndef IDENTAPI_H
#define IDENTAPI_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#pragma pack(1)
struct identapi_struct
{
    union {
        uint32_t RAW;
        struct {
            uint32_t REVISION: 4;
            uint32_t TYPE: 8;  
            uint32_t PROCESSOR: 4;
            uint32_t MANUFACTURER: 4;  
            uint32_t MEMORY_SIZE: 3;
            uint32_t NEW_FLAG: 1;
            uint32_t RESERVERED_2: 1;
            uint32_t WARRANTY: 1;
            uint32_t RESERVERED_1: 3;
            uint32_t OPT_READ: 1;
            uint32_t OTP_PROGRAM: 1;
            uint32_t OVERVOLT_ALLOWED: 1;
        };
    };
};
#pragma pack()

enum identapi_enum {
  IDENTAPI_REV,
  IDENTAPI_TYPE,
  IDENTAPI_PROC,
  IDENTAPI_MFG,
  IDENTAPI_MEM
};

const char *IDENTAPI_TYPES [20] =
{
  "A", "B",	 "A+", "B+",	 
  "2B", "Alpha", "CM1",
  "--RESERVERED--",
  "3B", "Zero", "CM3",
  "--RESERVERED--",
  "Zero W", "3B+", "3A+",
  "--RESERVERED--",
  "CM3+", "4B",
  "--RESERVERED--",
  "--RESERVERED--",
} ;

const char *IDENTAPI_MFGS [17] =
{
  "Sony UK",
  "Egoman",
  "Embest",
  "Sony Japan",
  "Embest",	
  "Stadium",	
  "--RESERVERED--",	
  "--RESERVERED--",	
  "--RESERVERED--",	
  "--RESERVERED--",	
  "--RESERVERED--",	
  "--RESERVERED--",	
  "--RESERVERED--",	
  "--RESERVERED--",	
  "--RESERVERED--",	
  "--RESERVERED--",	
  "Qisda",	
} ;

const char *IDENTAPI_PROCS [8] =
{
  "BCM2835",
  "BCM2836",
  "BCM2837",
  "BCM2711",
  "--RESERVERED--",
  "--RESERVERED--",
  "--RESERVERED--",
  "--RESERVERED--"
};

const int IDENTAPI_MEMS [8] =
{
   256, 512, 1024, 2048, 4096, 8192, 0, 0
};

uint32_t IDENTAPI_GET_Revision()
{
  FILE *cpuFd ;
  char line [120] ;
  char *c ;

  if ((cpuFd = fopen ("/proc/cpuinfo", "r")) == NULL)
  {
    printf ("Unable to open /proc/cpuinfo") ;
    return 1;
  }

  while (fgets (line, 120, cpuFd) != NULL)
    if (strncmp (line, "Revision", 8) == 0)
      break ;

  fclose (cpuFd) ;

  if (strncmp (line, "Revision", 8) != 0)
  {
     return 1;
  }

  for (c = &line [strlen (line) - 1] ; (*c == '\n') || (*c == '\r') ; --c)
    *c = 0 ;

  for (c = line ; *c ; c++) if (*c == ':') break ;

  if (*c != ':')
  {
	return 1;
  }

  c++ ;
  while (isspace (*c)) c++;
  if (!isxdigit (*c))
  {
    return 1;
  }
  return (uint32_t)strtol (c, NULL, 16) ; // Hex number with no leading 0x
}

int IDENTAPI_Parse_old (struct identapi_struct* rev, uint32_t revision_code)
{
  rev->RAW = revision_code;
  if (! rev->NEW_FLAG)
  { 
    rev->RAW = 0; // Reset values to 0x0
    if ((revision_code >= 0x2 && revision_code <= 0x15) && !(revision_code <= 0xc && revision_code >= 0xa ))// valid range
    {
	   if (revision_code <= 0x7 && revision_code >= 0x9) rev->TYPE = 0x0;  // A BCM2835 256mb 
       if (revision_code < 0x7 || (revision_code > 0xc && revision_code < 0x10) ) rev->TYPE = 0x1;  // B BCM2835 256mb
       if (revision_code > 0xc && revision_code != 0x12) rev->MEMORY_SIZE = 0x1; // 512MB
       if (revision_code == 0x10 || revision_code == 0x13) { rev->TYPE = 0x3; rev->REVISION = 2; } // B+
       if (revision_code == 0x11 || revision_code == 0x14) { rev->TYPE = 0x6; rev->REVISION = 0; }// CM1
       if (revision_code == 0x12 || revision_code == 0x15) { rev->TYPE = 0x2; rev->REVISION = 1; } // A+
       if (revision_code < 0x10 && revision_code > 0x3) rev->REVISION = 10;
       if (revision_code < 0x4 || (revision_code > 0x5 && revision_code < 0x8) || revision_code == 0xd || revision_code == 0xf) rev->MANUFACTURER = 0x1; // Egoman
       if (revision_code > 0x12 ) rev->MANUFACTURER = 0x2; // Embest
       if (revision_code == 0x5 || revision_code == 0x9) rev->MANUFACTURER = 0xf;// 0x10; // Qisda
	} else {
	  return 1;
    }
  }
  return 0;
}

const char* IDENTAPI_GET_str (struct identapi_struct rev, enum identapi_enum ret_type)
{
  switch( ret_type )
  {
    case IDENTAPI_TYPE : return IDENTAPI_TYPES[rev.TYPE];
    case IDENTAPI_PROC : return IDENTAPI_PROCS[rev.PROCESSOR];
    case IDENTAPI_MFG  : return IDENTAPI_MFGS[rev.MANUFACTURER];
    default: return NULL;
  }
  return NULL;
}

uint32_t IDENTAPI_GET_int (struct identapi_struct rev, enum identapi_enum ret_type)
{
  switch( ret_type )
  {
    case IDENTAPI_MEM  : return (uint32_t)IDENTAPI_MEMS[rev.MEMORY_SIZE];
    default: return 0;
  }
  return 0;
}

#endif
