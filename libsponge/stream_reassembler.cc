#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {
    //_unassembledBuffer.resize(capacity);
    _unassembledBuffer.push_back(make_pair(ULONG_LONG_MAX, ""));
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

    if (data.size()) {
        // if substring is totally pushed Or over maximun capacity, ignore it
        if (index + data.size() < firstUnassembled || index >= firstUnacceptable || index > _posEOF)
            return;
        const size_t startPos = max(index, firstUnassembled);
        const string newString = data.substr(startPos - index, firstUnacceptable - startPos);

        _unassembledBytes += push_unassembledString(newString, startPos);
        string temp = pop_unassembledString(firstUnassembled);

        _unassembledBytes -= temp.size();
        _output.write(temp);
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

inline size_t StreamReassembler::push_unassembledString(const string &data, const uint64_t index) {
    size_t appendBytes = 0;
    size_t pos = index;
    // not empty
    for (auto it = _unassembledBuffer.begin(); it != _unassembledBuffer.end(); it++) {
        if (pos >= index + data.size())
            break;
        if (index <= it->first) {
            if (pos < index)
                pos = index;
            if (it->first < pos)
                continue;
            string temp = data.substr(pos - index, it->first - pos);
            if (temp.size()) {
                appendBytes += temp.size();
                _unassembledBuffer.insert(it, make_pair(pos, temp));
                // it--;
            }
        }
        pos = it->first + it->second.size();
    }
    return appendBytes;
}

inline string StreamReassembler::pop_unassembledString(size_t firstUnassembled) {
    string temp;
    auto it = _unassembledBuffer.begin();
    while (it->first < ULONG_LONG_MAX) {
        auto itt = it;
        it++;

        if (itt->first == firstUnassembled) {
            temp.append(itt->second);
            firstUnassembled += itt->second.size();
            _unassembledBuffer.erase(itt);
        } else
            break;
    }
    return temp;
}