/*
 * @author: JiJie, simple_ai@outlook.com
 * This file is part of d_compute.
 * 
 * ----- The MIT License (MIT) ----- 
 * Copyright (c) 2009, JiJie
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
#ifndef _XLOGGER_H_
#define _XLOGGER_H_

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "VCriticalSection.h"

#ifdef _WIN32
#else
#include <unistd.h>
#endif

class XLogger
{
public:
#define LOG_TO_DEBUGER		0x01
#define LOG_TO_FILE			0x02
#define LOG_TO_CONSOLE		0x04
#define LINE_BUFFER_SIZE	1024
	XLogger( LPCTSTR filename = NULL, INT32 level = 1, INT32 mode = LOG_TO_DEBUGER | LOG_TO_FILE | LOG_TO_CONSOLE, bool append = false);
	
	inline VOID Print(INT32 level, LPCTSTR format, ...) {
		if (level > m_level) return;
			va_list ap;
			va_start(ap, format);
			ReallyPrint( format, ap);
			va_end(ap);
	}

	void SetMode(INT32 mode);

	INT32 GetMode();

	VOID SetLevel(INT32 level);
	INT32  GetLevel();

	VOID SetFile(LPCTSTR filename, bool append = false);
	virtual ~XLogger();
private:
	VOID ReallyPrintLine( LPCTSTR line);
	VOID ReallyPrint( LPCTSTR format, va_list ap);
	VOID OpenFile();
	VOID CloseFile();

	bool m_tofile, m_todebug, m_toconsole;
	INT32 m_level;
	INT32 m_mode;
	HANDLE hlogfile;
	LPSTR m_filename;
	bool m_append;

	time_t m_lastLogTime;
	VCriticalSection m_CriticalSection;
};

#endif // _XLOGGER_H_
