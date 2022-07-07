#pragma once

#include <mutex>
#include <memory>
#include <thread>
#include <vector>
#include <stdint.h>
#include <functional>
#include <condition_variable>

#include "data_modes.h"
#include "data_lengths.h"

namespace network
{
    using namespace std;

    class packet_parser
    {
    public:
        packet_parser(const unsigned char& start_code_value, const unsigned char& end_code_value);
        ~packet_parser(void);

    public:
        bool append(const vector<uint8_t>& data);
        void set_notification(const function<void(const data_modes&, const vector<uint8_t>&)>& notification);

    protected:
		void run(void);

    private:
		mutex _mutex;
		shared_ptr<thread> _thread;
		condition_variable _condition;

    private:
        bool _thread_stop;
        vector<uint8_t> _buffers;
		char _start_code_tag[start_code];
		char _end_code_tag[end_code];
        function<void(const data_modes&, const vector<uint8_t>&)> _notification;
    };
}