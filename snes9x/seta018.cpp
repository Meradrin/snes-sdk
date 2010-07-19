/**********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com) and
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  Brad Jorsch (anomie@users.sourceforge.net),
                             funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com),
                             Nach (n-a-c-h@users.sourceforge.net), and
                             zones (kasumitokoduck@yahoo.com)

  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com)
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti


  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley,
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001-2006    byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight,

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound DSP emulator code is derived from SNEeSe and OpenSPC:
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x filter
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  Specific ports contains the works of other authors. See headers in
  individual files.

  Snes9x homepage: http://www.snes9x.com

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without 
  fee, providing that this license information and copyright notice appear 
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
**********************************************************************************/

#include "seta.h"
#include "memmap.h"

ST018_Regs ST018;

static int line;	// line counter

extern "C"{
uint8 S9xGetST018(uint32 Address)
{
	uint8 t = 0;
	uint16 address = (uint16) Address & 0xFFFF;

	line++;

	// these roles may be flipped
	// op output
	if (address == 0x3804)
	{
		if (ST018.out_count)
		{
			t = (uint8) ST018.output [ST018.out_index];
			ST018.out_index++;
			if (ST018.out_count==ST018.out_index)
				ST018.out_count=0;
		}
		else
			t = 0x81;
	}
	// status register
	else if (address == 0x3800)
		t = ST018.status;
	
	printf( "ST018 R: %06X %02X\n", Address, t);

	return t;
}

void S9xSetST018(uint8 Byte, uint32 Address)
{
	uint16 address = (uint16) Address&0xFFFF;
	static bool reset = false;

	printf( "ST018 W: %06X %02X\n", Address, Byte );

	line++;

	if (!reset)
	{
		// bootup values
		ST018.waiting4command = true;
		ST018.part_command = 0;
		reset = true;
	}

	Memory.SRAM[address]=Byte;

	// default status for now
	ST018.status = 0x00;

	// op data goes through this address
	if (address==0x3804)
	{
		// check for new commands: 3 bytes length
		if(ST018.waiting4command && ST018.part_command==2)
		{
			ST018.waiting4command = false;
			ST018.command <<= 8;
			ST018.command |= Byte;
			ST018.in_index = 0;
			ST018.out_index = 0;
			ST018.part_command = 0;	// 3-byte commands
			ST018.pass = 0;	// data streams into the chip
			switch(ST018.command & 0xFFFFFF)
			{
			case 0x0100: ST018.in_count = 0; break;
			case 0xFF00: ST018.in_count = 0; break;
			default: ST018.waiting4command = true; break;
			}
		}
		else if(ST018.waiting4command)
		{
			// 3-byte commands
			ST018.part_command++;
			ST018.command <<= 8;
			ST018.command |= Byte;
		}
	}
	// extra parameters
	else if (address==0x3802)
	{
		ST018.parameters[ST018.in_index] = Byte;
		ST018.in_index++;
	}

	if (ST018.in_count==ST018.in_index)
	{
		// Actually execute the command
		ST018.waiting4command = true;
		ST018.in_index = 0;
		ST018.out_index = 0;
		switch (ST018.command)
		{
		// hardware check?
		case 0x0100:
			ST018.waiting4command = false;
			ST018.pass++;
			if (ST018.pass==1)
			{
				ST018.in_count = 1;
				ST018.out_count = 2;

				// Overload's research
				ST018.output[0x00] = 0x81;
				ST018.output[0x01] = 0x81;
			}
			else
			{
				//ST018.in_count = 1;
				ST018.out_count = 3;

				// no reason to change this
				//ST018.output[0x00] = 0x81;
				//ST018.output[0x01] = 0x81;
				ST018.output[0x02] = 0x81;

				// done processing requests
				if (ST018.pass==3)
					ST018.waiting4command = true;
			}
			break;

		// unknown: feels like a security detection
		// format identical to 0x0100
		case 0xFF00:
			ST018.waiting4command = false;
			ST018.pass++;
			if (ST018.pass==1)
			{
				ST018.in_count = 1;
				ST018.out_count = 2;

				// Overload's research
				ST018.output[0x00] = 0x81;
				ST018.output[0x01] = 0x81;
			}
			else
			{
				//ST018.in_count = 1;
				ST018.out_count = 3;

				// no reason to change this
				//ST018.output[0x00] = 0x81;
				//ST018.output[0x01] = 0x81;
				ST018.output[0x02] = 0x81;

				// done processing requests
				if (ST018.pass==3)
					ST018.waiting4command = true;
			}
			break;
		}
	}
}
}
