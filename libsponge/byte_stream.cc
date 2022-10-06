#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _buff(""), _capacity(capacity) {
    // DUMMY_CODE(capacity);
    _realCapacity = _capacity + 1;
    _buff.resize(_realCapacity);
    assert(capacity > 0);
}

size_t ByteStream::write(const string &data) {
    // DUMMY_CODE(data);
    size_t write = 0;
    for (auto &&c : data) {
        // if the next ptr of back is front, that buff is full
        if (_front == (_back + 1) % _realCapacity)
            break;
        _buff[_back] = c;
        write++;
        _back = (_back + 1) % _realCapacity;
    }
    _totin += write;
    return {write};
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    std::string outbuff;
    outbuff.clear();
    size_t ptr = _front;
    for (size_t i = 0; i < len; i++) {
        if (ptr == _back)
            break;
        outbuff += _buff[ptr];
        ptr = (ptr + 1) % _realCapacity;
    }
    return {outbuff};
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    // DUMMY_CODE(len);
    _front = (_front + len) % _realCapacity;
    _totout += len;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    // DUMMY_CODE(len);
    std::string outbuff;
    outbuff = this->peek_output(len);
    this->pop_output(outbuff.size());
    return {outbuff};
}

void ByteStream::end_input() { _isInputEnd = true; }

bool ByteStream::input_ended() const { return {_isInputEnd}; }

size_t ByteStream::buffer_size() const { return {(_back + _realCapacity - _front) % _realCapacity}; }

bool ByteStream::buffer_empty() const { return {_front == _back}; }

size_t ByteStream::remaining_capacity() const { return {_realCapacity - this->buffer_size() - 1}; }

bool ByteStream::eof() const { return (this->input_ended() && this->buffer_empty()); }

size_t ByteStream::bytes_written() const { return {_totin}; }

size_t ByteStream::bytes_read() const { return {_totout}; }
