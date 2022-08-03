#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), unassembledBuffer(), unreadBuffer(), posEOF(ULONG_LONG_MAX) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    // if (index < 1000 && data.size() < 20) {
    //     for (auto &&c : data)
    //         debug << int(c) << ",";
    //     debug << data << " | " << index << " | " << eof << endl;
    // }
    size_t firstUnread = _output.bytes_written();
    size_t firstUnassembled = _output.bytes_written() + unreadBuffer.size();

    // calc the position of eof
    if (eof)
        posEOF = index + data.size();

    // if substring is totally pushed, ignore it
    if (index + data.size() < firstUnread)
        return;

    if (index > posEOF)
        return;

    // save unassembled data
    if (index + data.size() >= firstUnassembled)
        for (size_t i = 0, pos = index; i < data.size() && pos < posEOF; i++, pos++) {
            if (pos >= firstUnassembled)
                unassembledBuffer[pos] = data[i];
            //! testpoint has bug??
            else if (pos >= firstUnread)
                unreadBuffer[unreadBuffer.size() - (firstUnassembled - pos)] = data[i];
        }

    // put assembled data to unread buffer
    while (unassembledBuffer.find(firstUnassembled) != unassembledBuffer.end()) {
        unreadBuffer.push_back(unassembledBuffer[firstUnassembled]);
        unassembledBuffer.erase(unassembledBuffer.find(firstUnassembled));
        firstUnassembled++;
    }

    // put assembled data to _output buffer
    // if (unreadBuffer.size() < 20) {
    //     for (auto &&c : unreadBuffer)
    //         debug << int(c) << ",";
    //     debug << "bufed data: " << unreadBuffer << endl;
    // }
    int byteWritten = _output.write(unreadBuffer);
    unreadBuffer = unreadBuffer.substr(byteWritten);

    if (_output.bytes_written() == posEOF)
        _output.end_input();

    return;
}

size_t StreamReassembler::unassembled_bytes() const { return {unassembledBuffer.size()}; }

bool StreamReassembler::empty() const {
    if (unreadBuffer.size() + unassembledBuffer.size())
        return false;
    return true;
}