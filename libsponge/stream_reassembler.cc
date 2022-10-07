#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {
    _unassembledBuffer.resize(capacity);
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // calc the position of eof
    if (eof)
        _posEOF = index + data.size();

    size_t firstUnread = _output.bytes_read();
    size_t firstUnassembled = firstUnread + _output.buffer_size();
    size_t firstUnacceptable = min(firstUnread + _capacity, _posEOF);

    sizedebug = max(sizedebug, index + data.size());

    if (data.size()) {
        // if substring is totally pushed Or over maximun capacity, ignore it
        if (index + data.size() < firstUnassembled || index >= firstUnacceptable || index > _posEOF)
            return;

        string buffer{};

        if (index <= firstUnassembled) {
            buffer = data.substr(firstUnassembled - index, _output.remaining_capacity());

            size_t startAssembled = firstUnassembled + buffer.size();
            for (size_t i = firstUnassembled; i < firstUnacceptable; i++) {
                if (_unassembledBuffer[i % _capacity].first) {
                    if (i >= startAssembled)
                        buffer.push_back(_unassembledBuffer[i % _capacity].second);
                    _unassembledBuffer[i % _capacity].first = false;
                    _unassembledBytes--;
                } else if (i >= startAssembled)
                    break;
            }
            sizedebug += buffer.size();
            _output.write(buffer);
            firstUnread = _output.bytes_read();
            firstUnassembled = firstUnread + _output.buffer_size();
        } else {  // save unassembled data
            size_t pos = (firstUnassembled > index) ? (firstUnassembled - index) : (0);
            for (size_t i = pos; i < data.size() && i + index < firstUnacceptable; i++) {
                if (!_unassembledBuffer[(i + index) % _capacity].first) {
                    _unassembledBuffer[(i + index) % _capacity] = make_pair(true, data[i]);
                    _unassembledBytes++;
                }
            }
        }
    }
    if (_output.bytes_written() == _posEOF)
        _output.end_input();
    return;
}

size_t StreamReassembler::unassembled_bytes() const { return {_unassembledBytes}; }

bool StreamReassembler::empty() const {
    if (_output.buffer_empty() && !_unassembledBytes)
        return true;
    return false;
}