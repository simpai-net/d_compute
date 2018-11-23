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
 
#include "stdafx.h"
#include "XLogger.h"

#ifdef _WIN32
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

XLogger::XLogger( LPCTSTR filename, INT32 level, INT32 mode, bool append )
{
	m_lastLogTime = 0;
	m_filename = NULL;
	m_append = false;
	hlogfile = NULL;
	m_todebug = false;
	m_toconsole = false;
	m_tofile = false;
	m_level = level;
	SetFile(filename, append);
	SetMode(mode);
}

void XLogger::SetMode(INT32 mode)
{
	m_mode = mode;
	if (mode & LOG_TO_DEBUGER)
		m_todebug = true;
	else
		m_todebug = false;
	if (mode & LOG_TO_FILE)
	{
		if (!m_tofile)
			OpenFile();
	}
	else
	{
		CloseFile();
		m_tofile = false;
	}
#ifdef _WIN32
	if (mode & LOG_TO_CONSOLE) {
		if (!m_toconsole)
			AllocConsole();
		m_toconsole = true;
	} else {
		m_toconsole = false;
	}
#else
#endif
}

INT32 XLogger::GetMode()
{
	return m_mode;
}

void XLogger::SetLevel(INT32 level)
{
	m_level = level;
}

INT32 XLogger::GetLevel() {
	return m_level;
}

void XLogger::SetFile(LPCTSTR filename, bool append) 
{
	CloseFile();
	if (m_filename != NULL)
	{
		free(m_filename);
		m_filename = NULL;
	}
	if(filename != NULL)
	{
		m_filename = strdup(filename);
	}

	m_append = append;
	OpenFile();
}
void XLogger::OpenFile()
{
	if (m_filename == NULL)
	{
		m_tofile = false;
		return;
	}
	m_tofile  = true;
	if (!m_append)
	{
		char *backupfilename = new char[strlen(m_filename)+5];
		if (backupfilename)
		{
			_tcscpy(backupfilename, m_filename);
			_tcscat(backupfilename, ".bak");
#ifdef _WIN32
			DeleteFile(backupfilename);
			MoveFile(m_filename, backupfilename);
#else
			remove(backupfilename);
			rename(m_filename, backupfilename);
#endif
			delete [] backupfilename;
		}
	}

#ifdef _WIN32
	hlogfile = CreateFile(
		m_filename,  GENERIC_WRITE, FILE_SHARE_READ, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL  );
#else
	hlogfile = open(m_filename, O_CREAT | O_RDWR | O_APPEND | O_SYNC, 00755);
#endif
#ifdef _WIN32
	if (hlogfile == INVALID_HANDLE_VALUE) {
#else
	if (hlogfile == -1) {
#endif
		m_todebug = true;
		m_tofile = false;
		Print(0, "Error opening log file %s\n", m_filename);
	}

#ifdef _WIN32
	if (m_append) 
	{
		SetFilePointer( hlogfile, 0, NULL, FILE_END );
	}
	else 
	{
		SetEndOfFile( hlogfile );
	}
#else

	if( m_append) {
		lseek(hlogfile, 0, SEEK_END);
	}
	else
	{
		ftruncate(hlogfile, 0);
	}
#endif
}

void XLogger::CloseFile() {
	if (hlogfile != NULL) {
#ifdef _WIN32
		CloseHandle(hlogfile);
#else
		close(hlogfile);
#endif
		hlogfile = NULL;
	}
}
inline void XLogger::ReallyPrintLine(LPCTSTR line) 
{
	m_CriticalSection.Enter();
#ifdef _WIN32
	if (m_todebug) OutputDebugString(line);
	if (m_toconsole) {
		DWORD byteswritten;
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), line, ( DWORD )strlen(line), &byteswritten, NULL); 
	};
#else
#endif
	if (m_tofile && (hlogfile != NULL)) {
		DWORD byteswritten;
#ifdef _WIN32
		WriteFile(hlogfile, line, strlen(line), &byteswritten, NULL); 
#else
		write(hlogfile, line, strlen(line));
#endif
	}
	m_CriticalSection.Leave();
}
void XLogger::ReallyPrint(LPCTSTR format, va_list ap) 
{
	time_t current = time(0);
	if (current != m_lastLogTime) {
		m_lastLogTime = current;

		char time_str[32];
		memcpy(time_str + 24 * sizeof(CHAR), "\r\n", sizeof(CHAR) * 2);
		memcpy(time_str, ctime(&m_lastLogTime), 24 * sizeof(CHAR));
		time_str[2 + 24] = (CHAR)'\0'; 
		ReallyPrintLine(time_str);
	}

	CHAR line[LINE_BUFFER_SIZE];
#ifdef _WIN32
	_vsnprintf(line, sizeof(line) - 2 * sizeof(CHAR), format, ap);
#else
	vsnprintf(line, sizeof(line) - 2 * sizeof(CHAR), format, ap);
#endif
	line[LINE_BUFFER_SIZE-2] = (CHAR)'\0';
	INT32 len = strlen(line);
	if (len > 0 && len <= sizeof(line) - 2 * sizeof(CHAR) && line[len-1] == (CHAR)'\n') {
		line[len-1] = (CHAR)'\r';
		line[len] = (CHAR)'\n';
		line[len+1] = (CHAR)'\0';
	}
	ReallyPrintLine(line);
}

XLogger::~XLogger()
{
	if (m_filename != NULL)
		free(m_filename);
	CloseFile();
}