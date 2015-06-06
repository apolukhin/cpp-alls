#include <cppalls/core/response.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/numeric/conversion/cast.hpp>

namespace {
    template <class StreamT, class T>
    StreamT& do_ostream(StreamT* stream, T& val) {
        boost::endian::native_to_little_inplace(val);
        stream->put_data(reinterpret_cast<const unsigned char*>(&val), sizeof(val));
        return *stream;
    }
} // anonymous namespace

namespace cppalls {

response& response::operator<< (bool val) {
    const unsigned char c = val;
    return (*this) << c;
}

response& response::operator<< (short val) {
    return do_ostream(this, val);
}

response& response::operator<< (unsigned short val) {
    return do_ostream(this, val);
}

response& response::operator<< (int val) {
    return do_ostream(this, val);
}

response& response::operator<< (unsigned int val) {
    return do_ostream(this, val);
}

response& response::operator<< (long val) {
    return do_ostream(this, val);
}

response& response::operator<< (unsigned long val) {
    return do_ostream(this, val);
}

response& response::operator<< (long long val) {
    return do_ostream(this, val);
}

response& response::operator<< (unsigned long long val) {
    return do_ostream(this, val);
}

response& response::operator<< (float val) {
    return do_ostream(this, val);
}

response& response::operator<< (double val) {
    return do_ostream(this, val);
}

response& response::operator<< (long double val) {
    return do_ostream(this, val);
}

response& response::operator<< (const std::string& val) {
    const unsigned int size = boost::numeric_cast<unsigned int>(val.size());
    (*this) << size;
    put_data(reinterpret_cast<const unsigned char*>(val.data()), size);
    return *this;
}

} // namespace cppalls
