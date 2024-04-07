#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  uint64_t next_index = first_index + data.size();
  uint64_t unacceptable_index = output_.writer().available_capacity() + expect_index_;

  if ( is_last_substring ) {
    close_index_ = next_index;
    if ( close_index_ == expect_index_ ) {
      output_.writer().close();
    }
  }

  if ( output_.writer().is_closed() ) {
    return;
  }

  // Ready bytes OR Unacceptable bytes
  if ( next_index <= expect_index_ or first_index >= unacceptable_index ) {
    return;
  }

  if ( next_index > unacceptable_index ) {
    data.resize( unacceptable_index - first_index );
    next_index = unacceptable_index;
  }

  if ( data.empty() ) {
    return;
  }

  if ( auto pit = buf_pending_.lower_bound( first_index ); pit != buf_pending_.begin() ) {
    --pit;
    uint64_t pit_next_index = pit->first + pit->second.size();
    uint64_t pit_first_index = pit->first;
    
    if ( pit_next_index >= next_index ) {
      return;
    }
    
    if ( pit_next_index > first_index and next_index > pit_next_index ) {
      data = pit->second + data.substr( pit_next_index - first_index );
      buf_pending_.erase( pit );
      first_index = pit_first_index;
    }
  }

  for ( auto nit = buf_pending_.lower_bound( first_index ); nit != buf_pending_.end(); ) {
    uint64_t nit_first_index = nit->first;
    uint64_t nit_next_index = nit->first + nit->second.size();

    if ( nit_first_index > next_index ) {
      break;
    } else if ( nit_next_index < next_index ) {
      nit = buf_pending_.erase( nit );
    } else {
      data += nit->second.substr( next_index - nit_first_index );
      nit = buf_pending_.erase( nit );
      next_index = nit_next_index;
    }
  }

  buf_pending_.emplace( first_index, data );

  for ( auto it = buf_pending_.begin(); it != buf_pending_.end(); ) {
    if ( it->first > expect_index_ ) {
      break;
    }
    
    string real_data = it->second.substr( expect_index_ - it->first );
    output_.writer().push( real_data );
    expect_index_ += real_data.size();
    if ( close_index_ == it->first + it->second.size() ) {
      output_.writer().close();
    }
    it = buf_pending_.erase( it ); 
  }

  bytes_pending_ = 0;
  for ( auto& v : buf_pending_ ) {
    bytes_pending_ += v.second.size();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return bytes_pending_;
}

