#include "packet_parser.h"

namespace network
{
    packet_parser::packet_parser(void)
    {
    }

    packet_parser::~packet_parser(void)
    {
    }

    void packet_parser::append(const vector<uint8_t>& data)
    {
		scoped_lock<mutex> guard(_mutex);

        _buffers.insert(_buffers.end(), data.begin(), data.end());

        _condition.notify_one();
    }

    void packet_parser::set_notification(const function<void(const data_modes&, const vector<uint8_t>&)>& notification)
    {
        _notification = notification;
    }
}