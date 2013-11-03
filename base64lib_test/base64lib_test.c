/*
 * base64lib - Base64 Encoding/Decoding Library
 * Copyright (C) 2013 Fernando Rodriguez
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <tchar.h>
#include <errno.h>
#include <dos.h>

#include "base64lib_test.h"
#include "..\base64lib\base64.h"
#include "..\base64lib\base64io.h"

#pragma comment(lib, "..\\debug\\base64lib.lib")

//
// Global Variables:
//
HWND hDlg;
HANDLE hWorkingThread;
BASE64IO io;
FILE* in;
FILE* out;
BOOL encoding = TRUE;
BOOL bPaused = FALSE;
char buff[1024];

//
// Forward declarations of functions included in this code module:
//
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void onOK();
void onCancel();
void onProgress();
void onPause();
void onBrowse(BOOL input);
void onRadioButtonChanged();
void onClosing();
void onDoWork(); 

void onDoWork() 
{
	//
	// do encoding/decoding
	//
	if (encoding) 
	{
		base64io_encode(&io, in, out);
	}
	else 
	{
		base64io_decode(&io, in, out);
	}
	//
	// close files
	//
	fclose(out);
	fclose(in);
}

void onOK()
{
	TCHAR strInputW[MAX_PATH + 1];
	TCHAR strOutputW[MAX_PATH + 1];
	char strInputFilename[MAX_PATH + 1];
	char strOutputFilename[MAX_PATH + 1];
	//
	// read the input and output file names and convert them to
	// wide chars
	//
	strInputW[SendDlgItemMessage(hDlg, ID_EDIT_INPUT, EM_GETLINE, 0, (LPARAM) strInputW)] = NULL;
	strOutputW[SendDlgItemMessage(hDlg, ID_EDIT_OUTPUT, EM_GETLINE, 0, (LPARAM) strOutputW)] = NULL;
	WideCharToMultiByte(CP_ACP, NULL, strInputW, wcslen(strInputW) + 1, strInputFilename, MAX_PATH + 1, NULL, FALSE);
	WideCharToMultiByte(CP_ACP, NULL, strOutputW, wcslen(strOutputW) + 1, strOutputFilename, MAX_PATH + 1, NULL, FALSE);
	//
	// prepare the base64io structure
	//
	io.chunk_buffer_size = 1024;	// set the working buffer size
	io.chunk_buffer = buff;			// to let library allocate buffer set to 0
	io.result = 0;					// clear result code
	io.running = 0;					// clear the running flag
	io.started = 0;					// clear started flag
	io.abort_requested = 0;			// clear abort_requested flag
	io.callback_function = 0;	// no callback

	if (in = fopen(strInputFilename, "rb"))
	{
		if (out = fopen(strOutputFilename, "wb")) 
		{
			//
			// disable UI and fire timer to update progress bar
			//
			EnableWindow(GetDlgItem(hDlg, ID_BROWSE_INPUT), FALSE);
			EnableWindow(GetDlgItem(hDlg, ID_BROWSE_OUTPUT), FALSE);
			EnableWindow(GetDlgItem(hDlg, ID_EDIT_INPUT), FALSE);
			EnableWindow(GetDlgItem(hDlg, ID_EDIT_OUTPUT), FALSE);
			EnableWindow(GetDlgItem(hDlg, ID_RB_ENCODE), FALSE);
			EnableWindow(GetDlgItem(hDlg, ID_RB_DECODE), FALSE);
			EnableWindow(GetDlgItem(hDlg, ID_OK), FALSE);
			SendDlgItemMessage(hDlg, ID_PROGRESS, PBM_SETPOS, 0, 0);
			SendDlgItemMessage(hDlg, ID_CANCEL, WM_SETTEXT, 0, (LPARAM) TEXT("Abort"));
			ShowWindow(GetDlgItem(hDlg, ID_PROGRESS), SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, ID_PAUSE), SW_SHOW);
			SetTimer(hDlg, 1, 100, (TIMERPROC) &onProgress);
			//
			// do work in new thread
			//
			hWorkingThread = 
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) &onDoWork, NULL, NULL, NULL);
		}
		else
		{
			TCHAR msg[500];
			TCHAR* fmt = TEXT("Could not open file: %s\r\nError code: %x");
			swprintf(msg, 500, fmt, strOutputW, errno);
			MessageBox(hDlg, msg, TEXT("Error"), MB_OK | MB_APPLMODAL | MB_ICONERROR);
			fclose(in);
		}
	}
	else
	{
		TCHAR msg[500];
		TCHAR* fmt = TEXT("Could not open file: %s\r\nError code: %x");
		swprintf(msg, 500, fmt, strInputW, errno);
		MessageBox(hDlg, msg, TEXT("Error"), MB_OK | MB_APPLMODAL | MB_ICONERROR);
	}
}

void onCancel()
{
	if (io.started && io.running)
	{
		if(MessageBox(hDlg, TEXT("Do you really want to abort?"), 
			TEXT("Abort"), MB_ICONQUESTION | MB_YESNO | MB_APPLMODAL) == IDYES)
		{
			//
			// disable cancel button
			//
			EnableWindow(GetDlgItem(hDlg, ID_OK), FALSE);
			//
			// request base64io thread to abort
			//
			io.abort_requested = 1;
			//
			// if working thread is paused then resume it
			//
			if (bPaused)
				onPause();
		}
	}
	else
	{
		//
		// close window
		//
		SendMessage(hDlg, WM_CLOSE, 0, 0);
	}
}

void onBrowse(BOOL input)
{
	BOOL rc = FALSE;
	TCHAR strPath[MAX_PATH + 1] = { 0 };

	OPENFILENAME ofn;
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlg;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = TEXT("All Files (*.*)\0*.*\0");
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = strPath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = input ? TEXT("Select Input file...") : TEXT("Select output file...");
	ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY;
	ofn.lpstrInitialDir = TEXT("");
	ofn.lpstrDefExt = TEXT("");
	ofn.lpstrCustomFilter = TEXT("");

	//
	// use the open file dialog to get the filename
	//
	rc = GetOpenFileName(&ofn);
	//
	// copy the filename to the edit control
	//
	if (rc) 
	{
		if (input)
		{
			SendDlgItemMessage(hDlg, ID_EDIT_INPUT, WM_SETTEXT, 0, (LPARAM) strPath);
		}
		else
		{
			SendDlgItemMessage(hDlg, ID_EDIT_OUTPUT, WM_SETTEXT, 0, (LPARAM) strPath);
		}
	}
}

void onRadioButtonChanged()
{
	LRESULT r = SendDlgItemMessage(hDlg, ID_RB_ENCODE, BM_GETCHECK, 0, 0);

	if (r == BST_CHECKED)
	{
		encoding = TRUE;
		SendDlgItemMessage(hDlg, ID_OK, WM_SETTEXT, 0, (LPARAM) TEXT("Encode"));
	}
	else
	{
		encoding = FALSE;
		SendDlgItemMessage(hDlg, ID_OK, WM_SETTEXT, 0, (LPARAM) TEXT("Decode"));
	}
}

void onProgress() 
{

	if (io.bytes_total > 0)
	{
		float pcnt = (float) io.bytes_processed / (float) io.bytes_total * 100;
		SendDlgItemMessage(hDlg, ID_PROGRESS, PBM_SETPOS, (int) pcnt, 0);
	}
	else
	{
		SendDlgItemMessage(hDlg, ID_PROGRESS, PBM_SETPOS, 0, 0);
	}
	if (io.started && !io.running)
	{
		//
		// enable controls
		//
		KillTimer(hDlg, 1);
		EnableWindow(GetDlgItem(hDlg, ID_BROWSE_INPUT), TRUE);
		EnableWindow(GetDlgItem(hDlg, ID_BROWSE_OUTPUT), TRUE);
		EnableWindow(GetDlgItem(hDlg, ID_EDIT_INPUT), TRUE);
		EnableWindow(GetDlgItem(hDlg, ID_EDIT_OUTPUT), TRUE);
		EnableWindow(GetDlgItem(hDlg, ID_RB_ENCODE), TRUE);
		EnableWindow(GetDlgItem(hDlg, ID_RB_DECODE), TRUE);
		EnableWindow(GetDlgItem(hDlg, ID_OK), TRUE);
		EnableWindow(GetDlgItem(hDlg, ID_CANCEL), TRUE);
		ShowWindow(GetDlgItem(hDlg, ID_PAUSE), SW_HIDE);
		SendDlgItemMessage(hDlg, ID_CANCEL, WM_SETTEXT, 0, (LPARAM) TEXT("Exit"));

		if (io.result == BASE64_SUCCESS) 
		{
			if (encoding)
			{
				MessageBox(hDlg, TEXT("Encoding completed successfully."), 
					TEXT("Finished"), MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(hDlg, TEXT("Decoding completed successfully."), 
					TEXT("Finished"), MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
			}
		}
		else
		{
			TCHAR msg[500];
			TCHAR err[100];
			MultiByteToWideChar(CP_ACP, NULL, base64_translate_result(io.result), 100, err, 100);
			swprintf(msg, 500, TEXT("Encoding error: %s"), err);
			MessageBox(hDlg, msg, TEXT("Error!"), MB_OK | MB_APPLMODAL | MB_ICONERROR);
		}

		ShowWindow(GetDlgItem(hDlg, ID_PROGRESS), SW_HIDE);
		CloseHandle(hWorkingThread);
		hWorkingThread = NULL;
	}
}

void onClosing()
{
	if(MessageBox(hDlg, TEXT("Are you sure you want to exit?"), 
		TEXT("Exit"), MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		DestroyWindow(hDlg);
	}
}

void onPause()
{
	if (!bPaused)
	{
		SuspendThread(hWorkingThread);
		bPaused = TRUE;
		SendDlgItemMessage(hDlg, ID_PAUSE, 
			WM_SETTEXT, 0, (LPARAM) TEXT("Resume"));
	}
	else
	{
		ResumeThread(hWorkingThread);
		bPaused = FALSE;
		SendDlgItemMessage(hDlg, ID_PAUSE, 
			WM_SETTEXT, 0, (LPARAM) TEXT("Pause"));
	}
}

//
// application enntry
//
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	BOOL ret;
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	//
	// initialize and show dialog
	//
	InitCommonControls();
	hDlg = CreateDialogParam(NULL, MAKEINTRESOURCE(IDD_DIALOG1), 0, DialogProc, 0);
	ShowWindow(hDlg, nCmdShow);

	SendDlgItemMessage(hDlg, ID_RB_ENCODE, BM_SETCHECK, 1, 0);

	//
	// message loop
	//
	while (ret = GetMessage(&msg, NULL, 0, 0) != 0) 
	{
		if (ret == -1) return ret;
		if (!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int) msg.wParam;
}

//
// message callback function for dialog
//
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case ID_BROWSE_INPUT: onBrowse(TRUE); return TRUE;
				case ID_BROWSE_OUTPUT: onBrowse(FALSE); return TRUE;
				case ID_RB_ENCODE: onRadioButtonChanged(); return TRUE;
				case ID_RB_DECODE: onRadioButtonChanged(); return TRUE;
				case ID_PAUSE: onPause(); return TRUE;
				case ID_OK: onOK(); return TRUE;
				case ID_CANCEL: onCancel(); return TRUE;
			}
			break;
		case WM_CLOSE: onClosing(); return TRUE;
		case WM_DESTROY: PostQuitMessage(0); return TRUE;
  }
  return FALSE;
}
