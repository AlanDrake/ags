// Alan v.Drake's experimental filters of wonder v0.8
//----------------------------------------------------------
//Copyright (C) 2011 Alan v.Drake

//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 2.1 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//Lesser General Public License for more details.
//
#include <allegro.h>


// adex_scanline_2x :  a simple 2x scanline 
void adex_scanline_2x(BITMAP *src, BITMAP *dest)
{
	int x1,x2,y1,y2, w = src->w, h = src->h;
	for (y1=0,y2=0; y1<h; y1++,y2+=2)
	{
		unsigned long *line1 = (unsigned long *)dest->line[y2];
		unsigned long *line2 = (unsigned long *)dest->line[y2+1];
		for (x1=0; x1<w; x1++)
		{
			int opixel = ((long *)src->line[y1])[x1];
			//int dark_color = ((opixel & 0xFEFEFE) >> 1); // fast 50%
			int r = (opixel >> 16) & 0xFF; 
			int g = (opixel >> 8) & 0xFF;  
			int b = opixel & 0xFF; 
			
			int lightFactor = 70+(((r+g+b)*25)/765);
			
			int dark_color = ( (r*lightFactor/100) << 16 | (g*lightFactor/100) << 8 | (b*lightFactor/100) );
			
			*line1++ = opixel;
			*line1++ = opixel;
			*line2++ = dark_color ;
			*line2++ = dark_color;
		}
	}      
}


// adex_tvcrt_4x: Attempts to simulate a television's CRT pattern. Would look better with some bilinear filtering, heh
void adex_tvcrt_4x(BITMAP *src, BITMAP *dest)
{
	int x1,x2,y1,y2, w = src->w, h = src->h;
	for (y1=0,y2=0; y1<h; y1++,y2+=4)
	{
		unsigned long *line1 = (unsigned long *)dest->line[y2];
		unsigned long *line2 = (unsigned long *)dest->line[y2+1];
		unsigned long *line3 = (unsigned long *)dest->line[y2+2];
		unsigned long *line4 = (unsigned long *)dest->line[y2+3];

		for (x1=0; x1<w; x1++)
		{
			int opixel = ((long *)src->line[y1])[x1];
			int apixel = ((long *)src->line[y1])[x1+1];
			//int dark_color = ((opixel & 0xFEFEFE) >> 1); // fast 50%

			int r = (opixel >> 16) & 0xFF; 
			int g = (opixel >> 8) & 0xFF;  
			int b = opixel & 0xFF; 

			int colorR,colorG,colorB;

			int lightFactor = 50+(((r+g+b)*2)/51);

			colorR = (r << 16 | (g*lightFactor/100) << 8 | (b*lightFactor/100)  );
			colorG = ( (r*lightFactor/100) << 16 | g << 8 | (b*lightFactor/100) );
			colorB = ( (r*lightFactor/100) << 16 | (g*lightFactor/100) << 8 | b );

			int dark_color = ( (r*lightFactor/100) << 16 | (g*lightFactor/100) << 8 | (b*lightFactor/100) );
			lightFactor += 10;
			int int_color = ( (r+((apixel>>16)&0xFF))*lightFactor/200 << 16 | (g+((apixel>>8)&0xFF))*lightFactor/200 << 8 | (b+(apixel&0xFF))*lightFactor/200 );

			if (x1 % 2)
			{
				for(x2=0;x2<4;x2++)
					*line1++ = dark_color;

				*line2++ = colorR;
				*line2++ = colorG;
				*line2++ = colorB;
				*line2++ = int_color;

				*line3++ = colorR;
				*line3++ = colorG;
				*line3++ = colorB;
				*line3++ = int_color;
			}
			else
			{
				*line1++ = colorR;
				*line1++ = colorG;
				*line1++ = colorB;
				*line1++ = int_color;

				*line2++ = colorR;
				*line2++ = colorG;
				*line2++ = colorB;
				*line2++ = int_color;

				for(x2=0;x2<4;x2++)
					*line3++ = dark_color;

			}

			*line4++ = colorR;
			*line4++ = colorG;
			*line4++ = colorB;
			*line4++ = int_color;

		}		
	}      
}

// adex_monitorcrt_4x: Simulates almost faithfully a CRT computer monitor
void adex_monitorcrt_4x(BITMAP *src, BITMAP *dest)
{
	int x1,x2,y1,y2, w = src->w, h = src->h;
	for (y1=0,y2=0; y1<h; y1++,y2+=4)
	{
		unsigned long *line1 = (unsigned long *)dest->line[y2];
		unsigned long *line2 = (unsigned long *)dest->line[y2+1];
		unsigned long *line3 = (unsigned long *)dest->line[y2+2];
		unsigned long *line4 = (unsigned long *)dest->line[y2+3];

		for (x1=0; x1<w; x1++)
		{
			int opixel = ((long *)src->line[y1])[x1];
			int apixel = ((long *)src->line[y1])[x1+1];

			int r = (opixel >> 16) & 0xFF; 
			int g = (opixel >> 8) & 0xFF;  
			int b = opixel & 0xFF; 

			int lightFactor = 20+(((r+g+b)*70)/765);

			int int_color = ( (r+((apixel>>16)&0xFF))/2 << 16 | (g+((apixel>>8)&0xFF))/2 << 8 | (b+(apixel&0xFF))/2 );
			int dark_color = ( (r*lightFactor/100) << 16 | (g*lightFactor/100) << 8 | (b*lightFactor/100) );
			int dark_int_color = ( (r+((apixel>>16)&0xFF))*lightFactor/200 << 16 | (g+((apixel>>8)&0xFF))*lightFactor/200 << 8 | (b+(apixel&0xFF))*lightFactor/200 );

			for (x2=0;x2<3;x2++)
				*line1++ = opixel;
			*line1++ = int_color;

			for (x2=0;x2<3;x2++)
				*line2++ = dark_color ;
			*line2++ = dark_int_color ;

			for (x2=0;x2<3;x2++)
				*line3++ = opixel;
			*line3++ = int_color;

			for (x2=0;x2<3;x2++)
				*line4++ = dark_color;
			*line4++ = dark_int_color;
		}
	}      
}

