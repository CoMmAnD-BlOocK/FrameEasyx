#pragma once
#include <stdio.h>
#include <iostream>
#include <easyx.h>
#include <WinSock2.h>
#include <Windows.h>
#include <list>
#include <math.h>
#include <wingdi.h>
#include <string>
#include <stdlib.h>
#pragma comment (lib, "MSIMG32.lib")
#pragma warning(disable:4244)
#define PI 3.1415926535897932384
#define absvalue(a) a >= 0 ? a : -a
#define sqar(a) a*a
#pragma warning(disable:4996)
#pragma warning(disable:6031)
#pragma warning(disable:6387)
#pragma warning(disable:26495)

using namespace std;

#define black		0
#define white		0xFFFFFF
#define red			0xFF0000
#define green		0x00FF00
#define blue		0x0000FF

DWORD ChangeCursorStyle(LPCWSTR cursor_style) {
	HCURSOR hCursor = LoadCursor(NULL, cursor_style);
	if (hCursor == NULL) return GetLastError();
	SetCursor(hCursor);
	return 0;
}

struct IDentifier {
	wchar_t name[32];
};

LOGFONT newfont(long height, long width) {
	LOGFONT logfont{};
	logfont.lfHeight = height;
	logfont.lfWidth = width;
	logfont.lfQuality = ANTIALIASED_QUALITY | PROOF_QUALITY;
	return logfont;
}

LOGFONT newfont(long height, long width, long weight) {
	LOGFONT logfont{};
	logfont.lfHeight = height;
	logfont.lfWidth = width;
	logfont.lfWeight = weight;
	logfont.lfQuality = ANTIALIASED_QUALITY | PROOF_QUALITY;
	return logfont;
}

LOGFONT newfont(long height, long width, long weight, const wchar_t facename[]) {
	LOGFONT logfont{};
	logfont.lfHeight = height;
	logfont.lfWidth = width;
	logfont.lfWeight = weight;
	logfont.lfQuality = ANTIALIASED_QUALITY | PROOF_QUALITY;
	memcpy(logfont.lfFaceName, facename, wcslen(facename));
	return logfont;
}

LOGFONT newfont(long height, long width, const wchar_t facename[]) {
	LOGFONT logfont{};
	logfont.lfHeight = height;
	logfont.lfWidth = width;
	logfont.lfQuality = ANTIALIASED_QUALITY | PROOF_QUALITY;
	memcpy(logfont.lfFaceName, facename, wcslen(facename));
	return logfont;
}


LINESTYLE newLineStyle (DWORD style_, DWORD thickness_, DWORD* puserstyle_, DWORD userstylecount_) {
	LINESTYLE ret;
	ret.style = style_;
	ret.thickness = thickness_;
	ret.puserstyle = puserstyle_;
	ret.userstylecount = userstylecount_;
	return ret;
}

IDentifier randomID() {
	IDentifier ret{};
	wchar_t rand_char = 0;
	for (int i = 0; i < 32; i++) {
		while (rand_char > 20) {
			rand_char = (WORD)rand();
		}
		ret.name[i] = rand_char;
	}
	return ret;
}

COORD getLinecenter(COORD a, COORD b) {
	if (a.X == b.X) {
		return COORD{ a.X, (short)(absvalue(a.Y - b.Y) / 2) };
	}
	else if (a.Y == b.Y) {
		return COORD{ a.Y, (short)((a.X - b.X) / 2) };
	}
	else {
		return COORD{ (short)(a.X + (b.X - a.X) / 2), (short)(a.Y + (b.Y - a.Y) / 2) };
	}
}

bool isinArea(RECT area,COORD point) {
	return
	point.X < area.right && point.X > area.left ?
	(point.Y > area.top && point.Y < area.bottom) : false;
}

void transparentimage(IMAGE* dstimg, int x, int y, IMAGE* srcimg)
{
	// 变量初始化
	DWORD* dst = GetImageBuffer(dstimg);
	DWORD* src = GetImageBuffer(srcimg);
	int src_width = srcimg->getwidth();
	int src_height = srcimg->getheight();
	int dst_width = (dstimg == NULL ? getwidth() : dstimg->getwidth());
	int dst_height = (dstimg == NULL ? getheight() : dstimg->getheight());

	// 计算贴图的实际长宽
	int iwidth = (x + src_width > dst_width) ? dst_width - x : src_width;		// 处理超出右边界
	int iheight = (y + src_height > dst_height) ? dst_height - y : src_height;	// 处理超出下边界
	if (x < 0) { src += -x;				iwidth -= -x;	x = 0; }				// 处理超出左边界
	if (y < 0) { src += src_width * -y;	iheight -= -y;	y = 0; }				// 处理超出上边界

	// 修正贴图起始位置
	dst += dst_width * y + x;

	// 实现透明贴图

	for (int iy = 0; iy < iheight; ++iy)
	{
		for (int i = 0; i < iwidth; ++i)

		{
			BYTE sa = (BYTE)((src[i] & 0xff000000) >> 24);//获取阿尔法值
			if (sa != 0)//假如是完全透明就不处理
				if (sa == 255)//假如完全不透明则直接拷贝
					dst[i] = src[i];
				else//真正需要阿尔法混合计算的图像边界才进行混合
					dst[i] = ((((src[i] & 0xff0000) >> 16) + ((dst[i] & 0xff0000) >> 16) * (255 - sa) / 255) << 16) | ((((src[i] & 0xff00) >> 8) + ((dst[i] & 0xff00) >> 8) * (255 - sa) / 255) << 8) | ((src[i] & 0xff) + (dst[i] & 0xff) * (255 - sa) / 255);
		}
		dst += dst_width;
		src += src_width;
	}
}

TCHAR* newtchar(wchar_t* str,size_t size) {
	TCHAR* ret = new TCHAR[size];
	_tcscpy(ret, str);
	return ret;
}

class BaseFrame {
public:
	virtual void print() = 0;
	virtual IDentifier getID() = 0;
	virtual void setalpha(BYTE alpha_) = 0;
	virtual BYTE getalpha() = 0;
	virtual void setshowmode(bool isshow_) = 0;
	virtual COORD& getcoordRef() = 0;
	IDentifier ID{};
	bool isshow = true;
	BYTE alpha = 255;
};

class Picture : BaseFrame {
private:
	IMAGE image;			//存储的图像
	COORD coord;			//坐标
public:
	Picture(SIZE imagesize, BYTE alpha_, COORD coord_, IDentifier ID_, bool transparentbk = true, bool isshow_ = true, LINESTYLE linestyle_ = LINESTYLE()) {
		ID = ID_;
		image = IMAGE(imagesize.cx, imagesize.cy);
		coord = coord_;
		isshow = isshow_;
		alpha = alpha_;
		DWORD* pointer = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&image);
		setlinestyle(&linestyle_);
		SetWorkingImage(buffer_device);
		if (transparentbk) {
			for (int i = 0; i < imagesize.cx * imagesize.cy; i++) {
				pointer[i] = 0x00000000;
			}
		}
		else {
			for (int i = 0; i < imagesize.cx * imagesize.cy; i++) {
				pointer[i] = 0xff000000;
			}
		}
	}

	Picture() {
		ID = randomID();
		image = IMAGE();
		alpha = 0xff;
		coord.X = coord.Y = 0;
		isshow = true;
	}

	IDentifier getID() {
		return ID;
	}

	void loadimage(IMAGE image_) {
		image = image_;
	}

	void setLineStyle(LINESTYLE linestyle_) {
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&image);
		setlinestyle(&linestyle_);
		SetWorkingImage(buffer_device);
	}

	void print() {
		if (isshow) {
			if (alpha == 255) {
				transparentimage(GetWorkingImage(), coord.X, coord.Y, &image);
				return;
			}
			IMAGE buffer = image;
			DWORD* pointer = GetImageBuffer(&buffer);
			memcpy(pointer, GetImageBuffer(&image), image.getwidth() * image.getheight());
			for (int i = 0; i < buffer.getwidth() * buffer.getheight(); i++) {
				reinterpret_cast<BYTE(&)[4]>(pointer[i])[3] *= (DWORD)alpha / (DWORD)255;
			}
			transparentimage(GetWorkingImage(), coord.X, coord.Y, &buffer);
		}
	}

	IMAGE& getimage() {
		IMAGE& ret = image;
		return ret;
	}

	void setshowmode(bool isshow_) {
		isshow = isshow_;
	}

	bool getshowmode() {
		return isshow;
	}

	COORD& getcoordRef() {
		return coord;
	}

	void setalpha(BYTE alpha_) {
		alpha = alpha_;
	}

	BYTE getalpha() {
		return alpha;
	}

	void arc_alpha(int left, int top, int right, int bottom, double stangle, double endangle,COLORREF color,BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setlinecolor(color);
		arc(left, top, right, bottom, stangle, endangle);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void circle_alpha(int x, int y, int radius, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setlinecolor(color);
		circle(x, y, radius);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void ellipse_alpha(int left, int top, int right, int bottom, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setlinecolor(color);
		ellipse(left, top, right, bottom);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void fillcircle_alpha(int x, int y, int radius, COLORREF fillcolor, COLORREF linecolor, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setfillcolor(fillcolor);
		setlinecolor(linecolor);
		fillcircle(x, y, radius);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void fillellipse_alpha(int left, int top, int right, int bottom, COLORREF fillcolor, COLORREF linecolor, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setfillcolor(fillcolor);
		setlinecolor(linecolor);
		ellipse(left, top, right, bottom);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void fillpie_alpha(int left, int top, int right, int bottom, double stangle, double endangle, COLORREF fillcolor, COLORREF linecolor, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setfillcolor(fillcolor);
		setlinecolor(linecolor);
		fillpie(left, top, right, bottom, stangle, endangle);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void fillpolygon_alpha(const POINT* points, int num, COLORREF fillcolor, COLORREF linecolor, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setfillcolor(fillcolor);
		setlinecolor(linecolor);
		fillpolygon(points, num);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void fillrectangle_alpha(int left, int top, int right, int bottom, COLORREF fillcolor, COLORREF linecolor, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setfillcolor(fillcolor);
		setlinecolor(linecolor);
		fillrectangle(left, top, right, bottom);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void fillroundrect_alpha(int left, int top, int right, int bottom, int ellipsewidth, int ellipseheight, COLORREF fillcolor, COLORREF linecolor, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setfillcolor(fillcolor);
		setlinecolor(linecolor);
		fillroundrect(left, top, right, bottom, ellipsewidth, ellipseheight);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void line_alpha(int x1, int y1, int x2, int y2, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setlinecolor(color);
		line(x1, y1, x2, y2);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void pie_alpha(int left, int top, int right, int bottom, double stangle, double endangle, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setlinecolor(color);
		pie(left, top, right, bottom, stangle, endangle);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void polybezier_alpha(const POINT* points, int num, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setlinecolor(color);
		polybezier(points, num);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void polygon_alpha(const POINT* points, int num, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setlinecolor(color);
		polygon(points, num);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void polyline_alpha(const POINT* points, int num, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setlinecolor(color);
		polyline(points, num);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void putpixel_alpha(int x, int y, COLORREF color, BYTE alpha = 255) {
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&image);
		pointer_OD[y * getwidth() + x] = (DWORD)BGR(color) + ((DWORD)alpha << 24);
		SetWorkingImage(buffer_device);
	}

	void rectangle_alpha(int left, int top, int right, int bottom, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setlinecolor(color);
		rectangle(left, top, right, bottom);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void roundrect_alpha(int left, int top, int right, int bottom, int ellipsewidth, int ellipseheight, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setlinecolor(color);
		roundrect(left, top, right, bottom, ellipsewidth, ellipseheight);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void solidcircle_alpha(int x, int y, int radius, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setfillcolor(color);
		solidcircle(x, y, radius);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void solidellipse_alpha(int left, int top, int right, int bottom, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setfillcolor(color);
		solidellipse(left, top, right, bottom);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void solidpie_alpha(int left, int top, int right, int bottom, double stangle, double endangle, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setfillcolor(color);
		solidpie(left, top, right, bottom, stangle, endangle);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void solidpolygon_alpha(const POINT* points, int num, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setfillcolor(color);
		solidpolygon(points, num);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void solidrectangle_alpha(int left, int top, int right, int bottom, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setfillcolor(color);
		solidrectangle(left, top, right, bottom);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void solidroundrect_alpha(int left, int top, int right, int bottom, int ellipsewidth, int ellipseheight, COLORREF color, BYTE alpha = 255) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setfillcolor(color);
		solidroundrect(left, top, right, bottom, ellipsewidth, ellipseheight);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

	void fillimage_alpha(COLORREF color, BYTE alpha = 255) {
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&image);
		setbkcolor(color);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			pointer_OD[i] = (((DWORD)alpha) << 24) + (DWORD)BGR(color);
		}
		SetWorkingImage(buffer_device);
	}

	int drawtext_alpha(LPCTSTR str, RECT* pRect, UINT uFormat, COLORREF textcolor, COLORREF bkcolor, LOGFONT logfont, BYTE alpha = 255, bool bkmode = TRANSPARENT) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setbkmode(bkmode);
		settextcolor(textcolor);
		setbkcolor(bkcolor);
		settextstyle(&logfont);
		int ret = drawtext(str, pRect, uFormat) == 0;
		if (ret == 0) return 0;
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
		return ret;
	}

	void outtextxy_alpha(int x, int y, LPCTSTR str, COLORREF textcolor, COLORREF bkcolor, LOGFONT logfont, BYTE alpha = 255, bool bkmode = TRANSPARENT) {
		IMAGE buffer = IMAGE(image);
		DWORD* pointer = GetImageBuffer(&buffer);
		DWORD* pointer_OD = GetImageBuffer(&image);
		IMAGE* buffer_device = GetWorkingImage();
		SetWorkingImage(&buffer);
		setbkmode(bkmode);
		settextcolor(textcolor);
		setbkcolor(bkcolor);
		settextstyle(&logfont);
		outtextxy(x, y, str);
		for (int i = 0; i < getwidth() * getheight(); i++) {
			if (pointer[i] != pointer_OD[i]) {
				pointer[i] = (((DWORD)alpha) << 24) + (pointer[i] & 0x00ffffff);
			}
		}
		image = buffer;
		SetWorkingImage(buffer_device);
	}

};

class ClickbleFrame : BaseFrame {
public:
	virtual bool isOnClink(ExMessage msg) = 0;
	virtual bool isClinked(ExMessage msg) = 0;
	virtual bool isAbove() = 0;
	virtual bool isRightClicked(ExMessage msg) = 0;
};

class Button :ClickbleFrame {
private:
	IDentifier ID;
	Picture* picture; //存储的图像
	bool canclick; //按钮可用状态
public:
	Button() {
		picture = new Picture;
		canclick = true;
		ID = randomID();
	}

	Button(Picture* picture_, IDentifier ID_, bool canclick_ = true) {
		picture = picture_;
		canclick = canclick_;
		ID = ID_;
	}

	void print(){
		picture->print();
	}

	void setalpha(BYTE alpha_) {
		picture->setalpha(alpha_);
	}

	bool isOnClink(ExMessage msg) {
		if (msg.message == WM_LBUTTONDOWN && canclick) {
			RECT rect{};
			rect.left = picture->getcoordRef().X; rect.right = picture->getcoordRef().X + picture->getimage().getwidth();
			rect.top = picture->getcoordRef().Y; rect.bottom = picture->getcoordRef().Y + picture->getimage().getheight();
			COORD coord{};
			coord.X = msg.x; coord.Y = msg.y;
			return isinArea(rect, coord);
		}
		return false;
	}

	bool isRightOnClink(ExMessage msg) {
		if (msg.message == WM_RBUTTONDOWN && canclick) {
			RECT rect{};
			rect.left = picture->getcoordRef().X; rect.right = picture->getcoordRef().X + picture->getimage().getwidth();
			rect.top = picture->getcoordRef().Y; rect.bottom = picture->getcoordRef().Y + picture->getimage().getheight();
			COORD coord{};
			coord.X = msg.x; coord.Y = msg.y;
			return isinArea(rect, coord);
		}
		return false;
	}

	bool isClinked(ExMessage msg) {
		if (msg.message == WM_LBUTTONDOWN && canclick) {
			RECT rect{};
			rect.left = picture->getcoordRef().X; rect.right = picture->getcoordRef().X + picture->getimage().getwidth();
			rect.top = picture->getcoordRef().Y; rect.bottom = picture->getcoordRef().Y + picture->getimage().getheight();
			COORD coord{};
			coord.X = msg.x; coord.Y = msg.y;
			return isinArea(rect, coord);
		}
		return false;
	}

	bool isRightClicked(ExMessage msg) {
		if (msg.message == WM_RBUTTONDOWN && canclick) {
			RECT rect{};
			rect.left = picture->getcoordRef().X; rect.right = picture->getcoordRef().X + picture->getimage().getwidth();
			rect.top = picture->getcoordRef().Y; rect.bottom = picture->getcoordRef().Y + picture->getimage().getheight();
			COORD coord{};
			coord.X = msg.x; coord.Y = msg.y;
			return isinArea(rect, coord);
		}
		return false;
	}

	bool isAbove() {
		POINT point{};
		GetCursorPos(&point);
		ScreenToClient(GetHWnd(), &point);
		RECT rect{};
		rect.left = picture->getcoordRef().X; rect.right = picture->getcoordRef().X + picture->getimage().getwidth();
		rect.top = picture->getcoordRef().Y; rect.bottom = picture->getcoordRef().Y + picture->getimage().getheight();
		return isinArea(rect, COORD{ (SHORT)point.x, (SHORT)point.y });
	}

	void setPicture(Picture* picture_) {
		picture = picture_;
	}

	Picture& getPictureRef() {
		return *picture;
	}

	Picture* getPicturePtr() {
		return picture;
	}

	COORD& getcoordRef() {
		return picture->getcoordRef();
	}

	IDentifier getID() {
		return ID;
	}

	BYTE getalpha() {
		return picture->getalpha();
	}

	void setshowmode(bool isshow_) {
		picture->setshowmode(isshow_);
	}
};

class CheckBox : ClickbleFrame {
public:
	CheckBox() {
		Picture* buffer = new Picture[8];
		memcpy(pictureList, buffer, sizeof(Picture) * 8);
		delete[] buffer;
		ID = randomID();
		canclick = true;
		auto_change = true;
		alpha = 255;
	}

	CheckBox(IDentifier ID_, bool canclick_ = true, BYTE auto_change_ = left, BYTE alpha_ = 255) {
		ID = ID_;
		canclick = canclick_;
		auto_change = auto_change_;
		alpha = alpha_;
	}

	CheckBox(Picture picureList_[], IDentifier ID_, bool canclick_, BYTE auto_change_ = left, BYTE alpha_ = 255) {
		if (sizeof(pictureList) / sizeof(Picture) > 8) throw "The size of picture list is too big!";
		memcpy(pictureList, picureList_, sizeof(pictureList) / sizeof(Picture));
		ID = ID_;
		canclick = canclick_;
		auto_change = auto_change_;
		alpha = alpha_;
	}

	bool isOnClink(ExMessage msg) {
		if (msg.message == WM_LBUTTONDOWN && canclick) {
			RECT rect{};
			rect.left = pictureList[state].getcoordRef().X; rect.right = pictureList[state].getcoordRef().X + pictureList[state].getimage().getwidth();
			rect.top = pictureList[state].getcoordRef().Y; rect.bottom = pictureList[state].getcoordRef().Y + pictureList[state].getimage().getheight();
			COORD coord{};
			coord.X = msg.x; coord.Y = msg.y;
			if (isinArea(rect, coord)) {
				if (auto_change == left || auto_change == (left | right)) {
					if (state == 1) state = 0;
					else state = 1;
				}
				return true;
			}
		}
		return false;
	}

	bool isRightOnClink(ExMessage msg) {
		if (msg.message == WM_RBUTTONDOWN && canclick) {
			RECT rect{};
			rect.left = pictureList[state].getcoordRef().X; rect.right = pictureList[state].getcoordRef().X + pictureList[state].getimage().getwidth();
			rect.top = pictureList[state].getcoordRef().Y; rect.bottom = pictureList[state].getcoordRef().Y + pictureList[state].getimage().getheight();
			COORD coord{};
			coord.X = msg.x; coord.Y = msg.y;
			if (isinArea(rect, coord)) {
				if (auto_change == left || auto_change == (left | right)) {
					if (state == 1) state = 0;
					else state = 1;
				}
				return true;
			}
		}
		return false;
	}

	bool isClinked(ExMessage msg) {
		if (msg.message == WM_LBUTTONDOWN && canclick) {
			RECT rect{};
			rect.left = pictureList[state].getcoordRef().X; rect.right = pictureList[state].getcoordRef().X + pictureList[state].getimage().getwidth();
			rect.top = pictureList[state].getcoordRef().Y; rect.bottom = pictureList[state].getcoordRef().Y + pictureList[state].getimage().getheight();
			COORD coord{};
			coord.X = msg.x; coord.Y = msg.y;
			return isinArea(rect, coord);
		}
		return false;
	}

	bool isRightClicked(ExMessage msg) {
		if (msg.message == WM_RBUTTONDOWN && canclick) {
			RECT rect{};
			rect.left = pictureList[state].getcoordRef().X; rect.right = pictureList[state].getcoordRef().X + pictureList[state].getimage().getwidth();
			rect.top = pictureList[state].getcoordRef().Y; rect.bottom = pictureList[state].getcoordRef().Y + pictureList[state].getimage().getheight();
			COORD coord{};
			coord.X = msg.x; coord.Y = msg.y;
			return isinArea(rect, coord);
		}
		return false;
	}

	BYTE getstate() {
		return state;
	}

	BYTE setstate(BYTE state_) {
		state = state_;
	}

	bool isAbove() {
		POINT point{};
		GetCursorPos(&point);
		ScreenToClient(GetHWnd(), &point);
		RECT rect{};
		rect.left = pictureList[state].getcoordRef().X; rect.right = pictureList[state].getcoordRef().X + pictureList[state].getimage().getwidth();
		rect.top = pictureList[state].getcoordRef().Y; rect.bottom = pictureList[state].getcoordRef().Y + pictureList[state].getimage().getheight();
		return isinArea(rect, COORD{ (SHORT)point.x, (SHORT)point.y });
	}

	void setalpha(BYTE alpha_) {
		alpha = alpha_;
	}

	void print() {
		pictureList[state].print();
	}

	/// <summary>
	/// 在检测按下左键时自动更改图像
	/// disable - 禁用自动更改
	/// left - 点击左键时更改
	/// right - 点击右键时更改
	/// 用 | 运算符混合模式
	/// </summary>
	enum auto_change_mode {
		disable = 0,
		left = 1,
		right = 3
	};
private:
	/// <summary>
	/// 保存选择框各种状态图像的数组。
	/// pictureList[0]	一般状态
	/// pictureList[1]	选择状态
	/// pictureList[2]	部分选择状态（需要手动更改）
	/// 你仍可将其他内存利用
	/// </summary>
	Picture pictureList[8];
	bool canclick;
	IDentifier ID;
	BYTE alpha;
	BYTE state = 0;
	BYTE auto_change;
};

class PictureList {
private:
	list<Picture> Plist;
	list<Picture>::iterator iterator;
	IDentifier ID;
public:
	PictureList(list<Picture> list_, IDentifier ID_) {
		ID = ID_;
		Plist = list_;
		iterator = Plist.begin();
	}

	PictureList() {
		ID = randomID();
		iterator = Plist.begin();
	};

	void next(int i = 1) {
		for (int i_ = 0; i_ < i; i_++) {
			if (iterator == --Plist.end()) {//如果已经到列表末尾
				iterator = Plist.begin();
				return;
			}
			else {
				iterator++;
			}
		}
	}

	void go_to(int i) {
		iterator = Plist.begin();
		next(i);
	}

	void last(int i = 1) {
		for (int i_ = 0; i_ < i; i_++) {
			if (iterator == Plist.begin()) {//如果已经到列表开头
				iterator = --Plist.end();
				return;
			}
			else {
				iterator--;
			}
		}
	}

	void add_to(Picture picture, int x) {
		list<Picture>::iterator iterator_ = Plist.begin();
		for (int i = 0; i < x; i++, iterator_++);
		Plist.insert(iterator_, picture);
	}

	void to_delete(int x) {
		list<Picture>::iterator iterator_ = Plist.begin();
		for (int i = 0; i < x; i++, iterator_++);
		Plist.erase(iterator_);
	}

	void swap(int x1, int x2) {
		list<Picture>::iterator iterator1, iterator2 = Plist.begin();
		for (int i = 0; i < x1; i++, iterator1++);
		for (int i = 0; i < x2; i++, iterator2++);
		Picture picture1 = *iterator1, picture2 = *iterator2;
		Plist.erase(iterator1);
		Plist.insert(iterator1, picture2);
		Plist.erase(iterator2);
		Plist.insert(iterator2, picture1);
	}

	list<Picture>& GetPicturelist() {
		return Plist;
	}

	list<Picture>::iterator& GetIterator() {
		return iterator;
	}

	Picture picture_now() {
		return *iterator;
	}

	Picture& picture_on_pointing() {
		return *iterator;
	}

	int lenth() {
		int ret = 0;
		for (list<Picture>::iterator iterator_;; iterator_++) {
			ret++;
			if (iterator_ == --Plist.end()) break;
		}
		return ret;
	}

	IDentifier getID() {
		return ID;
	}
};

class Label : BaseFrame {
private:
	wstring text; //标签文字
	LOGFONT font; //文字造型
	COORD coord; //标签的坐标
	BYTE bkmode;
	BYTE alpha;
	COLORREF textcolor; //标签的前景色（文字颜色）
	COLORREF bkcolor; //标签的背景色，当bkmode为TRANSPARENT（1）时，背景色无效
public:
	Label(wstring text_, COORD coord_, IDentifier ID_, LOGFONT font_, COLORREF textcolor_ = BLACK, BYTE alpha_ = 255, BYTE bkmode_ = TRANSPARENT, COLORREF bkcolor_ = WHITE, bool isshow_ = true) {
		text = text_;
		coord = coord_;
		textcolor = textcolor_;
		bkmode = bkmode_;
		bkcolor = bkcolor_;
		isshow = isshow_;
		font = font_;
		alpha = alpha_;
		ID = ID_;
	}

	Label() {
		text = L"";
		coord.X = coord.Y = 0;
		textcolor = BLACK;
		font = LOGFONT{};
		bkmode = TRANSPARENT;
		bkcolor = WHITE;
		isshow = true;
		ID = randomID();
	}
	
	void setshowmode(bool isshow_) {
		isshow = isshow_;
	}

	void settext(wstring text_) {
		text = text_;
	}

	wstring gettext() {
		return text;
	}

	void print() {
		if (alpha == 255) {
			settextcolor(textcolor);
			settextstyle(&font);
			setbkcolor(bkcolor);
			setbkmode(bkmode);
			outtextxy(coord.X, coord.Y, text.c_str());
		}
		else {
			IMAGE* gethdc = new IMAGE();
			IMAGE* buffer_device = GetWorkingImage();
			SIZE* buffer_size = new SIZE;
			SetWorkingImage(gethdc);
			settextstyle(&font);
			SetWorkingImage(buffer_device);
			GetTextExtentPoint32W(GetImageHDC(gethdc), text.c_str(), sizeof(text), buffer_size);
			delete gethdc;
			Picture buffer(*buffer_size, 255, coord, IDentifier{ L"drawLabel" });
			buffer.outtextxy_alpha(0, 0, text.c_str(), textcolor, bkcolor, font, alpha, bkmode);
			buffer.print();
		}
	}

	COORD& getcoordRef() {
		return coord;
	}

	IDentifier getID() {
		return ID;
	}

	void setalpha(BYTE alpha_) {
		alpha = alpha_;
	}

	BYTE getalpha() {
		return alpha;
	}
};

class BaseTextBox : BaseFrame {
public:
	LOGFONT font{};
	COLORREF textcolor = BLACK; //前景色（文字颜色）
	COLORREF bkcolor = WHITE; //背景色
	COORD coord{};
	short width = 0, height = 0;
	BYTE alpha = 255;
	bool isshow = true;
	IDentifier ID{};
	virtual void setfont(LOGFONT font_) = 0;
	BYTE bkmode = TRANSPARENT; //背景模式
};

class TextFrame : BaseTextBox {
private:
	UINT uFormat; //格式化字符串的方法
	wstring text;
public:
	TextFrame(wstring text_, COORD coord, IDentifier ID_, LOGFONT font_, short width_, short height_, UINT uFormat_ = 0, COLORREF textcolor_ = BLACK, BYTE alpha_ = 255, BYTE bkmode_ = TRANSPARENT, COLORREF bkcolor_ = WHITE, int isshow_ = true) {
		ID = ID_;
		coord = coord;
		text = text_;
		uFormat = uFormat_;
		textcolor = textcolor_;
		bkcolor = bkcolor_;
		bkmode = bkmode_;
		isshow = isshow_;
		alpha = alpha_;
		font = font_;
		width = width;
		height = height_;
	}

	TextFrame() {
		coord.X = coord.Y = 0;
		text = L"";
		uFormat = 0;
		textcolor = BLACK;
		bkcolor = WHITE;
		bkmode = TRANSPARENT;
		isshow = true;
		alpha = 255;
		ID = randomID();
		font = LOGFONT{};
		width = height = 0;
	}

	void settext(wstring newtext) {
		text = newtext;
	}

	void print() {
		if (alpha == 255) {
			settextcolor(textcolor);
			setbkcolor(bkcolor);
			setbkmode(bkmode);
			settextstyle(&font);
			RECT buffer_rect = RECT{ coord.X,coord.Y,coord.X + width,coord.Y + height };
			drawtext(text.c_str(), &buffer_rect, uFormat);
		}
		else if (alpha == 0) return;
		else {
			IMAGE* device = GetWorkingImage();
			Picture buffer(SIZE{ width, height }, 255, coord, randomID());
			RECT buffer_rect = RECT{ 0,0,width,height };
			buffer.drawtext_alpha(text.c_str(), &buffer_rect, uFormat, textcolor, bkcolor, font, alpha, bkmode);
			buffer.print();
		}
	}

	void setshowmode(bool isshow_) {
		isshow = isshow_;
	}

	void setfont(LOGFONT font_) {
		font = font_;
	}

	COORD& getcoordRef() {
		return coord;
	}

	IDentifier getID() {
		return ID;
	}

	BYTE getalpha() {
		return alpha;
	}

	void setalpha(BYTE alpha_) {
		alpha = alpha_;
	}

	void alphaadd(BYTE alpha_) {
		alpha += alpha;
	}
};

class FrameList {
private:
	list<void*> Flist;
	IDentifier ID;
public:
	FrameList() {
		ID = randomID();
	};

	FrameList(IDentifier ID_) {
		ID = ID_;
	};

	IDentifier getID() {
		return ID;
	}

	void add_to(void* frame, int x) {
		list<void*>::iterator iterator_ = Flist.begin();
		for (int i = 0; i < x; i++, iterator_++);
		Flist.insert(iterator_, frame);
	}

	void add_to(void* const frame[], int count, int x) {
		for (int i = 0; i < count; i++, x++) {
			add_to(frame[i], x);
		}
	}

	void to_delete(int x) {
		list<void*>::iterator iterator_ = Flist.begin();
		for (int i = 0; i < x; i++, iterator_++);
		Flist.erase(iterator_);
	}

	void to_delete(int x, int count) {
		for (int i = x; i < count; i++) {
			to_delete(i);
		}
	}

	void push(void* frame) {
		Flist.push_front(frame);
	}

	void push(void* frame[], int count) {
		for (int i = 0; i < count; i++) {
			Flist.push_front(frame[i]);
		}
	}

	void* pop() {
		void* frame = Flist.front();
		Flist.pop_front();
		return frame;
	}

	int lenth() {
		int ret = 0;
		for (list<void*>::iterator iterator_;; iterator_++) {
			ret++;
			if (iterator_ == --Flist.end()) break;
		}
		return ret;
	}
	
	void swap(int x1, int x2) {
		list<void*>::iterator iterator1, iterator2 = Flist.begin();
		for (int i = 0; i < x1; i++, iterator1++);
		for (int i = 0; i < x2; i++, iterator2++);
		void* picture1 = *iterator1, * picture2 = *iterator2;
		Flist.erase(iterator1);
		Flist.insert(iterator1, picture2);
		Flist.erase(iterator2);
		Flist.insert(iterator2, picture1);
	}
	
	void* get(int x) {
		list<void*>::iterator iterator_ = Flist.begin();
		for (int i = 0; i < x; i++) {
			if (iterator_ == --Flist.end()) break;
			iterator_++;
		}
		return *iterator_;
	}

	void* getbyID(IDentifier ID) {
		for (list<void*>::iterator iterator_ = Flist.begin();;) {
			iterator_++;
			if (iterator_ == --Flist.end()) return NULL;
			if (typeid(*iterator_) == typeid(BaseFrame*)) {
				if (static_cast<BaseFrame*>(*iterator_)->getID().name == ID.name) return *iterator_;
			}
			else if (typeid(*iterator_) == typeid(PictureList*)) {
				if (static_cast<PictureList*>(*iterator_)->getID().name == ID.name) return *iterator_;
			}
			else if (typeid(*iterator_) == typeid(FrameList*)) {
				if (static_cast<PictureList*>(*iterator_)->getID().name == ID.name) return *iterator_;
			}
		}
	}

	void setalpha(BYTE alpha_) {
		for (list<void*>::iterator iterator_ = Flist.begin();; iterator_++) {
			if (typeid(*iterator_) == typeid(BaseFrame*)) {
				static_cast<BaseFrame*>(*iterator_)->setalpha(alpha_);
			}
			else if (typeid(*iterator_) == typeid(PictureList*)) {
				PictureList* picturelist = static_cast<PictureList*>(*iterator_);
				for (int x = 0; x < picturelist->lenth(); x++) {
					picturelist->next();
					picturelist->picture_on_pointing().setalpha(alpha_);
				}
			}
			else if (typeid(*iterator_) == typeid(FrameList*)) {
				static_cast<FrameList*>(*iterator_)->setalpha(alpha_);
			}
			if (iterator_ == --Flist.end()) break;
		}
	}

	void alphaadd(char16_t alpha_) {
		for (list<void*>::iterator iterator_ = Flist.begin();;) {
			if (iterator_ == Flist.end()) break;
			if (typeid(*iterator_) == typeid(BaseFrame*)) {
				static_cast<BaseFrame*>(*iterator_)->setalpha(static_cast<BaseFrame*>(*iterator_)->getalpha() + alpha_);
			}
			else if (typeid(*iterator_) == typeid(PictureList*)) {
				PictureList* picturelist = static_cast<PictureList*>(*iterator_);
				for (int x = 0; x < picturelist->lenth(); x++) {
					picturelist->next();
					picturelist->picture_on_pointing().setalpha(picturelist->picture_on_pointing().getalpha() + alpha_);
				}
			}
			else if (typeid(*iterator_) == typeid(FrameList*)) {
				static_cast<FrameList*>(*iterator_)->alphaadd(alpha_);
			}
			iterator_++;
		}
	}

	int where(void* p) {
		list<void*>::iterator iterator_ = Flist.begin();
		char x = 0;
		while (true) {
			if (*iterator_ == p) return x;
			if (iterator_ == --Flist.end()) return -1;
			x++;
		}
	}

	int wherebyID(IDentifier ID) {
		return where(getbyID(ID));
	}

	void all_hide() { //把控件全部隐藏
		for (list<void*>::iterator iterator_ = Flist.begin();;) {
		if (iterator_ == Flist.end()) break;
			else if (typeid(*iterator_) == typeid(BaseFrame*)) {
				static_cast<BaseFrame*>(*iterator_)->setshowmode(false);
			}
			else if (typeid(*iterator_) == typeid(PictureList*)) {
			PictureList* picturelist = static_cast<PictureList*>(*iterator_);
			for (int x = 0; x < picturelist->lenth(); x++) {
				picturelist->next();
				picturelist->picture_on_pointing().setshowmode(false);
			}
			}
			else if (typeid(*iterator_) == typeid(FrameList*)) {
				static_cast<FrameList*>(*iterator_)->all_hide();
			}
			iterator_++;
		}
	}

	void all_show() { //把控件全部显示
		for (list<void*>::iterator iterator_ = Flist.begin();;) {
			if (iterator_ == Flist.end()) break;
			if (typeid(*iterator_) == typeid(BaseFrame*)) {
				static_cast<BaseFrame*>(*iterator_)->setshowmode(true);
			}
			else if (typeid(*iterator_) == typeid(PictureList*)) {
				PictureList* picturelist = static_cast<PictureList*>(*iterator_);
				for (int x = 0; x < picturelist->lenth(); x++) {
					picturelist->next();
					picturelist->picture_on_pointing().setshowmode(true);
				}
			}
			else if (typeid(*iterator_) == typeid(FrameList*)) {
				static_cast<FrameList*>(*iterator_)->all_show();
			}
			iterator_++;
		}
	}
	
	void setshowmode(int x, bool showmode) {
		if (typeid(get(x)) == typeid(BaseFrame)) {
			static_cast<BaseFrame*>(get(x))->setshowmode(showmode);
		}
		else if (typeid(get(x)) == typeid(PictureList*)) {
			PictureList* picturelist = static_cast<PictureList*>(get(x));
			for (int x = 0; x < picturelist->lenth(); x++) {
				picturelist->next();
				picturelist->picture_on_pointing().setshowmode(showmode);
			}
		}
		else if (typeid(get(x)) == typeid(FrameList*)) {
			if (showmode == true) static_cast<FrameList*>(get(x))->all_show();
			else static_cast<FrameList*>(get(x))->all_hide();
		}
	}

	void print() {
		for (list<void*>::iterator iterator_ = Flist.begin();;) {
			if (iterator_ == Flist.end()) break;
			else if (typeid(*iterator_) == typeid(BaseFrame*)) {
				static_cast<BaseFrame*>(*iterator_)->print();
			}
			else if (typeid(*iterator_) == typeid(PictureList*)) {
				PictureList* picturelist = static_cast<PictureList*>(*iterator_);
				for (int x = 0; x < picturelist->lenth(); x++) {
					picturelist->next();
					picturelist->picture_on_pointing().print();
				}
			}
			else if (typeid(*iterator_) == typeid(FrameList*)) {
				static_cast<FrameList*>(*iterator_)->all_hide();
			}
			iterator_++;
		}
	}

	void setshowmodeByID(IDentifier ID, bool showmode) {
		void* frame = getbyID(ID);
		if (typeid(frame) == typeid(BaseFrame*)) {
			static_cast<BaseFrame*>(frame)->setshowmode(showmode);
		}
		else if (typeid(frame) == typeid(PictureList*)) {
			PictureList* picturelist = static_cast<PictureList*>(frame);
			for (int x = 0; x < picturelist->lenth(); x++) {
				picturelist->next();
				picturelist->picture_on_pointing().setshowmode(showmode);
			}
		}
		else if (typeid(frame) == typeid(FrameList*)) {
			if (showmode == true) static_cast<FrameList*>(frame)->all_show();
			else static_cast<FrameList*>(frame)->all_hide();
		}
	}

	void X_add(int x) {
		for (list<void*>::iterator iterator_ = Flist.begin();; iterator_++) {
			if (typeid(*iterator_) == typeid(BaseFrame*)) {
				static_cast<BaseFrame*>(*iterator_)->getcoordRef().X += x;
			}
			else if (typeid(*iterator_) == typeid(PictureList*)) {
				PictureList* picturelist = static_cast<PictureList*>(*iterator_);
				for (int i = 0; i < picturelist->lenth(); i++) {
					picturelist->next();
					picturelist->picture_on_pointing().getcoordRef().X += x;
				}
			}
			else if (typeid(*iterator_) == typeid(FrameList*)) {
				static_cast<FrameList*>(*iterator_)->X_add(x);
			}
			if (iterator_ == --Flist.end()) break;
		}
	}
	
	void Y_add(int y) {
		for (list<void*>::iterator iterator_ = Flist.begin();; iterator_++) {
			if (typeid(*iterator_) == typeid(BaseFrame*)) {
				static_cast<BaseFrame*>(*iterator_)->getcoordRef().Y += y;
			}
			else if (typeid(*iterator_) == typeid(PictureList*)) {
				PictureList* picturelist = static_cast<PictureList*>(*iterator_);
				for (int i = 0; i < picturelist->lenth(); i++) {
					picturelist->next();
					picturelist->picture_on_pointing().getcoordRef().Y += y;
				}
			}
			else if (typeid(*iterator_) == typeid(FrameList*)) {
				static_cast<FrameList*>(*iterator_)->Y_add(y);
			}
			if (iterator_ == --Flist.end()) break;
		}
	}
};

enum drawset_default {		//供BoxStyle使用的枚举，在复合选项时使用 | 运算符
	no_border_no_fill = 0,
	has_fill = 1,
	has_border = 2,
	has_border_style = 6    //包含has_border
};

struct BoxStyle {
	LINESTYLE border_style;	//边框的线的造型
	COLORREF border_color;	//边框颜色
	COLORREF fill_color;	//内部填充的颜色
	/* drawset掩码图
	*  7:3                  保留(如果不是0则报错)
	*  2                    是否有边框造型(需要有边框才能生效)
	*  1                    是否有边框
	*  0                    是否有填充
	* ===================================================================
	* |7 6 5 4 3 2 1 0													|
	* |. . . . . . . .      no border no fill no border style.			|
	* |. . . . . . . x      has fill but no border and border style.	|
	* |. . . . . . x .      has border but no fill and border style.	|
	* |. . . . . . x x      has border and fill but no border style.	|
	* |. . . . . x . .      no border no fill no border style.			|
	* |. . . . . x x .      no fill but has border and border style.	|
	* |. . . . . x x x      has border,fill and border style.			|
	* ===================================================================
	*/
	BYTE drawset;			//边框和填充设置，参阅drawset_default的声明
};

class EditBox : BaseTextBox {
private:
	wstring text;						//文本
	unsigned long choosetxt_index[2];	//选择的文本一头一尾的下标
	BoxStyle style;						//方框样式
	bool is_cursor_show;				//光标是否显示
	bool is_focus;						//是否在焦点上
	unsigned long cursor_pointing;		//光标在字符串中指示的位置
	COLORREF cursor_color;				//光标颜色
	COLORREF choose_color;				//选择文字的颜色
	POINT print_offset;					//打印文字时的偏离
	list<RECT> choose_area;				//选择文本时的高亮区域
	bool caninput;						//能否输入字符(可打印字符)
	bool oneLine;						//是否为单行文本

	/// <summary>
	/// (该函数仅供内部使用。)
	/// 将鼠标的光标位置转换成输入框的光标位置。
	/// </summary>
	/// <param name="cursor">鼠标的光标位置</param>
	/// <returns>适配于本类的输入框的光标位置</returns>
	unsigned long MouseToCursor(POINT cursor) {
		if (text.length() == 0) return 0;
		if (oneLine) {
			IMAGE* buf_img = new IMAGE;
			IMAGE* buf_device = GetWorkingImage();
			LPINT buf_maxsize = new int;
			LPSIZE buf_size = new SIZE;
			SetWorkingImage(buf_img);
			settextstyle(&font);
			SetWorkingImage(buf_device);
			GetTextExtentExPointW(buf_img->GetHDC(), text.c_str(), text.length() + 1, cursor.x - print_offset.x - coord.X,
				buf_maxsize, NULL, buf_size);
			delete buf_img;
			delete buf_size;
			unsigned long ret = *buf_maxsize;
			delete buf_maxsize;
			if (ret > text.length()) return ret - 1;//避免引发溢出
			return ret;
		}
		unsigned long text_row = (cursor.y - coord.Y - print_offset.y) / font.lfHeight;//鼠标点击的文本的所在行
		wstring* buf_string = new wstring;
		unsigned long* buf_newline = new unsigned long;
		unsigned long* i = new unsigned long;
		unsigned long sizeofCharToCursorAsRow = 0;
		*buf_newline = *i = 0;
		while (*buf_newline < text_row) {
			if ((*buf_newline) > text_row) {
				delete buf_newline;
				break;
			}
			if (text[*i] == '\n') {
				if ((*buf_newline) < text_row) buf_string->clear();
				(*buf_newline)++;
				continue;
			}
			(*buf_string) += text[*i];
			sizeofCharToCursorAsRow++;
			(*i)++;
		}
		delete buf_newline;
		delete i;
		IMAGE* buf_img = new IMAGE;
		IMAGE* buf_device = GetWorkingImage();
		LPINT buf_maxsize = new int;
		LPSIZE buf_size = new SIZE;
		SetWorkingImage(buf_img);
		settextstyle(&font);
		SetWorkingImage(buf_device);
		delete buf_device;
		GetTextExtentExPointW(buf_img->GetHDC(), buf_string->c_str(), buf_string->length() + 1, cursor.x - print_offset.x - coord.X,
			buf_maxsize, NULL, buf_size);
		delete buf_string;
		delete buf_img;
		delete buf_size;
		unsigned long ret = *buf_maxsize + sizeofCharToCursorAsRow;
		delete buf_maxsize;
		return ret;
	}

	/// <summary>
	/// (该函数仅供内部使用。)
	/// 将指定位置的字符转换成字符所占的矩形空间。
	/// </summary>
	/// <param name="cursor">指定的位置（该字符在输入框内的文字的下标）。</param>
	/// <returns>字符所占的矩形空间。若输入框内没有文字，或光标指向的字符为换行符，函数将返回RECT{ 0,0,0,0 }。</returns>
	RECT PointingCharToRect(unsigned long cursor) {
		if ((text.length() == 0)||(text[cursor] == L'\n')) return RECT{0,0,0,0};
		IMAGE* buf_img = new IMAGE;
		IMAGE* buf_device = GetWorkingImage();
		SetWorkingImage(buf_img);
		settextstyle(&font);
		SetWorkingImage(buf_device);
		delete buf_device;
		wstring* to_cursor = new wstring;//光标所在行开头到光标的字符串
		unsigned long line_to_cursor = 0;//到光标所在行的行数
		for (; line_to_cursor < cursor; line_to_cursor++) {
			if (text[line_to_cursor] == L'\n') {
				to_cursor->clear();
				continue;
			}
			(*to_cursor) += text[line_to_cursor];
		}
		SIZE buf_size{};
		SIZE buf_cursor_char_size{};
		GetTextExtentPoint32W(buf_img->GetHDC(), to_cursor->c_str(), to_cursor->length(), &buf_size);
		GetTextExtentPoint32W(buf_img->GetHDC(), &text[cursor], 1, &buf_cursor_char_size);
		return RECT{ coord.X + print_offset.x + buf_size.cx,
			coord.Y + print_offset.y + font.lfHeight * (long)line_to_cursor,
			coord.X + print_offset.x + buf_size.cx + buf_cursor_char_size.cx,
			coord.Y + print_offset.y + font.lfHeight * (long)line_to_cursor + buf_cursor_char_size.cy };
	}
public:
	EditBox(COORD coord_, IDentifier ID_, LOGFONT font_, BoxStyle style_, short width_, short height_, bool caninput_ = true, COLORREF choose_color_ = BLUE, COLORREF cursor_color_ = RGB(127, 127, 127), POINT text_offset_ = POINT{ 5,5 }, bool oneLine_ = true, COLORREF textcolor_ = BLACK, BYTE alpha_ = 255, wstring text_ = L"", int isshow_ = true) {
		text = text_;
		text = L"";
		coord = coord_;
		ID = ID_;
		font = font_;
		textcolor = textcolor_;
		choose_color = choose_color_;
		alpha = alpha_;
		bkmode = TRANSPARENT;
		bkcolor = WHITE;
		isshow = isshow_;
		style = style_;
		width = width_;
		height = height_;
		cursor_color = cursor_color_;
		print_offset = text_offset_;
		oneLine = oneLine_;
		caninput = caninput_;
	}

	EditBox() {
		text = L"";
		coord = COORD{};
		ID = randomID();
		font = LOGFONT{};
		textcolor = BLACK;
		alpha = 255;
		bkmode = TRANSPARENT;
		bkcolor = WHITE;
		isshow = true;
		choose_color = BLUE;
		style = BoxStyle{ newLineStyle(PS_SOLID,1,0,0),BLACK,WHITE,has_border | has_border_style | has_fill };
		width = 0;
		height = 0;
		cursor_color = RGB(127, 127, 127);
		print_offset = POINT{ 5,5 };
		oneLine = true;
		caninput = true;
	}
	void setfont(LOGFONT font_) {
		font = font_;
	}

	BYTE getalpha() {
		return alpha;
	}

	COORD& getcoordRef() {
		return coord;
	}

	IDentifier getID() {
		return ID;
	}

	void setshowmode(bool isshow_) {
		isshow = isshow_;
	}

	void setalpha(BYTE alpha_) {
		alpha = alpha;
	}

	void print() {
		if (isshow == false) return;
		setbkmode(TRANSPARENT);
		Picture buffer = Picture(SIZE{ width, height }, alpha, coord, randomID());
		setfillcolor(style.fill_color);
		if (style.drawset == 1 || style.drawset == 5) solidrectangle(coord.X, coord.Y, coord.X + width, coord.Y + height);
		else if (style.drawset == 2)
			buffer.rectangle_alpha(coord.X, coord.Y, coord.X + width, coord.Y + height, style.border_color);
		else if (style.drawset == 3)
			buffer.fillrectangle_alpha(coord.X, coord.Y, coord.X + width, coord.Y + height, style.fill_color, style.border_color);
		else if (style.drawset == 6) {
			setlinestyle(&style.border_style);
			solidrectangle(coord.X, coord.Y, coord.X + width, coord.Y + height);
		}
		else if (style.drawset == 7) {
			setlinestyle(&style.border_style);
			fillrectangle(coord.X, coord.Y, coord.X + width, coord.Y + height);
		}
		else if (style.drawset == 0 || style.drawset == 4);
		else throw "The drawset is wrong!";

		settextstyle(&font);
		RECT* buffer_rect = new RECT;
		*buffer_rect = RECT{ coord.X + print_offset.x,
			coord.Y + print_offset.y,
			coord.X + print_offset.x + width,
			coord.Y + print_offset.y + height };
		drawtext(text.c_str(), buffer_rect, 0);

		if (choosetxt_index[1] != choosetxt_index[0]) {
			setfillcolor(choose_color);
			for (list<RECT>::iterator i = choose_area.begin(); i != choose_area.end(); i++) {
				solidrectangle((*i).left, (*i).top, (*i).right, (*i).bottom);
			}
		}

		if (is_cursor_show && is_focus) {
			setlinecolor(cursor_color);
			setlinestyle(PS_SOLID, 2);
			if (text.size() == 0) line(coord.X, coord.Y, coord.X, coord.Y + font.lfHeight);
			else {
				//获得光标位置
				IMAGE* buf_img = new IMAGE;
				IMAGE* buf_device = GetWorkingImage();
				SetWorkingImage(buf_img);
				settextstyle(&font);
				SetWorkingImage(buf_device);
				unsigned long vertical_offset = 0;
				string* buffer_s = new string;
				if (oneLine != 0) {
					for (unsigned long i = 0; i <= cursor_pointing; i++) {
						if (text.c_str()[i] == '\n') {
							vertical_offset++;
							buffer_s->clear();
							continue;
						}
						(*buffer_s) += text.c_str()[i];
					}
				}
				SIZE text_size{};
				if (buffer_s->length() != 0) {
					wchar_t* buffer_text = new wchar_t[buffer_s->size() + 1];
					size_t text_lenth = mbstowcs(buffer_text, buffer_s->c_str(), buffer_s->size() + 1);
					GetTextExtentPoint32A(buf_img->GetHDC(), buffer_s->c_str(), text_lenth, &text_size);
					delete buf_img;
					delete[] buffer_text;
				}
				delete buffer_s;
				//打印光标
				line(coord.X + text_size.cx + print_offset.x,
					coord.Y + vertical_offset * text_size.cy + print_offset.y,
					coord.X + text_size.cx + print_offset.x,
					coord.Y + text_size.cy + print_offset.y);
			}
		}
		buffer.print();
	}

	/// <summary>
	/// 请求焦点，如果点击输入框则获取焦点。该函数同样可用于刷新焦点(如果点击其他地方则失去焦点)
	/// </summary>
	/// <param name="msg">消息，用于判断是否点击</param>
	/// <returns>如果点击了输入框(获得了焦点)，返回true，否则返回false</returns>
	bool requestFocus(ExMessage msg) {
		if (msg.message == WM_LBUTTONDOWN && caninput) {
			choosetxt_index[0] = choosetxt_index[1] = 0;
			RECT rect{};
			rect.left = coord.X; rect.right = coord.X + width;
			rect.top = coord.Y; rect.bottom = coord.Y + height;
			COORD coord{};
			coord.X = msg.x; coord.Y = msg.y;
			if (isinArea(rect, coord)) {
				is_focus = true;
				//重新设置光标位置
				cursor_pointing = MouseToCursor(POINT{ msg.x,msg.y });
				return true;
			}
		}
		return false;
	}

	void setFocus(bool is_Focus_) {
		is_focus = is_Focus_;
	}

	/// <summary>
	/// 接收指定消息中的字符，即输入字符。
	/// </summary>
	/// <param name="msg">指定的消息</param>
	/// <returns>是否有字符输入，如果有字符输入返回true，如果没有字符输入或没有焦点默认返回false</returns>
	bool input(ExMessage msg) {
		if (is_focus == false) return false;//如果没有焦点则不接受输入
		if (msg.message == WM_KEYDOWN) {
			//向左键
			if (msg.vkcode == VK_LEFT) {
				if (choosetxt_index[1] - choosetxt_index[0] != 0) {
					choosetxt_index[0] = choosetxt_index[1] = 0;
					choose_area.clear();
				}
				if (cursor_pointing != 0) cursor_pointing--;
				if (text[cursor_pointing] == '\n') cursor_pointing--;
			}
			//向右键
			else if (msg.vkcode == VK_RIGHT) {
				if (cursor_pointing < text.length()) cursor_pointing++;
				if (text[cursor_pointing] == '\n') cursor_pointing++;
			}
			//向上键
			else if (msg.vkcode == VK_UP) {
				choosetxt_index[0] = choosetxt_index[1] = 0;
				choose_area.clear();
				if ((oneLine == false) && (text.length()) != 0) {
					unsigned int cursor_offset = 0;		//这一行文本从开头到光标所在地方的字符长度
					unsigned int cursor_to_end = 0;		//这一行文本从光标所在地方到结尾的字符长度
					unsigned int lastline_length = 0;	//上一行的字符长度
					for (unsigned long i = 0; i < cursor_pointing; i++) {
						if (text[i] == '\n') {
							cursor_offset = 0;
							continue;
						}
						cursor_offset++;
					}
					while (text[cursor_pointing + (unsigned long)cursor_to_end] != '\n') cursor_to_end++;
					while (text[cursor_pointing - (unsigned long)cursor_to_end - (unsigned long)lastline_length] != '\n') lastline_length++;
					if (cursor_to_end > lastline_length) cursor_pointing -= (cursor_offset + 1);
					else cursor_pointing -= (lastline_length + 1);
				}
			}
			//向下键
			else if (msg.vkcode == VK_DOWN) {
				choosetxt_index[0] = choosetxt_index[1] = 0;
				choose_area.clear();
				if (oneLine == false && (text.length()) != 0) {
					unsigned int cursor_offset = 0;		//这一行文本从开头到光标所在地方的字符长度
					unsigned int cursor_to_end = 0;		//这一行文本从光标所在地方到结尾的字符长度
					unsigned int nextline_length = 0;	//下一行的字符长度
					for (unsigned long i = 0; i < cursor_pointing; i++) {
						if (text[i] == '\n') {
							cursor_offset = 0;
							continue;
						}
						cursor_offset++;
					}
					while (text[cursor_pointing + cursor_to_end] != '\n') cursor_to_end++;
					while (text[cursor_pointing + cursor_to_end + nextline_length] != '\n') nextline_length++;
					if (cursor_offset > nextline_length) cursor_pointing += (cursor_to_end + nextline_length + 1);
					else cursor_pointing += (cursor_to_end + cursor_offset + 1);
				}
			}
			//backspace键
			else if (msg.vkcode == VK_BACK) {
				if (text.length() == 0) return true;
				if (choosetxt_index[0] != choosetxt_index[1]) {
					wstring::iterator i_begin = text.begin();
					for (unsigned long i = 0; i < choosetxt_index[0]; i++) i_begin++;
					wstring::iterator i_end = i_begin;
					for (unsigned long i = 0; i < choosetxt_index[1] - choosetxt_index[0]; i++) i_begin++;
					text.erase(i_begin, i_end);
					cursor_pointing = choosetxt_index[0];
					choosetxt_index[1] = choosetxt_index[0];
					choose_area.clear();
					return true;
				}
				text.erase(cursor_pointing);
				cursor_pointing--;
			}
			//delete键
			else if (msg.vkcode == VK_DELETE) {
				if (text.length() == 0) return true;
				if (choosetxt_index[0] != choosetxt_index[1]) {
					wstring::iterator i_begin = text.begin();
					for (unsigned long i = 0; i < choosetxt_index[0]; i++) i_begin++;
					wstring::iterator i_end = i_begin;
					for (unsigned long i = 0; i < choosetxt_index[1] - choosetxt_index[0]; i++) i_begin++;
					text.erase(i_begin, i_end);
					cursor_pointing = choosetxt_index[0];
					choosetxt_index[1] = choosetxt_index[0];
					choose_area.clear();
					return true;
				}
				text.erase(cursor_pointing + 1);
			}
			//ctrl+C
			else if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) && (GetAsyncKeyState(0x43) & 0x8000)) {
				if (choosetxt_index[0] == choosetxt_index[1]) return false;
				wstring TempBin = text.substr(choosetxt_index[0], choosetxt_index[1] - choosetxt_index[0]);
				HGLOBAL hMemBin = NULL;
				PCHAR   LockBin = NULL;
				OpenClipboard(NULL);
				EmptyClipboard();
				hMemBin = GlobalAlloc(GMEM_MOVEABLE, TempBin.size() + 1);
				LockBin = (PCHAR)GlobalLock(hMemBin);
				PCHAR TempBinC = new char[TempBin.size() + 1];
				wcstombs(TempBinC, TempBin.c_str(), TempBin.size() + 1);
				RtlMoveMemory(LockBin, TempBinC, TempBin.size() + 1);
				delete[] TempBinC;
				GlobalUnlock(hMemBin);
				LockBin = NULL;
				SetClipboardData(CF_TEXT, hMemBin);
				CloseClipboard();
				return false;
			}
			//ctrl+X
			else if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) && (GetAsyncKeyState(0x58) & 0x8000)) {
				if (choosetxt_index[0] == choosetxt_index[1]) return false;
				wstring TempBin = text.substr(choosetxt_index[0], choosetxt_index[1] - choosetxt_index[0]);
				HGLOBAL hMemBin = NULL;
				PCHAR   LockBin = NULL;
				OpenClipboard(NULL);
				EmptyClipboard();
				hMemBin = GlobalAlloc(GMEM_MOVEABLE, TempBin.size() + 1);
				LockBin = (PCHAR)GlobalLock(hMemBin);
				PCHAR TempBinC = new char[TempBin.size() + 1];
				wcstombs(TempBinC, TempBin.c_str(), TempBin.size() + 1);
				RtlMoveMemory(LockBin, TempBinC, TempBin.size() + 1);
				delete[] TempBinC;
				GlobalUnlock(hMemBin);
				LockBin = NULL;
				SetClipboardData(CF_TEXT, hMemBin);
				CloseClipboard();
				wstring::iterator i_begin = text.begin();
				for (unsigned long i = 0; i < choosetxt_index[0]; i++) i_begin++;
				wstring::iterator i_end = i_begin;
				for (unsigned long i = 0; i < choosetxt_index[1] - choosetxt_index[0]; i++) i_begin++;
				text.erase(i_begin, i_end);
				cursor_pointing = choosetxt_index[0];
				choosetxt_index[1] = choosetxt_index[0];
				choose_area.clear();
				return false;
			}
			//ctrl+V
			else if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) && (GetAsyncKeyState(0x56) & 0x8000)) {
				if (choosetxt_index[0] == choosetxt_index[1]) return false;
				HGLOBAL hMemBin = NULL;
				PCHAR   LockBin = NULL;
				PCHAR   TempBin = NULL;
				OpenClipboard(NULL);
				hMemBin = GetClipboardData(CF_TEXT);
				LockBin = (PCHAR)GlobalLock(hMemBin);
				TempBin = (PCHAR)malloc(GlobalSize(hMemBin));
				if (TempBin != NULL)
				{
					RtlMoveMemory(TempBin, LockBin, GlobalSize(hMemBin));
				}
				GlobalUnlock(hMemBin);
				LockBin = NULL;
				CloseClipboard();

				string* TempBinS = new string;
				(*TempBinS) = TempBin;
				PWCHAR TempBinW = new wchar_t[TempBinS->size() + 1];
				mbstowcs(TempBinW, TempBinS->c_str(), TempBinS->size() + 1);
				delete TempBinS;
				wstring::iterator i_begin = text.begin();
				for (unsigned long i = 0; i < choosetxt_index[0]; i++) i_begin++;
				wstring::iterator i_end = i_begin;
				for (unsigned long i = 0; i < choosetxt_index[1] - choosetxt_index[0]; i++) i_begin++;
				text.replace(i_begin, i_end, TempBinW);
				delete[] TempBinW;
				cursor_pointing = choosetxt_index[0];
				choosetxt_index[1] = choosetxt_index[0];
				choose_area.clear();
				free(TempBin);
				choose_area.clear();
				cursor_pointing = choosetxt_index[0];
				choosetxt_index[1] = choosetxt_index[0];
				return false;
			}
		}
		else if (msg.message == WM_CHAR) {
			if ((wchar_t)msg.ch == L'\b') return false;
			if ((wchar_t)msg.ch == L'\n') {
				if (oneLine == true) {
					return false;
				}
			}
			if (choosetxt_index[0] != choosetxt_index[1]) {
				wstring::iterator i_begin = text.begin();
				for (unsigned long i = 0; i < choosetxt_index[0]; i++) i_begin++;
				wstring::iterator i_end = i_begin;
				for (unsigned long i = 0; i < choosetxt_index[1] - choosetxt_index[0]; i++) i_begin++;
				text.replace(i_begin, i_end, 1, (wchar_t)msg.ch);
				cursor_pointing = choosetxt_index[0];
				choosetxt_index[1] = choosetxt_index[0];
				choose_area.clear();
				return true;
			}
			text.insert(cursor_pointing, 1, (wchar_t)msg.ch);
			cursor_pointing++;
		}
		else return false;
		return true;
	}

	void chooseText(ExMessage msg) {
		static bool isforward;//是否向前选择(<-这个方向)
		static ExMessage lastmsg;
		if (msg.message == WM_LBUTTONDOWN) {
			if (lastmsg.message == WM_LBUTTONUP) {
				choosetxt_index[0] = choosetxt_index[1] = 0;
				choose_area.clear();
			}
			if (isinArea(RECT{ coord.X + print_offset.x,coord.Y + print_offset.y,
				coord.X + print_offset.x + width,coord.X + print_offset.x + height },
				COORD{ msg.x,msg.y })) {
				unsigned long cursor_pointing_buffer = MouseToCursor(POINT{ msg.x,msg.y });
				if (isforward) {//如果是向前选择
					if (cursor_pointing_buffer < choosetxt_index[0]) {
						for (unsigned long i = 0; i < choosetxt_index[0] - cursor_pointing; i++) {
							choose_area.push_front(PointingCharToRect(choosetxt_index[0] - i));
						}
						choosetxt_index[0] = cursor_pointing;
					}
					else if ((cursor_pointing_buffer > choosetxt_index[0]) || cursor_pointing_buffer < choosetxt_index[1]) {
						isforward = false;
						for (unsigned long i = 0; i < cursor_pointing - choosetxt_index[0]; i++) {
							choose_area.pop_front();
						}
						choosetxt_index[0] = cursor_pointing;
					}
					else if (cursor_pointing_buffer > choosetxt_index[1]) {
						isforward = false;
						choose_area.clear();
						choosetxt_index[0] = choosetxt_index[1];
						for (unsigned long i = 0; i < cursor_pointing - choosetxt_index[0]; i++) {
							choose_area.push_back(PointingCharToRect(choosetxt_index[0] + i));
						}
						choosetxt_index[1] = cursor_pointing_buffer;
					}
					else if (cursor_pointing_buffer == choosetxt_index[0]) {
						isforward = false;
						choose_area.clear();
						choosetxt_index[0] = choosetxt_index[1] = cursor_pointing_buffer;
					}
				}
				else {//如果是向后选择
					if (cursor_pointing_buffer > choosetxt_index[1]) {
						for (unsigned long i = 0; i < cursor_pointing - choosetxt_index[1]; i++) {
							choose_area.push_back(PointingCharToRect(choosetxt_index[1] + i));
						}
						choosetxt_index[1] = cursor_pointing;
					}
					else if ((cursor_pointing_buffer > choosetxt_index[0]) || cursor_pointing_buffer < choosetxt_index[1]) {
						isforward = true;
						for (unsigned long i = 0; i < cursor_pointing - choosetxt_index[0]; i++) {
							choose_area.pop_back();
						}
						choosetxt_index[1] = cursor_pointing;
					}
					else if (cursor_pointing_buffer < choosetxt_index[0]) {
						isforward = true;
						choose_area.clear();
						choosetxt_index[1] = choosetxt_index[0];
						for (unsigned long i = 0; i < choosetxt_index[0] - cursor_pointing; i++) {
							choose_area.push_front(PointingCharToRect(choosetxt_index[0] - i));
						}
						choosetxt_index[0] = cursor_pointing_buffer;
					}
					else if (cursor_pointing_buffer == choosetxt_index[0]) {
						isforward = true;
						choose_area.clear();
						choosetxt_index[0] = choosetxt_index[1] = cursor_pointing_buffer;
					}
				}
			}
		}
		lastmsg = msg;
	}

	void settext(wstring text_) {
		text = text_;
	}

	void setCursorShowmode(bool is_cursor_show_) {
		is_cursor_show = is_cursor_show_;
	}

	BoxStyle& getBoxstyleRef() {
		return style;
	}

	void setCursorColor(COLORREF cursor_color_) {
		cursor_color = cursor_color_;
	}
};
