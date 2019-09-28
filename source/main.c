#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_video.h>

#include "dvd.h"

static u8 dvdLogo8[72*42];

static inline void decompress()
{
	u16 bit = 0, offset = 0;
	
	for (u8 y = 0; y < 42; y++)
	for (u8 x = 0; x < 72; x++)
	{
		dvdLogo8[y * 72 + x] = (dvdLogo1[offset] >> (bit++ % 8)) & 1;
			
		if (bit % 8 == 0)
		{
			offset++;
			bit = 0;
		}
	}
}

static inline void draw(u16 x, u16 y)
{
	u16 offset = 0;

	for (u8 yPos = y; yPos < y + 42; yPos++)
	for (u8 xPos = x; xPos < x + 36; xPos++)
		((u16 *)VRAM)[yPos * 120 + xPos] = ((u16 *)dvdLogo8)[offset++];
}

int main()
{
	u16 xpos = 0, ypos = 0;
	bool dirUp = false, dirRight = true;

	irqInit();
	irqEnable(IRQ_VBLANK);

	SetMode(MODE_4 | BG2_ON);

	BG_PALETTE[1] = 0x91C9;

	decompress();

	for (;;)
	{	
		xpos += dirRight ?  1 : -1;
		ypos += dirUp    ? -2 :  2;
		
		if (xpos <= 0 || xpos >= 85 ||
			ypos <= 0 || ypos >= 120)
			BG_PALETTE[1] += 0x3039;
		
		if (xpos <= 0)
		{
			xpos     = 0;
			dirRight = true;
		}

		if (ypos <= 0)
		{
			ypos  = 0;
			dirUp = false;
		}

		if (xpos >= 85)
		{
			xpos     = 85;
			dirRight = false;
		}

		if (ypos >= 120)
		{
			ypos  = 120;
			dirUp = true;
		}
		
		*(u32 *)VRAM = 0;
		CpuFastSet((u32*)VRAM, (u32*)VRAM, FILL | COPY32 | (240*160)/sizeof(u32));
			
		draw(xpos, ypos);
		
		VBlankIntrWait();
	}
}
