#ifndef _performance_h_
#define _performance_h_

class Performance
{
	public:
		void start() { 
			gettimeofday(&_starttime,0x0);
		}
		void stop() {
			gettimeofday(&_endtime, 0x0);
		}
		int report_nsec() {
			return _endtime.tv_usec - _starttime.tv_usec;
		}
		int report_sec() {
			return _endtime.tv_sec - _starttime.tv_sec;
		}

	private: 
		struct timeval _starttime, _endtime, _timediff;
};

#endif
