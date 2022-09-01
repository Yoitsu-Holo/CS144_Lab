#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : buff(""), front(0), back(0), isInputEnd(false), totin(0), totout(0) {
    // DUMMY_CODE(capacity);
    buff.clear();
    buff.resize(capacity + 1);
    assert(capacity > 0);
}

size_t ByteStream::write(const string &data) {
    // DUMMY_CODE(data);
    size_t write = 0;
    for (auto &&c : data) {
        // if the next ptr of back is front, that buff is full
        if (front == (back + 1) % buff.size())
            break;
        buff[back] = c;
        write++;
        totin++;
        back = (back + 1) % buff.size();
    }
    return {write};
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    // DUMMY_CODE(len);
    std::string outbuff;
    size_t ptr = front;
    for (size_t i = 0; i < len; i++) {
        if (ptr == back)
            break;
        outbuff += buff[ptr];
        ptr = (ptr + 1) % buff.size();
    }
    return {outbuff};
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    // DUMMY_CODE(len);
    front = (front + len) % buff.size();
    totout += len;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    // DUMMY_CODE(len);
    std::string outbuff = this->peek_output(len);
    this->pop_output(outbuff.size());
    return {outbuff};
}

void ByteStream::end_input() { isInputEnd = true; }

bool ByteStream::input_ended() const { return {isInputEnd}; }

size_t ByteStream::buffer_size() const { return {(back + buff.size() - front) % buff.size()}; }

bool ByteStream::buffer_empty() const { return {front == back}; }

size_t ByteStream::remaining_capacity() const { return {buff.size() - this->buffer_size() - 1}; }

bool ByteStream::eof() const { return (this->input_ended() && this->buffer_empty()); }

size_t ByteStream::bytes_written() const { return {totin}; }

size_t ByteStream::bytes_read() const { return {totout}; }
