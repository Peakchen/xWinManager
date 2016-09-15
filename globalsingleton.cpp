#include "globalsingleton.h"
#include "swmgrapp.h"

GlobalSingleton::GlobalSingleton() :m_Mutex(INVALID_HANDLE_VALUE)
{

}

GlobalSingleton::~GlobalSingleton() {
	if (m_Mutex != INVALID_HANDLE_VALUE) {
		CloseHandle(m_Mutex);
	}
}

GlobalSingleton *GlobalSingleton::Instance() {
	static GlobalSingleton _instance;
	HANDLE hMutex = INVALID_HANDLE_VALUE;
	if (_instance.m_Mutex == INVALID_HANDLE_VALUE) {
		std::string mutexName;
		mutexName.append("Global\\");
		mutexName.append(SwmgrApp::GetSoftwareName().toStdString());
		hMutex = CreateEventA(NULL, FALSE, FALSE, mutexName.data());
		if (GetLastError() == ERROR_ALREADY_EXISTS || hMutex == NULL) {
			if (hMutex != NULL) {
				CloseHandle(hMutex);
			}
			return NULL;
		}
		else {
			_instance.setGlobalMutex(hMutex);
		}
	}
    return &_instance;
}

void GlobalSingleton::setGlobalMutex(HANDLE hMutex) {
    m_Mutex = hMutex;
}
