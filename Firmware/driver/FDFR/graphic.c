#include "stdface.h"

static short	*paper;
static short	linecb;
static short	penclr;

void setPaper(short *yuv, short width)
{
	paper  = yuv;
	linecb = width;
}

void setPenColor(short clr)
{
	penclr = clr;
}

#define	PLOT(x, y)	if (y * linecb + x >= 0) *(paper + y * linecb + x) = penclr
// Bresenham return number of pixel to plot
void Bresenham(int x1, int y1, int x2, int y2)
{
	signed short	ix;
	signed short	iy;
	int				delta_x, delta_y;
	int				error;
 
	// if x1 == x2 or y1 == y2, then it does not matter what we set here
	delta_x = (x2 > x1?(ix = 1, x2 - x1):(ix = -1, x1 - x2)) << 1;
	delta_y = (y2 > y1?(iy = 1, y2 - y1):(iy = -1, y1 - y2)) << 1;
 
    PLOT(x1, y1);
    if (delta_x >= delta_y)
    {
        // error may go below zero
 		error = delta_y - (delta_x >> 1);
        while (x1 != x2)
        {
            if (error >= 0)
            {
                if (error || (ix > 0))
                {
                    y1 += iy;
                    error -= delta_x;
                }
                // else do nothing
            }
            // else do nothing
 
            x1 += ix;
            error += delta_y;
            PLOT(x1, y1);
        }
    }
    else
    {
		// error may go below zero
		error = delta_x - (delta_y >> 1);
 
        while (y1 != y2)
        {
            if (error >= 0)
            {
                if (error || (iy > 0))
                {
                    x1 += ix;
                    error -= delta_y;
                }
                // else do nothing
            }
            // else do nothing
 
            y1 += iy;
            error += delta_x;
            PLOT(x1, y1);
        }
    }
}

// drawRectangle return hiword = number of pixel to plot for a side
//                      loword = number of pixel to plot for another side
// NOTE: The POINT *p should be POINT p[4]. Be careful, there is no any check.
void drawRectangle(POINT* p)
{
	Bresenham(p[0].x, p[0].y, p[1].x, p[1].y);
	Bresenham(p[1].x, p[1].y, p[2].x, p[2].y);
	Bresenham(p[2].x, p[2].y, p[3].x, p[3].y);
	Bresenham(p[3].x, p[3].y, p[0].x, p[0].y);
}
