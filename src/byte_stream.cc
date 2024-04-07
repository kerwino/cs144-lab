#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  if ( closed_ or available_capacity() == 0 or data.empty() ) {
    return;
  }

  if ( data.size() > available_capacity() ) {
    data.resize( available_capacity() );
  }

  bytes_buffered_ += data.size();
  bytes_pushed_ += data.size();

  buf_.push( data );
}

string_view Reader::peek() const
{
  if ( buf_.empty() ) {
    return string_view {};
  } else {
    return string_view { buf_.front() }.substr( remove_pos_ );
  }
}

void Reader::pop( uint64_t len )
{
  if ( buf_.empty() or len == 0 ) {
    return;
  }

  len = min( len, bytes_buffered_ );

  bytes_buffered_ -= len;
  bytes_poped_ += len;

  while ( len != 0 ) {
    const uint64_t front_real_size { buf_.front().size() - remove_pos_ };
    if ( len < front_real_size ) {
      remove_pos_ += len;
      break;
    }
    buf_.pop();
    remove_pos_ = 0;
    len -= front_real_size;
  }
}

bool Writer::is_closed() const
{
  return closed_;
}

void Writer::close()
{
  closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - bytes_buffered_;
}

uint64_t Writer::bytes_pushed() const
{
  return bytes_pushed_;
}

uint64_t Reader::bytes_popped() const
{
  return bytes_poped_;
}

uint64_t Reader::bytes_buffered() const
{
  return bytes_buffered_;
}

bool Reader::is_finished() const
{
  return closed_ and bytes_buffered_ == 0;
}

