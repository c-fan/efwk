//
//    Copyright (C) 2017 LGPL
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
//    USA

/*
 *---------------------------------------------------------------------------
 *| VERSION	| AUTHOR		| DATE			| NOTE							|
 *---------------------------------------------------------------------------
 *| 01		| Luo Renan	| 2017-06-28	|								|
 *---------------------------------------------------------------------------
 */


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <fwk/basic/types.h>
#include <fwk/basic/basictrace.h>
#include "basictrace.h"

void fwk_basictrace_getLength (const char *pi1Fmt, va_list VarArgList, uint32_t *pi4ArgCount, uint32_t *pi4VarArgLength)
{
    const char		*pi1FmtString;
    const char		*pi1TmpFmtString;
    const char		*pi1TmpArgString;
    const uint32_t		u4MaxIntLength = 19;
    uint32_t	u4ArgCount = 0;
    char*	pAddr;
    uint32_t	u4VarArgLength 	= 0;

    pi1FmtString = pi1Fmt;
    while (*pi1FmtString)
    {
        if (*pi1FmtString == '%')
        {
            pi1FmtString++;
            if (*pi1FmtString != '%')
            {
                pAddr = (char*)va_arg (VarArgList, fwk_addr_t);
                u4ArgCount++;
                pi1TmpFmtString = pi1FmtString;
                if(*pi1TmpFmtString == '-')
                {
                    ++pi1TmpFmtString;
                }
                while((*pi1TmpFmtString >= '0') &&(*pi1TmpFmtString <= '9'))
                {
                    ++pi1TmpFmtString;
                }
                if((*pi1TmpFmtString == 'd') || (*pi1TmpFmtString == 'x') || (*pi1TmpFmtString == 'X') ||(*pi1TmpFmtString == 'u'))
                {
                    u4VarArgLength += u4MaxIntLength;
                }
                else if(*pi1TmpFmtString == 'c')
                {
                    ++u4VarArgLength;
                }
                else if(*pi1TmpFmtString == 's')
                {
                    if(pAddr != 0)
                    {
                        pi1TmpArgString = pAddr;
                        while(*pi1TmpArgString != '\0')
                        {
                            ++u4VarArgLength;
                            ++pi1TmpArgString;
                        }
                    }
                }
            }
        }
        pi1FmtString++;
    }
    *pi4ArgCount = u4ArgCount;
    *pi4VarArgLength = u4VarArgLength;
    return;
}

void fwk_basictrace_print(const uint32_t u4Mod, const uint32_t u4Level, const char *pi1Fmt, ...)
{
	va_list 	VarArgList;
	va_list 	VarArgListA;
	uint32_t	u4ArgCount;
	uint32_t	u4VarArgLength = 0;
	uint32_t	u4NameLength = 0;
	uint32_t	u4Pos = 0;
	uint32_t	u4FmtLength = 0;
	char		ai1MsgBuf[FWK_BASICTRACE_STR_LEN_MAX];
	uint32_t 	u4Print = 0;
	
	if(gu4FwkBasictaceEnable != 1)
	{
		printf("Trace is disabled.\r\n");
		return;
	}
	
	if (u4Level == FWK_BASICTRACE_LEVEL_ERR)
	{
		sprintf(gai1FwkBasictraceBuffer,"Module[0x%x]Level[0x%x]",u4Mod,u4Level);
		u4Print = 1;
	}
	else if( ((u4Mod) & gu4FwkBasictraceModule) && ((u4Level) & gu4FwkBasictraceLevel))
	{
		sprintf(gai1FwkBasictraceBuffer,"Module[0x%x]Level[0x%x]",u4Mod,u4Level);
		u4Print = 1;
	} 

	if(u4Print == 1)
	{
		va_start (VarArgList, pi1Fmt);
		va_start (VarArgListA, pi1Fmt);

		fwk_basictrace_getLength(pi1Fmt, VarArgList, &u4ArgCount, &u4VarArgLength);

		u4NameLength = strlen(gai1FwkBasictraceBuffer);

		if(u4NameLength >= FWK_BASICTRACE_MODULE_NAME_LEN_MAX)
		{
			printf ("Length of trace name is error[%d].\n", u4NameLength);
			return;
		}
		if (strcmp (gai1FwkBasictraceBuffer, ""))
		{
			u4Pos += sprintf (ai1MsgBuf + u4Pos, "%s: ", gai1FwkBasictraceBuffer);
		}
		u4NameLength = u4Pos;

		u4FmtLength = strlen(pi1Fmt);
		u4VarArgLength += u4NameLength;
		u4VarArgLength += u4FmtLength;

		if(u4VarArgLength >= FWK_BASICTRACE_STR_LEN_MAX)
		{
			printf("Length of arguments is error[%d].\n",u4VarArgLength);
			return;
		}
		vsprintf (ai1MsgBuf + u4Pos, pi1Fmt, VarArgListA);
		printf("%s", ai1MsgBuf);

		va_end (VarArgList);
	}
}

void fwk_basictrace_setModule(const uint32_t u4Mod, const bool_t Enable)
{
	if(Enable == TRUE)
	{
		gu4FwkBasictraceModule |= u4Mod;
	}
	else if (Enable == FALSE)
	{
		gu4FwkBasictraceModule &= (~u4Mod);
	}
	else
	{
		printf("param(u4Mod:0x%x,Enable:%d) are error.\r\n",u4Mod, Enable);
	}
}

void fwk_basictrace_setLevel(const uint32_t u4Level, const bool_t Enable)
{
	if(Enable == TRUE)
	{
		gu4FwkBasictraceLevel |= u4Level;
	}
	else if (Enable == FALSE)
	{
		gu4FwkBasictraceLevel &= (~u4Level);
	}
	else
	{
		printf("param(u4Level:0x%x,Enable:%d) are error.\r\n",u4Level, Enable);
	}
}

void fwk_basictrace_show(void)
{
	printf("Enabled Basic Trace Module are:0x%x\r\n",gu4FwkBasictraceModule);
	printf("Enabled Basic Trace Level are:0x%x\r\n",gu4FwkBasictraceLevel);
}

