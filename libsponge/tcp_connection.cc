#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return {_sender.stream_in().remaining_capacity()}; }

size_t TCPConnection::bytes_in_flight() const { return {_sender.bytes_in_flight()}; }

size_t TCPConnection::unassembled_bytes() const { return {_receiver.unassembled_bytes()}; }

size_t TCPConnection::time_since_last_segment_received() const { return {_time_since_last_segment_received}; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    if (!_active)
        return;
    _time_since_last_segment_received = 0;
    // cerr << "Log: " << state().state_summary(_receiver) << " | " << state().state_summary(_sender) << endl;

    // RST flag
    if (seg.header().rst) {
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _active = false;
        return;
    }
    // Close/Listen
    if (state() == TCPState::State::CLOSED || state() == TCPState::State::LISTEN) {
        if (seg.header().syn) {
            _receiver.segment_received(seg);
            connect();
        }
        return;
    }
    // SYN SENT
    if (state() == TCPState::State::SYN_SENT) {
        if (seg.header().syn && seg.header().ack) {
            // rcv SYN,ACK | snd ACK | in ESTAB
            _sender.ack_received(seg.header().ackno, seg.header().win);
            _receiver.segment_received(seg);
            _sender.send_empty_segment();
            send_segments();
        } else if (seg.header().syn && !seg.header().ack) {
            // rcv SYN | snd ACK | in SYN recv
            _receiver.segment_received(seg);
            _sender.send_empty_segment();
            send_segments();
        }
        return;
    }
    // SYN RCVD
    if (state() == TCPState::State::SYN_RCVD) {
        // rcv ACK | in ESTAB
        _sender.ack_received(seg.header().ackno, seg.header().win);
        _receiver.segment_received(seg);
        return;
    }
    // ESTAB
    if (state() == TCPState::State::ESTABLISHED) {
        // passive close
        // rcv FIN | snd ACK
        if (seg.header().fin) {
            _sender.ack_received(seg.header().ackno, seg.header().win);
            _receiver.segment_received(seg);
            _sender.send_empty_segment();
            send_segments();
            return;
        }
        // snd DATA | update ACK
        else {
            _sender.ack_received(seg.header().ackno, seg.header().win);
            _receiver.segment_received(seg);
            if (seg.length_in_sequence_space() > 0)
                _sender.send_empty_segment();
            _sender.fill_window();
            send_segments();
            return;
        }
    }
    if (state() == TCPState::State::CLOSE_WAIT) {
    }
    // FIN WAIT-1
    if (state() == TCPState::State::FIN_WAIT_1) {
        if (seg.header().fin) {
            // rcv FIN | send ACK | in CLOSING/TIME WAIT
            _sender.ack_received(seg.header().ackno, seg.header().win);
            _receiver.segment_received(seg);
            _sender.send_empty_segment();
            send_segments();
        } else if (seg.header().ack) {
            // rcv ACK | in FIN WAIT-2
            _sender.ack_received(seg.header().ackno, seg.header().win);
            _receiver.segment_received(seg);
            send_segments();
        }
        return;
    }
    // FIN WAIT-2
    if (state() == TCPState::State::FIN_WAIT_2) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
        _receiver.segment_received(seg);
        _sender.send_empty_segment();
        send_segments();
        return;
    }
    // TIME WAIT
    if (state() == TCPState::State::TIME_WAIT) {
        if (seg.header().fin) {
            // rcv FIN | hold TIME WAIT
            _sender.ack_received(seg.header().ackno, seg.header().win);
            _receiver.segment_received(seg);
            _sender.send_empty_segment();
            send_segments();
        }
        return;
    }
    // OTHER
    {
        _sender.ack_received(seg.header().ackno, seg.header().win);
        _receiver.segment_received(seg);
        _sender.fill_window();
        send_segments();
    }
}

bool TCPConnection::active() const { return {_active}; }

size_t TCPConnection::write(const string &data) {
    if (!data.size())
        return 0;
    size_t writeByte = _sender.stream_in().write(data);
    _sender.fill_window();
    send_segments();
    return {writeByte};
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    if (!_active)
        return;
    _time_since_last_segment_received += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        reset_connection();
        return;
    }
    send_segments();
    return;
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    send_segments();
    return;
}

void TCPConnection::connect() {
    _sender.fill_window();
    send_segments();
    return;
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

void TCPConnection::send_segments() {
    TCPSegment seg;
    while (!_sender.segments_out().empty()) {
        seg = _sender.segments_out().front();
        _sender.segments_out().pop();
        if (_receiver.ackno().has_value()) {
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
            seg.header().win = _receiver.window_size();
        }
        _segments_out.push(seg);
    }
    if (_receiver.stream_out().input_ended()) {
        if (!_sender.stream_in().eof())
            _linger_after_streams_finish = false;
        else if (_sender.bytes_in_flight() == 0)
            if (!_linger_after_streams_finish || _time_since_last_segment_received >= 10 * _cfg.rt_timeout)
                _active = false;
    }
}

void TCPConnection::reset_connection() {
    TCPSegment seg;
    seg.header().rst = true;
    _receiver.stream_out().set_error();
    _sender.stream_in().set_error();
    _active = false;
    _segments_out.push(seg);
    return;
}