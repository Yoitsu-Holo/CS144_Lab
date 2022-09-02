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
    , _stream(capacity) {}

void TCPSender::fill_window() {
    size_t win_size = _win_size ? _win_size : 1;

    while (win_size > _outgoing_bytes) {
        TCPSegment seg;

        // if first segment, set header
        // set syn and update status
        if (!_syn)
            _syn = seg.header().syn = true;

        // set seqno
        seg.header().seqno = next_seqno();

        size_t payload_size = min(TCPConfig::MAX_PAYLOAD_SIZE, win_size - _outgoing_bytes - seg.header().syn);
        string payload = _stream.read(payload_size);

        if (!_fin && _stream.eof() && payload.size() + _outgoing_bytes < win_size)
            _fin = seg.header().fin = true;

        seg.payload() = Buffer(std::move(payload));

        // stream is empty, break
        if (seg.length_in_sequence_space() == 0)
            break;

        // reset RTO
        if (_segments_outgoing.empty()) {
            _retransmission_timeout = _initial_retransmission_timeout;
            _retransmission_timecount = 0;
        }

        // send segment
        _segments_out.push(seg);
        // trace segment
        _outgoing_bytes += seg.length_in_sequence_space();
        _segments_outgoing.insert(make_pair(_next_seqno, seg));
        // update next seqno
        _next_seqno += seg.length_in_sequence_space();
        // if FIN, break
        if (seg.header().fin)
            break;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    size_t absolute_seqno = unwrap(ackno, _isn, _next_seqno);

    // unreliable ACK, ignore it
    if (absolute_seqno > _next_seqno)
        return;

    auto it = _segments_outgoing.begin();
    while (it != _segments_outgoing.end()) {
        const TCPSegment &seg = it->second;
        if (it->first + seg.length_in_sequence_space() <= absolute_seqno) {
            _outgoing_bytes -= seg.length_in_sequence_space();
            it = _segments_outgoing.erase(it);

            _retransmission_timeout = _initial_retransmission_timeout;
            _retransmission_timecount = 0;
        } else
            break;
    }

    // update window size
    _win_size = window_size;
    _consecutive_retransmissions = 0;
    fill_window();
    return;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    _retransmission_timecount += ms_since_last_tick;

    // timeout and have segments not ACKed
    if (_retransmission_timecount >= _retransmission_timeout &&
        _segments_outgoing.begin() != _segments_outgoing.end()) {
        // net traffic, doubled retransmisson_timeout !!
        if (_win_size > 0)
            _retransmission_timeout *= 2;

        // retransmission and retransmisson count +1
        _segments_out.push(_segments_outgoing.begin()->second);
        _consecutive_retransmissions++;

        // reset RTO, wait next timeout
        _retransmission_timecount = 0;
    }
}

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    _segments_out.push(seg);
}

uint64_t TCPSender::bytes_in_flight() const { return _outgoing_bytes; }

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }