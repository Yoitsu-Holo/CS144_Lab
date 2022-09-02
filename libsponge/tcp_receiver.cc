#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    TCPHeader header = seg.header();
    if (!header.syn && !_syn)
        return;
    if (header.syn && _syn)
        return;
    if (header.syn) {
        _syn = true;
        _isn = header.seqno.raw_value();
    }
    if (header.fin)
        _fin = true;
    size_t index = unwrap(header.seqno, WrappingInt32(_isn), _reassembler.unassembled_bytes());
    _reassembler.push_substring(seg.payload().copy(), header.syn ? 0 : index - 1, header.fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_syn)
        return nullopt;
    size_t absolute_seqno = stream_out().bytes_written() + 1;
    if (stream_out().input_ended())
        return wrap(absolute_seqno + 1, WrappingInt32(_isn));
    return wrap(absolute_seqno, WrappingInt32(_isn));
}

size_t TCPReceiver::window_size() const { return _capacity - stream_out().buffer_size(); }
