#pragma once

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // WIN32_LEAN_AND_MEAN
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif // _WIN32
#include <chrono>
#include <mutex>

#ifndef COMPILER_BARRIER
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define COMPILER_BARRIER()		_ReadWriteBarrier()
#else
#define COMPILER_BARRIER()		asm volatile ("" : : : "memory")
#endif
#endif

using namespace std::chrono;

class StopWatch {
private:
    std::chrono::time_point<high_resolution_clock> start_time_;
    std::chrono::time_point<high_resolution_clock> stop_time_;
    std::chrono::duration<double> interval_time_;
	double total_elapsed_time_;

public:
    StopWatch() : interval_time_{0}, total_elapsed_time_(0.0) {};
    ~StopWatch() {};

	void reset() {
		total_elapsed_time_ = 0.0;
        start_time_ = std::chrono::high_resolution_clock::now();
        interval_time_ = std::chrono::duration_cast< std::chrono::duration<double> >(start_time_ - start_time_);
	}

    void start() {
        start_time_ = std::chrono::high_resolution_clock::now();
		COMPILER_BARRIER();
    }

    void stop() {
		COMPILER_BARRIER();
        stop_time_ = std::chrono::high_resolution_clock::now();
    }

	void again() {
		double elapsed_time = getElapsedTime();
        COMPILER_BARRIER();
		total_elapsed_time_ += elapsed_time;
	}
    
    double getElapsedTime() {
        COMPILER_BARRIER();
        interval_time_ = std::chrono::duration_cast< std::chrono::duration<double> >(stop_time_ - start_time_);
        return interval_time_.count();
    }

    double getMillisec() {
        return getElapsedTime() * 1000.0;
    }

    double getSecond() {
        return getElapsedTime();
    }

    double getTotalMillisec() const {
        return getTotalSecond() * 1000.0;
    }

    double getTotalSecond() const {
        COMPILER_BARRIER();
        return total_elapsed_time_;
    }
};

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)
class StopWatch_v2 {
private:
    size_t start_time_;
    size_t stop_time_;
    double interval_time_;
	double total_elapsed_time_;

public:
    StopWatch_v2() : start_time_(0), stop_time_(0), interval_time_(0.0), total_elapsed_time_(0.0) {};
    ~StopWatch_v2() {};

	void reset() {
		total_elapsed_time_ = 0.0;
        start_time_ = timeGetTime();
        interval_time_ = 0.0;
	}

    void start() {
        start_time_ = timeGetTime();
		COMPILER_BARRIER();
    }

    void stop() {
		COMPILER_BARRIER();
        stop_time_ = timeGetTime();
    }

	void again() {
		double elapsed_time = getElapsedTime();
        COMPILER_BARRIER();
		total_elapsed_time_ += elapsed_time;
	}
    
    double getElapsedTime() {
        COMPILER_BARRIER();
        interval_time_ = (double)(stop_time_ - start_time_) / 1000.0;
        return interval_time_;
    }

    double getMillisec() {
        return getElapsedTime() * 1000.0;
    }

    double getSecond() {
        return getElapsedTime();
    }

    double getTotalMillisec() const {
        return getTotalSecond() * 1000.0;
    }

    double getTotalSecond() const {
        COMPILER_BARRIER();
        return total_elapsed_time_;
    }
};
#endif // _WIN32

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)
typedef StopWatch stop_watch;
#else
typedef StopWatch stop_watch;
#endif // _WIN32

#undef COMPILER_BARRIRER
