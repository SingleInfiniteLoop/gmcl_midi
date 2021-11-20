#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <windows.h>

#define MAG10E7                 10000000
#define MAG10E9                 1000000000

/* Number of 100ns-seconds between the beginning of the Windows epoch
 * (Jan. 1, 1601) and the Unix epoch (Jan. 1, 1970)
 */
#define DELTA_EPOCH_IN_100NS    INT64_C(116444736000000000)

static inline int lc_set_errno(int result)
{
    if (result != 0) {
        errno = result;
        return -1;
    }
    return 0;
}

/**
 * Get the time of the specified clock clock_id and stores it in the struct
 * timespec pointed to by tp.
 * @param  clock_id The clock_id argument is the identifier of the particular
 *         clock on which to act. The following clocks are supported:
 * <pre>
 *     CLOCK_REALTIME  System-wide real-time clock. Setting this clock
 *                 requires appropriate privileges.
 *     CLOCK_MONOTONIC Clock that cannot be set and represents monotonic
 *                 time since some unspecified starting point.
 *     CLOCK_PROCESS_CPUTIME_ID High-resolution per-process timer from the CPU.
 *     CLOCK_THREAD_CPUTIME_ID  Thread-specific CPU-time clock.
 * </pre>
 * @param  tp The pointer to a timespec structure to receive the time.
 * @return If the function succeeds, the return value is 0.
 *         If the function fails, the return value is -1,
 *         with errno set to indicate the error.
 */
int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    int result = EINVAL;
    unsigned __int64 timestamp;
    LARGE_INTEGER frequency, counter;
    union
    {
        unsigned __int64 u64;
        FILETIME ftime;
    } creation_time, exit_time, kernel_time, user_time;

    switch (clock_id)
    {
    case CLOCK_REALTIME:
        GetSystemTimeAsFileTime(&creation_time.ftime);
        timestamp = creation_time.u64 - DELTA_EPOCH_IN_100NS;
        tp->tv_sec = timestamp / MAG10E7;
        tp->tv_nsec = (int)(timestamp % MAG10E7) * 100;
        result = 0;
        break;
    case CLOCK_MONOTONIC:
        if (QueryPerformanceFrequency(&frequency) &&
            QueryPerformanceCounter(&counter))
        {

            tp->tv_sec = counter.QuadPart / frequency.QuadPart;
            tp->tv_nsec = (int)(((counter.QuadPart % frequency.QuadPart) *
                                 MAG10E9 + (frequency.QuadPart >> 1)) /
                                frequency.QuadPart);
            if (tp->tv_nsec >= MAG10E9)
            {
                tp->tv_sec++;
                tp->tv_nsec -= MAG10E9;
            }
            result = 0;
        }
        else
        result = lc_set_errno(EINVAL);
        break;
    case CLOCK_PROCESS_CPUTIME_ID:
        if (GetProcessTimes(GetCurrentProcess(),
                            &creation_time.ftime, &exit_time.ftime,
                            &kernel_time.ftime, &user_time.ftime))
        {
            timestamp = kernel_time.u64 + user_time.u64;
            tp->tv_sec = timestamp / MAG10E7;
            tp->tv_nsec = (int)(timestamp % MAG10E7) * 100;
            result = 0;
        }
        break;
    case CLOCK_THREAD_CPUTIME_ID: 
        if (GetThreadTimes(GetCurrentThread(),
                           &creation_time.ftime, &exit_time.ftime,
                           &kernel_time.ftime, &user_time.ftime))
        {
            timestamp = kernel_time.u64 + user_time.u64;
            tp->tv_sec = timestamp / MAG10E7;
            tp->tv_nsec = (int)(timestamp % MAG10E7) * 100;
            result = 0;
        }
        break;
    default:
        ;
    }
    return lc_set_errno(result);
}

