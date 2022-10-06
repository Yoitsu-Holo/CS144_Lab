#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _capacity(capacity) {}

size_t ByteStream::write(const string &data) {
    size_t writeBytes = min(data.size(), remaining_capacity());
    _totin += writeBytes;
    _buffSize += writeBytes;
    _buff.push_back(data.substr(0, writeBytes));
    return writeBytes;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t peekBytes = min(len, _buffSize);
    std::string temp;
    for (int i = 0; temp.length() < peekBytes; i++)
        temp.append(_buff[i]);
    return temp.substr(0, peekBytes);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t lestBytes = min(len, _buffSize);
    _totout += len;
    _buffSize -= len;
    while (lestBytes) {
        std::string temp = _buff.front();
        _buff.pop_front();
        if (lestBytes >= temp.size())
            lestBytes -= temp.size();
        else {
            _buff.push_front(temp.substr(lestBytes));
            lestBytes = 0;
        }
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    std::string outbuff = peek_output(len);
    pop_output(outbuff.size());
    // std::cerr << "Info: " << len << endl;
    return {outbuff};
}

void ByteStream::end_input() { _isInputEnd = true; }

bool ByteStream::input_ended() const { return {_isInputEnd}; }

size_t ByteStream::buffer_size() const { return {_buffSize}; }

bool ByteStream::buffer_empty() const { return {_buff.empty()}; }

size_t ByteStream::remaining_capacity() const { return {_capacity - _buffSize}; }

bool ByteStream::eof() const { return (this->input_ended() && this->buffer_empty()); }

size_t ByteStream::bytes_written() const { return {_totin}; }

size_t ByteStream::bytes_read() const { return {_totout}; }
