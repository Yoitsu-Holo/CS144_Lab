#ifndef SPONGE_LIBSPONGE_BYTE_STREAM_HH
#define SPONGE_LIBSPONGE_BYTE_STREAM_HH

#include <string>
#include <assert.h>

//! \brief An in-order byte stream.

//! Bytes are written on the "input" side and read from the "output"
//! side.  The byte stream is finite: the writer can end the input,
//! and then no more bytes can be written.
class ByteStream {
  private:
    // Your code here -- add private members as necessary.
    std::string _buff;           // buffer
    size_t _front{0}, _back{0};  // two pointer, pointed buffer front and back
    bool _isInputEnd{false};
    size_t _totin{0}, _totout{0};
    size_t _capacity, _realCapacity{0};
    //& Hint: This doesn't need to be a sophisticated data structure at
    //& all, but if any of your tests are taking longer than a second,
    //& that's a sign that you probably want to keep exploring
    //& different approaches.

    bool _error{};  //!< Flag indicating that the stream suffered an error.

  public:
    //^ Construct a stream with room for `capacity` bytes.
    ByteStream(const size_t capacity);

    //^ \name "Input" interface for the writer
    //! @{
    size_t write(const std::string &data);  //* Write a string of bytes into the stream. Write as many
                                            //* as will fit, and return how many were written.
                                            //* \returns the number of bytes accepted into the stream
    size_t remaining_capacity() const;      //* \returns the number of additional bytes that the stream has space for
    void end_input();                       //* Signal that the byte stream has reached its ending
    void set_error() { _error = true; }     //* Indicate that the stream suffered an error.
    //! @}

    //^ \name "Output" interface for the reader
    //!@{
    std::string peek_output(const size_t len) const;  //* Peek at next "len" bytes of the stream
                                                      //* \returns a string
    void pop_output(const size_t len);                //* Remove bytes from the buffer
    std::string read(const size_t len);    //* Read (i.e., copy and then pop) the next "len" bytes of the stream
                                           //*\returns a string
    bool input_ended() const;              //* \returns `true` if the stream input has ended
    bool error() const { return _error; }  //* \returns `true` if the stream has suffered an error
    size_t buffer_size() const;            //* \returns the maximum amount that can currently be read from the stream
    bool buffer_empty() const;             //* \returns `true` if the buffer is empty
    bool eof() const;                      //* \returns `true` if the output has reached the ending
    //! @}

    //^ \name General accounting
    //!@{
    size_t bytes_written() const;  //* Total number of bytes written
    size_t bytes_read() const;     //* Total number of bytes popped
    //!@}
};

#endif  // SPONGE_LIBSPONGE_BYTE_STREAM_HH
