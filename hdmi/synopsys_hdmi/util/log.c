/*
 * log.c
 *
 *  Created on: Jun 25, 2010
 * 
 * Synopsys Inc.
 * SG DWC PT02
 */

#include "log.h"
#include <common.h>


static verbose_t log_Verbose = VERBOSE_NOTICE;
static numeric_t log_Numeric = NUMERIC_HEX;
static unsigned log_VerboseWrite = 0;

void log_SetVerbose(verbose_t verbose)
{
	log_Verbose = verbose;
}

void log_SetNumeric(numeric_t numeric)
{
	log_Numeric = numeric;
}

void log_SetVerboseWrite(unsigned state)
{
	log_VerboseWrite = state;
}

void log_PrintWrite(unsigned a, unsigned b)
{
	if (log_VerboseWrite == 1)
	{
		if (log_Numeric == NUMERIC_DEC)
		{
			printf("%d, %d\n", a, b);
		}
		else
		{
			printf("0x%x, 0x%x\n", a, b);
		}
	}
}

void log_Print0(verbose_t verbose, const char* functionName)
{
	if (verbose == VERBOSE_ERROR)
	{
		printf("ERROR: ");
	}
	if (verbose == VERBOSE_WARN)
	{
		printf("WARNING: ");
	}
	if (verbose <= log_Verbose)
	{
		printf("%s\n", functionName);
	}
}

void log_Print1(verbose_t verbose, const char* functionName, const char* a)
{
	if (verbose == VERBOSE_ERROR)
	{
		printf("ERROR: ");
	}
	if (verbose == VERBOSE_WARN)
	{
		printf("WARNING: ");
	}
	if (verbose <= log_Verbose)
	{
		printf("%s: %s\n", functionName, a);
	}
}

void log_Print2(verbose_t verbose, const char* functionName, const char* a,
		unsigned b)
{
	if (verbose == VERBOSE_ERROR)
	{
		printf("ERROR: ");
	}
	if (verbose == VERBOSE_WARN)
	{
		printf("WARNING: ");
	}
	if (verbose <= log_Verbose)
	{
		if (log_Numeric == NUMERIC_DEC)
		{
			printf("%s: %s, %d\n", functionName, a, b);
		}
		else
		{
			printf("%s: %s, 0x%x\n", functionName, a, b);
		}
	}
}

void log_Print3(verbose_t verbose, const char* functionName, const char* a,
		unsigned b, unsigned c)
{
	if (verbose == VERBOSE_ERROR)
	{
		printf("ERROR: ");
	}
	if (verbose == VERBOSE_WARN)
	{
		printf("WARNING: ");
	}
	if (verbose <= log_Verbose)
	{
		if (log_Numeric == NUMERIC_DEC)
		{
			printf("%s: %s, %d, %d\n", functionName, a, b, c);
		}
		else
		{
			printf("%s: %s, 0x%x, 0x%x\n", functionName, a, b, c);
		}
	}
}

void log_PrintInt(verbose_t verbose, const char* functionName, unsigned a)
{
	if (verbose <= log_Verbose)
	{
		if (log_Numeric == NUMERIC_DEC)
		{
			printf("%s: %d\n", functionName, a);
		}
		else
		{
			printf("%s: 0x%x\n", functionName, a);
		}
	}
}

void log_PrintInt2(verbose_t verbose, const char* functionName, unsigned a,
		unsigned b)
{
	if (verbose <= log_Verbose)
	{
		if (log_Numeric == NUMERIC_DEC)
		{
			printf("%s: %d, %d\n", functionName, a, b);
		}
		else
		{
			printf("%s: 0x%x, 0x%x\n", functionName, a, b);
		}
	}
}
