#include <cppalls/core/request.hpp>
#include <boost/endian/conversion.hpp>

namespace {
    template <class StreamT, class T>
    StreamT& do_istream(StreamT* stream, T& val) {
        stream->extract_data(reinterpret_cast<unsigned char *>(&val), sizeof(val));
        boost::endian::native_to_little_inplace(val);
        return *stream;
    }
} // anonymous namespace

namespace cppalls { 

request& request::operator>> (bool& val) {
    unsigned char c;
    (*this) >> c;
    val = !!c;
    return *this;
}

request& request::operator>> (char& val) {
    return do_istream(this, val);
}

request& request::operator>> (unsigned char& val) {
    return do_istream(this, val);
}

request& request::operator>> (short& val) {
    return do_istream(this, val);
}

request& request::operator>> (unsigned short& val) {
    return do_istream(this, val);
}

request& request::operator>> (int& val) {
    return do_istream(this, val);
}

request& request::operator>> (unsigned int& val) {
    return do_istream(this, val);
}

request& request::operator>> (long& val) {
    return do_istream(this, val);
}

request& request::operator>> (unsigned long& val) {
    return do_istream(this, val);
}

request& request::operator>> (long long& val) {
    return do_istream(this, val);
}

request& request::operator>> (unsigned long long& val) {
    return do_istream(this, val);
}

request& request::operator>> (float& val) {
    return do_istream(this, val);
}

request& request::operator>> (double& val) {
    return do_istream(this, val);
}

request& request::operator>> (long double& val) {
    return do_istream(this, val);
}

request& request::operator>> (std::string& val) {
    unsigned int size;
    (*this) >> size;
    val.resize(size + 1);
    extract_data(reinterpret_cast<unsigned char*>(&val[0]), size);
    return *this;
}

} // namespace cppalls

