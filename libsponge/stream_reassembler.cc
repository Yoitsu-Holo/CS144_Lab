#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), que(), isEOF(false), posEOF(0) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if (data.size() < 100)
        std::cerr << "Information : "
                  << "data: " << data << "; index: " << index << "; eof: " << eof << endl;
    // DUMMY_CODE(data, index, eof);
    que.push(make_pair(index, data));
    if (eof)
        isEOF = true;
    prr tempprr;          // temp pair<index, string>
    std::string orgstr;   // oranginal string
    std::string tempstr;  // string buffer to use unordered_map
    size_t idx;
    if (eof)
        posEOF = index + data.size();
    while (!que.empty() && que.top().first <= _output.bytes_written()) {
        if (!_output.remaining_capacity())
            break;

        tempprr = que.top();
        que.pop();

        idx = tempprr.first;
        orgstr = tempprr.second;

        if (idx + orgstr.size() < _output.bytes_written())
            continue;

        tempstr = orgstr.substr(_output.bytes_written() - idx);

        _output.write(tempstr);

        if (idx + orgstr.size() > _output.bytes_written()) {
            que.push(tempprr);
        }

        std::cerr  //<< "Function : " << orgstr << " :: " << tempstr << endl
            << "  startpoint " << _output.bytes_written() << "; endpoint " << idx + orgstr.size() << endl;
    }
    // if (eof)
    // if (que.empty() && isEOF) {
    if (isEOF && posEOF <= _output.bytes_written()) {
        std::cerr << "successfully" << endl;
        _output.end_input();
    }

    std::cerr << "Information : >>" << _output.bytes_written() << "  eof :" << _output.eof() << endl;
}

size_t StreamReassembler::unassembled_bytes() const { return {_output.buffer_size()}; }

bool StreamReassembler::empty() const { return {_output.buffer_empty()}; }
