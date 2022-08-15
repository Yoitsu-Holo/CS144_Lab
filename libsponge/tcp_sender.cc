#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>
#include <iostream>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _retransmission_timeout(retx_timeout) {}

void TCPSender::fill_window() {
    TCPSegment seg;
    if (!_syn) {
        // set header
        seg.header().syn = true;
        seg.header().seqno = wrap(0, _isn);
        // update status
        _syn = true;
        _next_seqno = 1;
        _segments_outstanding.push(seg);
        _byte_in_flight++;
        // send segment
        _segments_out.push(seg);
    } else {  // send other segment
        size_t win_size = (_win_size == 0) ? (1) : (_win_size);
        size_t remain_size = 0;
        while (_fin == false && (remain_size = win_size - (_next_seqno - _recv_ackno)) != 0) {
            size_t payload_size = min(remain_size, TCPConfig::MAX_PAYLOAD_SIZE);
            string payload = _stream.read(payload_size);
            seg.payload() = Buffer(std::move(payload));

            if (_stream.eof() && seg.length_in_sequence_space() < remain_size) {
                seg.header().fin = true;
                _fin = true;
            }

            // stream is empty, break
            if (seg.length_in_sequence_space() == 0)
                break;

            // send and update
            seg.header().seqno = next_seqno();
            _next_seqno += seg.length_in_sequence_space();
            _byte_in_flight += seg.length_in_sequence_space();
            _segments_outstanding.push(seg);
            _segments_out.push(seg);
        }
    }

    if (!_time_run) {
        _time_run = true;
        _RTO = 0;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    size_t absolute_seqno = unwrap(ackno, _isn, _recv_ackno);

    if (_syn && _next_seqno == 1 && _next_seqno == 1 && _next_seqno != absolute_seqno)
        return;

    _win_size = window_size;

    if (absolute_seqno <= _recv_ackno)
        return;

    _recv_ackno = absolute_seqno;
    TCPSegment seg;
    while (!_segments_outstanding.empty()) {
        seg = _segments_outstanding.front();
        if (unwrap(seg.header().seqno, _isn, _recv_ackno) + seg.length_in_sequence_space() <= absolute_seqno) {
            _byte_in_flight -= seg.length_in_sequence_space();
            _segments_outstanding.pop();
        } else
            break;
    }

    fill_window();
    _retransmission_timeout = _initial_retransmission_timeout;
    _consecutive_retransmissions = 0;
    _RTO = 0;
    return;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_time_run)
        return;
    _RTO += ms_since_last_tick;
    if (_RTO >= _retransmission_timeout && _time_run && !_segments_outstanding.empty()) {
        // if timeout, retransmission
        _segments_out.push(_segments_outstanding.front());

        // retransmisson count add 1
        _consecutive_retransmissions++;

        // doubled retransmisson_timeout
        if (_segments_outstanding.front().header().syn == true || _win_size != 0)
            _retransmission_timeout *= 2;

        // reset RTO, wait next timeout
        _RTO = 0;
    }
}

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    _segments_out.push(seg);
}

uint64_t TCPSender::bytes_in_flight() const { return _byte_in_flight; }

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }