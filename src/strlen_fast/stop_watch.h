#pragma once

#include <chrono>

#ifndef COMPILER_BARRIER
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define COMPILER_BARRIER()		_ReadWriteBarrier()
#else
#define COMPILER_BARRIER()		asm volatile ("" : : : "memory")
#endif
#endif

using namespace std::chrono;

class StopWatch {
public:
    typedef std::chrono::time_point<high_resolution_clock>  time_clock;
    typedef std::chrono::duration<double>                   time_elapsed;

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
        interval_time_ = std::chrono::duration_cast<time_elapsed>(start_time_ - start_time_);
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

#undef COMPILER_BARRIRER
