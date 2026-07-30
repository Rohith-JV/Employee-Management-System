#ifndef EMS_THREADING_H
#define EMS_THREADING_H

/* Pthreads on POSIX; equivalent native primitives for the Windows MinGW build. */
#ifdef _WIN32
#include <windows.h>
typedef HANDLE EmsThread;
typedef CRITICAL_SECTION EmsMutex;
typedef HANDLE EmsCondition;
typedef DWORD EMS_THREAD_RETURN;
#define EMS_THREAD_CALL WINAPI

static inline int ems_mutex_init(EmsMutex *mutex) { InitializeCriticalSection(mutex); return 0; }
static inline void ems_mutex_destroy(EmsMutex *mutex) { DeleteCriticalSection(mutex); }
static inline void ems_mutex_lock(EmsMutex *mutex) { EnterCriticalSection(mutex); }
static inline void ems_mutex_unlock(EmsMutex *mutex) { LeaveCriticalSection(mutex); }
static inline int ems_condition_init(EmsCondition *condition) { *condition = CreateEvent(NULL, FALSE, FALSE, NULL); return *condition == NULL; }
static inline void ems_condition_destroy(EmsCondition *condition) { CloseHandle(*condition); }
static inline void ems_condition_signal(EmsCondition *condition) { SetEvent(*condition); }
static inline void ems_condition_wait(EmsCondition *condition, EmsMutex *mutex) { LeaveCriticalSection(mutex); WaitForSingleObject(*condition, INFINITE); EnterCriticalSection(mutex); }
static inline int ems_thread_create(EmsThread *thread, EMS_THREAD_RETURN (EMS_THREAD_CALL *start)(void *), void *argument) {
    *thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)start, argument, 0, NULL);
    return *thread == NULL;
}
static inline void ems_thread_join(EmsThread thread) { WaitForSingleObject(thread, INFINITE); CloseHandle(thread); }
#else
#include <pthread.h>
typedef pthread_t EmsThread;
typedef pthread_mutex_t EmsMutex;
typedef pthread_cond_t EmsCondition;
typedef void *EMS_THREAD_RETURN;
#define EMS_THREAD_CALL

static inline int ems_mutex_init(EmsMutex *mutex) { return pthread_mutex_init(mutex, NULL); }
static inline void ems_mutex_destroy(EmsMutex *mutex) { pthread_mutex_destroy(mutex); }
static inline void ems_mutex_lock(EmsMutex *mutex) { pthread_mutex_lock(mutex); }
static inline void ems_mutex_unlock(EmsMutex *mutex) { pthread_mutex_unlock(mutex); }
static inline int ems_condition_init(EmsCondition *condition) { return pthread_cond_init(condition, NULL); }
static inline void ems_condition_destroy(EmsCondition *condition) { pthread_cond_destroy(condition); }
static inline void ems_condition_signal(EmsCondition *condition) { pthread_cond_signal(condition); }
static inline void ems_condition_wait(EmsCondition *condition, EmsMutex *mutex) { pthread_cond_wait(condition, mutex); }
static inline int ems_thread_create(EmsThread *thread, EMS_THREAD_RETURN (EMS_THREAD_CALL *start)(void *), void *argument) { return pthread_create(thread, NULL, start, argument); }
static inline void ems_thread_join(EmsThread thread) { pthread_join(thread, NULL); }
#endif

#endif
