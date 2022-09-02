#ifndef SPONGE_LIBSPONGE_TCP_SENDER_HH
#define SPONGE_LIBSPONGE_TCP_SENDER_HH

#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <functional>
#include <queue>
#include <map>

//! \brief The "sender" part of a TCP implementation.

//! Accepts a ByteStream, divides it up into segments and sends the
//! segments, keeps track of which segments are still in-flight,
//! maintains the Retransmission Timer, and retransmits in-flight
//! segments if the retransmission timer expires.
class TCPSender {
  private:
    WrappingInt32 _isn;                                 //& our initial sequence number, the number for our SYN.
    std::queue<TCPSegment> _segments_out{};             //& outbound queue of segments that the TCPSender wants sent
    unsigned int _initial_retransmission_timeout;       //& retransmission timer for the connection
    ByteStream _stream;                                 //& outgoing stream of bytes that have not yet been sent
    uint64_t _next_seqno{0};                            //& the (absolute) sequence number for the next byte to be sent
    unsigned int _retransmission_timeout{0};            //~ retransmission timeout
    unsigned int _retransmission_timecount{0};          //~ retransmission timecount
    unsigned int _consecutive_retransmissions{0};       //~ consecutive retransmissions
    std::map<size_t, TCPSegment> _segments_outgoing{};  //~ outstanding segments
    size_t _outgoing_bytes{0};                          //~ the nnmber of bytes send but not ACK
    bool _syn{false}, _fin{false};                      //~ is SYN or FIN
    size_t _win_size{1};                                //~ window size

  public:
    TCPSender(const size_t capacity = TCPConfig::DEFAULT_CAPACITY,
              const uint16_t retx_timeout = TCPConfig::TIMEOUT_DFLT,
              const std::optional<WrappingInt32> fixed_isn = {});  //* Initialize a TCPSender

    //^ \name "Input" interface for the writer

    ByteStream &stream_in() { return _stream; }
    const ByteStream &stream_in() const { return _stream; }

    //^ \name Methods that can cause the TCPSender to send a segment

    void fill_window();  //* \brief create and send segments to fill as much of the window as possible
    void ack_received(const WrappingInt32 ackno,
                      const uint16_t window_size);  //* \brief A new acknowledgment was received
    void tick(const size_t ms_since_last_tick);     //* \brief Notifies the TCPSender of the passage of time
    void send_empty_segment();  //* \brief Generate an empty-payload segment (useful for creating empty ACK segments)

    //^ \name Accessors

    //* \brief How many sequence numbers are occupied by segments sent but not yet acknowledged?
    //* \note count is in "sequence space," i.e. SYN and FIN each count for one byte
    //* (see TCPSegment::length_in_sequence_space())
    size_t bytes_in_flight() const;

    //* \brief Number of consecutive retransmissions that have occurred in a row
    unsigned int consecutive_retransmissions() const;

    std::queue<TCPSegment> &segments_out() { return _segments_out; }

    //^ \name What is the next sequence number? (used for testing)

    uint64_t next_seqno_absolute() const { return _next_seqno; }
    WrappingInt32 next_seqno() const { return wrap(_next_seqno, _isn); }
};

#endif  // SPONGE_LIBSPONGE_TCP_SENDER_HH
