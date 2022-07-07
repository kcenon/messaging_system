#pragma once

#include <mutex>
#include <vector>
#include <stdint.h>
#include <functional>
#include <condition_variable>

#include "data_modes.h"

namespace network
{
    using namespace std;

    class packet_parser
    {
    public:
        packet_parser(void);
        ~packet_parser(void);

    public:
        void append(const vector<uint8_t>& data);
        void set_notification(const function<void(const data_modes&, const vector<uint8_t>&)>& notification);

    private:
		mutex _mutex;
		condition_variable _condition;

    private:
        vector<uint8_t> _buffers;
        function<void(const data_modes&, const vector<uint8_t>&)> _notification;
    };
}