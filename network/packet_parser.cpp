#include "packet_parser.h"

#include <cstring>
#include <optional>

namespace network
{
    packet_parser::packet_parser(const unsigned char& start_code_value, const unsigned char& end_code_value) 
        : _thread_stop(false), _notification(nullptr)
    {
        memset(_start_code_tag, start_code_value, start_code);
		memset(_end_code_tag, end_code_value, end_code);

        _thread = make_shared<thread>(&packet_parser::run, this);
    }

    packet_parser::~packet_parser(void)
    {
        _thread_stop =true;

		if (_thread != nullptr)
		{
			if (_thread->joinable())
			{
				_condition.notify_one();
				_thread->join();
			}
			_thread.reset();
		}

		_thread_stop = false;
    }

    bool packet_parser::append(const vector<uint8_t>& data)
    {
        if (_notification == nullptr)
        {
            return false;
        }

		scoped_lock<mutex> guard(_mutex);

        _buffers.insert(_buffers.end(), data.begin(), data.end());

        _condition.notify_one();

        return true;
    }

    void packet_parser::set_notification(const function<void(const data_modes&, const vector<uint8_t>&)>& notification)
    {
        _notification = notification;
    }

    void packet_parser::run(void)
    {
        bool parsed_start_code = false;
        optional<data_modes> packet_mode = {};
        bool parsed_length_code = false;
        size_t data_length = 0;

        vector<uint8_t> buffers;
        vector<uint8_t> parsed_data;

        while(!_thread_stop)
        {
            unique_lock<mutex> unique(_mutex);
			_condition.wait(unique, [this] { return _thread_stop || !_buffers.empty(); });

			buffers.insert(buffers.end(), _buffers.begin(), _buffers.end());
            _buffers.clear();

			unique.unlock();

            if (!parsed_start_code)
            {
                if (start_code > buffers.size())
                {
                    continue;
                }

                size_t index = 0;
                bool compared = false;
                size_t count = buffers.size() - start_code;
                for(; index < count; ++index)
                {
                    if (buffers[index] != _start_code_tag[0])
                    {
                        continue;
                    }

                    compared = true;
                    for(size_t index2 = 1; index2 < start_code; ++index2)
                    {
                        if (buffers[index2] != _start_code_tag[index2])
                        {
                            compared = false;
                            break;
                        }
                    }

                    if (!compared)
                    {
                        continue;
                    }

                    break;
                }
                
                parsed_start_code = compared;
                buffers.erase(buffers.begin(), buffers.begin() + index + (compared ? start_code : 0));
            }

            if (!packet_mode.has_value())
            {
                if (mode_code > buffers.size())
                {
                    continue;
                }

                packet_mode = { (data_modes)buffers[0] };
                buffers.erase(buffers.begin());
            }

            if (!parsed_length_code)
            {
                if (length_code > buffers.size())
                {
                    continue;
                }

                data_length = 0;
                memcpy(&data_length, buffers.data(), length_code);
                buffers.erase(buffers.begin(), buffers.begin() + length_code);
                parsed_length_code = true;
            }

            if (data_length > 0)
            {
                if (data_length > buffers.size())
                {
                    continue;
                }

                parsed_data = vector<uint8_t>(buffers.begin(), buffers.begin() + data_length);
                buffers.erase(buffers.begin(), buffers.begin() + data_length);
                data_length = 0;
            }

            if (data_length == 0)
            {
                if (end_code > buffers.size())
                {
                    continue;
                }

                _notification(packet_mode.value(), parsed_data);

                buffers.erase(buffers.begin(), buffers.begin() + end_code);
                parsed_start_code = false;
                packet_mode.reset();
                parsed_length_code = false;
            }
        }
    }
}