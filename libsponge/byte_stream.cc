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
    _buff.append(data.substr(0, writeBytes));
    return writeBytes;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t peekBytes = min(len, _buff.size());
    return _buff.substr(0, peekBytes);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t popBytes = min(len, _buff.size());
    _totout += popBytes;
    _buff.assign(_buff.begin() + popBytes, _buff.end());
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

size_t ByteStream::buffer_size() const { return {_buff.size()}; }

bool ByteStream::buffer_empty() const { return {_buff.empty()}; }

size_t ByteStream::remaining_capacity() const { return {_capacity - _buff.size()}; }

bool ByteStream::eof() const { return (this->input_ended() && this->buffer_empty()); }

size_t ByteStream::bytes_written() const { return {_totin}; }

size_t ByteStream::bytes_read() const { return {_totout}; }
