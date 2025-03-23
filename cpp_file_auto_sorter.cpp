#include<stdio.h>
#include<windows.h>

//
// Function: get_date
// ----------------------------
// Gets the current local date and formats it as a string "YYYY-MM-DD".
// This is used for naming folders based on today's date.
//
// Parameters:
//   buffer - A char array where the date string will be stored
//   size   - The size of the buffer
//
void get_date(char* buffer, int size){
	SYSTEMTIME st;
	GetLocalTime(&st);
	snprintf(buffer, size, "%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
}

//
// Function: is_file_create_today
// ----------------------------
// Checks whether a given file was created today.
//
// Parameters:
//   filepath - Full path to the file to check
//
// Returns:
//   TRUE if the file was created today, FALSE otherwise
//
BOOL is_file_create_today(const char* filepath){
	printf("Function 2 called\n");
	HANDLE file;
	FILETIME ftCreate, ftLocal;
	SYSTEMTIME stFileTime, stNow;

	printf("Trying to open file: %s\n", filepath);

	file = CreateFile(
		filepath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (file == INVALID_HANDLE_VALUE) {
		printf("[DEBUG] CreateFile failed (Error code: %lu)\n", GetLastError());
		return FALSE;
	}

	if (!GetFileTime(file, &ftCreate, NULL, NULL)) {
		CloseHandle(file);
		return FALSE;
	}

	FileTimeToLocalFileTime(&ftCreate, &ftLocal);
	FileTimeToSystemTime(&ftLocal, &stFileTime);
	GetLocalTime(&stNow);
	CloseHandle(file);

	printf("File creation date: %04d-%02d-%02d\n", stFileTime.wYear, stFileTime.wMonth, stFileTime.wDay);
	printf("Current date:       %04d-%02d-%02d\n", stNow.wYear, stNow.wMonth, stNow.wDay);

	if (stFileTime.wYear == stNow.wYear &&
		stFileTime.wMonth == stNow.wMonth &&
		stFileTime.wDay == stNow.wDay)
		return TRUE;

	return FALSE;
}

//
// Function: move_file_if_created_today
// ------------------------------------
// Moves the specified file into a folder named with today's date,
// but only if the file was created today.
//
// Parameters:
//   directory - The directory where the file currently resides
//   fileName  - The name of the file to move
//
void move_file_if_created_today(const char* directory, const char* fileName){
	char filepath[MAX_PATH];
	char dateStr[20];
	char dateFolder[MAX_PATH];
	char newFilepath[MAX_PATH];

	snprintf(filepath, sizeof(filepath), "%s\\%s", directory, fileName);

	is_file_create_today(filepath);
	// Uncomment to only move files created today
	// if (!is_file_create_today(filepath)) return;

	get_date(dateStr, sizeof(dateStr));
	printf("%s\n", dateStr);

	snprintf(dateFolder, sizeof(dateFolder), "%s\\%s", directory, dateStr);
	CreateDirectory(dateFolder, NULL);

	snprintf(newFilepath, sizeof(newFilepath), "%s\\%s", dateFolder, fileName);

	MoveFile(filepath, newFilepath);
}

//
// Function: check_folder
// ----------------------------
// Continuously monitors a directory for newly added or renamed files.
// If a new .cpp file is detected, checks its creation date and moves
// it to today's date-named folder if applicable.
//
// Parameters:
//   directory - The directory to monitor
//
void check_folder(const char* directory){
	HANDLE hDirectory;
	char buffer[1024];
	DWORD bytesReturned;

	hDirectory = CreateFile(
		directory,
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL);

	if (hDirectory == INVALID_HANDLE_VALUE) {
		printf("Failed to open folder.\n");
		return;
	}

	printf("Monitoring folder: %s\n", directory);

	while (1) {
		if (ReadDirectoryChangesW(
			hDirectory,
			buffer,
			sizeof(buffer),
			FALSE,
			FILE_NOTIFY_CHANGE_FILE_NAME,
			&bytesReturned,
			NULL,
			NULL
		)) {
			FILE_NOTIFY_INFORMATION* info = (FILE_NOTIFY_INFORMATION*)buffer;
			printf("Detected action: %d\n", info->Action);

			if (info->Action == FILE_ACTION_ADDED || info->Action == FILE_ACTION_RENAMED_NEW_NAME) {
				char fileName[MAX_PATH];
				int len = WideCharToMultiByte(CP_ACP, 0, info->FileName, info->FileNameLength / sizeof(WCHAR),
					fileName, MAX_PATH, NULL, NULL);
				fileName[len] = '\0';

				if (strstr(fileName, ".cpp")) {
					Sleep(300);
					move_file_if_created_today(directory, fileName);
					printf("[Move attempt] %s\n", fileName);
				}
				else {
					printf("[Ignored file] %s\n", fileName);
				}
			}
		}
		Sleep(100);
	}
	CloseHandle(hDirectory);
}

//
// Main Function
//
int main() {
	check_folder("C:\\Users\\dbswodyd\\Desktop\\study\\C\\C Express");
	return 0;
}

